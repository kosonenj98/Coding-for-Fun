#include "traininghandler.h"

namespace
{
    const QString &logTag()
    {
        static const QString tag = QStringLiteral("TrainingHandler");
        return tag;
    }
}

TrainingHandler::TrainingHandler(Logger &logger, QObject *parent)
    : QObject{parent}, m_logger(logger)
{
    m_logger.verbose(logTag(), QStringLiteral("Initializing TrainingHandler..."));
    m_logger.verbose(logTag(), QStringLiteral("Initializing TrainingHandler done!"));
}

void TrainingHandler::startTraining(const Training::StartRequest &request)
{
    Vocab::GetDataRequest getTrainingDataRequest;
    getTrainingDataRequest.filters = request.vocabFilters;
    emit requestGetData(getTrainingDataRequest);
}

void TrainingHandler::handleGetDataFinished(const Vocab::GetDataResult &result)
{
    if (result.code != Error::Code::Success)
    {
        m_logger.error(logTag(), QStringLiteral("Failed to get training data! ErrorCode %1").arg(Error::codeToQString(result.code)));
        Training::Result trainingResult;
        trainingResult.code = result.code;
        emit trainingFinished(trainingResult);
        return;
    }

    m_questionDatas.clear();
    m_trainingHistory.clear();
    m_totalEntryCount = result.totalEntryCount;
    m_questionDatas.reserve(m_totalEntryCount);
    m_newQuestionIndex = 0;
    m_currentQuestionIndex = m_newQuestionIndex;
    createQuestionDatas(result.datas);
    for (const auto &questionData : m_questionDatas)
    {
        Training::HistoryInfo info;
        info.questionIndex = questionData.index;
        info.roundIndex = questionData.roundContexts.count() - 1;
        m_trainingHistory.append(info);
    }

    handleTraining();
}

void TrainingHandler::handleAnswer(const Training::Answer &answerData)
{
    switch (answerData.state)
    {
    case Training::QuestionState::Unanswered:
    {
        int questionIndex = m_trainingHistory.at(m_currentQuestionIndex).questionIndex;
        Training::QuestionData &questionData = m_questionDatas[questionIndex];
        questionData.roundContexts.last().state = Training::QuestionState::Answered;
        questionData.roundContexts.last().answer = answerData.answer;
        questionData.roundContexts.last().isCorrect = true; // TODO: check answer
        handleTraining();
        emit nextQuestionAvailable(!m_trainingFinished);
        break;
    }

    // Pressing enter again with answered question yields latest question
    case Training::QuestionState::Answered:
        newQuestion();
        break;
    }
}

void TrainingHandler::changeQuestion(const Training::ChangeQuestionRequest &request)
{
    Training::ChangeQuestionResult result;

    switch (request.change)
    {
    case Training::ChangeQuestion::Next:
    {
        Training::HistoryInfo info = m_trainingHistory.at(m_currentQuestionIndex);
        if (m_questionDatas.at(info.questionIndex).roundContexts.at(info.roundIndex).state == Training::QuestionState::Unanswered)
        {
            // Cannot skip unanswered question
            result.order = Training::QuestionOrder::Last;
            break;
        }

        if (m_currentQuestionIndex < m_newQuestionIndex)
        {
            m_currentQuestionIndex++;
            result.order = m_currentQuestionIndex < m_newQuestionIndex ? Training::QuestionOrder::Middle : Training::QuestionOrder::Last;
        }
        else if (m_currentQuestionIndex == m_newQuestionIndex)
        {
            m_newQuestionIndex++;
            m_currentQuestionIndex = m_newQuestionIndex;
            result.order = Training::QuestionOrder::Last;
        }
        else
        {
            m_currentQuestionIndex = m_newQuestionIndex;
            result.code = Error::Code::OutOfBoundsEnd;
        }

        break;
    }

    case Training::ChangeQuestion::Previous:
    {
        if (m_currentQuestionIndex > 0)
        {
            m_currentQuestionIndex--;
            result.order = m_currentQuestionIndex > 0 ? Training::QuestionOrder::Middle : Training::QuestionOrder::First;
        }
        else
        {
            result.code = Error::Code::OutOfBoundsStart;
        }

        break;
    }

    case Training::ChangeQuestion::New:
        if (m_newQuestionIndex < m_trainingHistory.count() - 1)
        {
            // Do not skip unanswered questions
            Training::HistoryInfo info = m_trainingHistory.at(m_newQuestionIndex);
            if (m_questionDatas.at(info.questionIndex).roundContexts.at(info.roundIndex).state == Training::QuestionState::Answered)
            {
                m_newQuestionIndex++;
            }
        }
        m_currentQuestionIndex = m_newQuestionIndex;
        result.order = Training::QuestionOrder::Last;

        break;
    }
    emit changeQuestionFinished(result);

    handleQuestion();
}

