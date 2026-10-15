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
    Vocab::GetRequest getTrainingDataRequest;
    getTrainingDataRequest.filters = request.vocabFilters;
    emit requestGetData(getTrainingDataRequest);
}

void TrainingHandler::handleGetDataFinished(const Vocab::GetResult &result)
{
    if (result.code != Error::Code::Success)
    {
        m_logger.error(logTag(), QStringLiteral("Failed to get training data! ErrorCode %1").arg(Error::codeToQString(result.code)));
        Training::Result trainingResult;
        trainingResult.code = result.code;
        emit trainingFinished(trainingResult);
        return;
    }

    m_round = 1;
    m_totalEntryCount = result.totalEntryCount;
    m_correctAnswerCount = 0;
    m_datas = result.datas;
    m_currentQuestionIndex = 0;
    createQuestions();
    handleQuestions();
}

void TrainingHandler::handleAnswer(const Training::QuestionData &answerData)
{
    Training::QuestionData &questionData = m_questionDatas[m_currentQuestionIndex].questionData;
    questionData.state = Training::QuestionState::Answered;
    questionData.answer = answerData.answer;

    // TODO: check answer
    questionData.isCorrect = true;
    m_correctAnswerCount++;
    questionData.context.correctAnswerCount = m_correctAnswerCount;

    handleQuestions();
}

void TrainingHandler::changeQuestion(const Training::QuestionChangeRequest &request)
{
    Training::QuestionChangeResult result;

    switch (request.change)
    {
    case Training::QuestionChange::Next:
        if (m_currentQuestionIndex < m_questionDatas.count() - 1)
        {
            m_currentQuestionIndex++;
        }
        else
        {
            result.code = Error::Code::OutOfBoundsStart;
        }
        break;

    case Training::QuestionChange::Previous:
        if (m_currentQuestionIndex > 0)
        {
            m_currentQuestionIndex--;
        }
        else
        {
            result.code = Error::Code::OutOfBoundsEnd;
        }
        break;
    }
    emit changeQuestionFinished(result);

    handleQuestions();
}

void TrainingHandler::createQuestions()
{
    for (const auto &vocabData : m_datas)
    {
        for (auto it = vocabData.entries.cbegin(); it != vocabData.entries.cend(); it++)
        {
            m_questionDatas.append(createQuestion(vocabData.title, QString(), it.key(), it.value()));
        }

        for (auto groupIt = vocabData.groupEntries.cbegin(); groupIt != vocabData.groupEntries.cend(); groupIt++)
        {
            for (auto it = groupIt.value().cbegin(); it != groupIt.value().cend(); it++)
            {
                m_questionDatas.append(createQuestion(vocabData.title, groupIt.key(), it.key(), it.value()));
            }
        }
    }
}

Training::QuestionWithVocabContext TrainingHandler::createQuestion(const QString &vocabTitle, const QString &groupTitle, const QString &question, const QSet<QString> &correctAnswer)
{
    Training::Context context;
    context.round = m_round;
    context.totalEntryCount = m_totalEntryCount;
    context.correctAnswerCount = m_correctAnswerCount;

    Training::QuestionData questionData;
    questionData.context = context;
    questionData.number = m_questionDatas.count() + 1;
    questionData.state = Training::QuestionState::Unanswered;
    questionData.question = question;
    questionData.correctAnswer = correctAnswer;
    questionData.isCorrect = false;

    Training::QuestionWithVocabContext questionWithVocabContext;
    questionWithVocabContext.vocabTitle = vocabTitle;
    questionWithVocabContext.groupTitle = groupTitle;
    questionWithVocabContext.questionData = questionData;

    return questionWithVocabContext;
}

void TrainingHandler::handleQuestions()
{
    emit currentQuestion(m_questionDatas.at(m_currentQuestionIndex).questionData);
}
