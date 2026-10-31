#ifndef TRAININGHANDLER_H
#define TRAININGHANDLER_H

#include "Logging/logger.h"
#include "answerchecker.h"
#include "DataTypes/training.h"
#include "DataTypes/vocab.h"
#include "DataTypes/statistics.h"

#include <QObject>
#include <QElapsedTimer>

class TrainingHandler : public QObject
{
    Q_OBJECT
public:
    explicit TrainingHandler(Logger &logger, QObject *parent = nullptr);

    void startTraining(const Training::StartRequest &request);
    void handleAnswer(const Training::Answer &answerData);
    void changeQuestion(const Training::ChangeQuestionRequest &request);
    void continueTraining(const Training::ContinueRequest &request);
    void stopTraining(const Training::StopRequest &request);

public slots:
    void handleGetDataFinished(const Vocab::GetDataResult &result);
    void handleAnalyzeTrainingFinished(const Statistics::AnalyzeTrainingResult &result);

signals:
    void requestGetData(const Vocab::GetDataRequest &request);
    void question(const Training::Question &question);
    void changeQuestionFinished(const Training::ChangeQuestionResult &result);
    void nextQuestionAvailable(bool available);
    void trainingFinished(const Training::FinishedResult &result);
    void stopTrainingFinished(const Training::StopResult &result);
    void requestAnalyzeTraining(const Statistics::AnalyzeTrainingRequest &request);

private:
    void createQuestionDatas(const QList<Vocab::Data> &datas);
    Training::QuestionData createQuestionData(const QString &vocabTitle, const QString &groupTitle, const QString &question, const QSet<QString> &correctAnswer);
    Training::Question createQuestion(const Training::QuestionData &questionData, int roundIndex = -1);
    void handleTraining();
    void handleQuestion();
    void newQuestion();
    void handleNewRound(const QList<int> &questionDataIndices);
    bool allAnswered();
    int getCorrectAnswerCount();
    const QList<int> getQuestionIndicesForWrongAnswers();
    void handleUnansweredQuestion(const QString &answer);
    void handleAnsweredQuestion();
    void handleTrainingFinished();
    void handleStop();
    void handleSaveAndStop();
    void handleOnlyStop();
    void handleForcedStop();
    void handleAnalyzeTraining(bool saveResults);

    Logger &m_logger;
    AnswerChecker m_answerChecker;

    QDateTime m_startedAt;
    QElapsedTimer m_trainingTimer;
    QList<Training::QuestionData> m_questionDatas;
    QList<Training::HistoryInfo> m_trainingHistory;
    int m_totalEntryCount;
    int m_round;
    int m_newQuestionIndex;
    int m_currentQuestionIndex;
    int m_longestCorrectStreak;
    int m_currentCorrectStreak;
    bool m_trainingFinished;
};

#endif // TRAININGHANDLER_H