void TrainingHandler::createQuestionDatas(const QList<Vocab::Data> &datas)
{
    for (const auto &vocabData : datas)
    {
        for (auto it = vocabData.entries.cbegin(); it != vocabData.entries.cend(); it++)
        {
            m_questionDatas.append(createQuestionData(vocabData.title, QString(), it.key(), it.value()));
        }

        for (auto groupIt = vocabData.groupEntries.cbegin(); groupIt != vocabData.groupEntries.cend(); groupIt++)
        {
            for (auto it = groupIt.value().cbegin(); it != groupIt.value().cend(); it++)
            {
                m_questionDatas.append(createQuestionData(vocabData.title, groupIt.key(), it.key(), it.value()));
            }
        }
    }
}

Training::QuestionData TrainingHandler::createQuestionData(const QString &vocabTitle, const QString &groupTitle, const QString &question, const QSet<QString> &correctAnswer)
{
    Training::VocabContext vocabContext;
    vocabContext.vocabTitle = vocabTitle;
    vocabContext.groupTitle = groupTitle;

    Training::RoundContext roundContext;
    roundContext.roundNumber = 1;
    roundContext.questionNumber = m_questionDatas.count() + 1;
    roundContext.roundEntryCount = m_totalEntryCount;
    roundContext.state = Training::QuestionState::Unanswered;
    roundContext.isCorrect = false;

    Training::QuestionData questionData;
    questionData.index = m_questionDatas.count();
    questionData.vocabContext = vocabContext;
    questionData.roundContexts.append(roundContext);
    questionData.question = question;
    questionData.correctAnswer = correctAnswer;
    questionData.wrongCount = 0; // Gets updated later
    questionData.correctCount = 0; // Gets updated later

    return questionData;
}

Training::Question TrainingHandler::createQuestion(const Training::QuestionData &questionData, int roundIndex)
{
    Training::TrainingContext trainingContext;
    trainingContext.totalEntryCount = m_totalEntryCount;
    trainingContext.correctAnswerCount = getCorrectAnswerCount();

    Training::Question question;
    question.index = questionData.index;
    question.trainingContext = trainingContext;
    question.roundContext = roundIndex < 0 ? questionData.roundContexts.last() : questionData.roundContexts.at(roundIndex);
    question.question = questionData.question;
    question.correctAnswer = questionData.correctAnswer;

    return question;
}

void TrainingHandler::handleTraining()
{
    m_trainingFinished = getCorrectAnswerCount() == m_totalEntryCount;
    if (m_trainingFinished)
    {
        // All questions answered correctly
        Training::Result result;
        emit trainingFinished(result); // TODO: User may want to continue training
    }
    else if (allAnswered())
    {
        // Add questions with wrong answers to next round
        handleNextRound(getQuestionIndicesForWrongAnswers());
    }

    handleQuestion();
}

void TrainingHandler::handleQuestion()
{
    Training::HistoryInfo info = m_trainingHistory[m_currentQuestionIndex];
    Training::Question currentQuestion = createQuestion(m_questionDatas.at(info.questionIndex), info.roundIndex);
    emit question(currentQuestion);
}

void TrainingHandler::newQuestion()
{
    Training::ChangeQuestionRequest request;
    request.change = Training::ChangeQuestion::New;
    changeQuestion(request);
}

void TrainingHandler::handleNextRound(const QList<int> &questionDataIndices)
{
    int questionNumber = 0;
    for (const auto &index : questionDataIndices)
    {
        Training::QuestionData &questionData = m_questionDatas[index];

        Training::HistoryInfo historyInfo;
        historyInfo.questionIndex = questionData.index;
        historyInfo.roundIndex = questionData.roundContexts.count();

        Training::RoundContext newRoundContext;
        newRoundContext.roundNumber = questionData.roundContexts.last().roundNumber + 1;
        newRoundContext.questionNumber = ++questionNumber;
        newRoundContext.roundEntryCount = questionDataIndices.count();
        newRoundContext.state = Training::QuestionState::Unanswered;
        newRoundContext.isCorrect = false;

        questionData.roundContexts.append(newRoundContext);
        m_trainingHistory.append(historyInfo);
    }
}

bool TrainingHandler::allAnswered()
{
    // Check if there exists unanswered questions
    return !std::any_of(m_questionDatas.cbegin(), m_questionDatas.cend(),
                        [](const Training::QuestionData &questionData)
                        {
                            return questionData.roundContexts.last().state == Training::QuestionState::Unanswered;
                        });
}

int TrainingHandler::getCorrectAnswerCount()
{
    // Check latest round information for answer status
    return std::count_if(m_questionDatas.cbegin(), m_questionDatas.cend(),
        [](const Training::QuestionData &questionData)
        {
            return questionData.roundContexts.last().isCorrect;
        }
    );
}

const QList<int> TrainingHandler::getQuestionIndicesForWrongAnswers()
{
    QList<int> result;
    for (const auto &questionData : m_questionDatas)
    {
        if (!questionData.roundContexts.last().isCorrect)
        {
            result.append(questionData.index);
        }
    }

    return result;
}
