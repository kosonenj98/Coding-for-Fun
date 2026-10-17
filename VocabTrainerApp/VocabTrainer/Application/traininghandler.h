#ifndef TRAININGHANDLER_H
#define TRAININGHANDLER_H

#include "Logging/logger.h"
#include "DataTypes/training.h"
#include "DataTypes/vocab.h"

#include <QObject>

class TrainingHandler : public QObject
{
    Q_OBJECT
public:
    explicit TrainingHandler(Logger &logger, QObject *parent = nullptr);

    void startTraining(const Training::StartRequest &request);
    void handleAnswer(const Training::Answer &answerData);
    void changeQuestion(const Training::ChangeQuestionRequest &request);

public slots:
    void handleGetDataFinished(const Vocab::GetDataResult &result);

signals:
    void requestGetData(const Vocab::GetDataRequest &request);
    void question(const Training::Question &question);
    void changeQuestionFinished(const Training::ChangeQuestionResult &result);
    void nextQuestionAvailable(bool available);
    void trainingFinished(const Training::Result &result);

private:
    void createQuestionDatas(const QList<Vocab::Data> &datas);
    Training::QuestionData createQuestionData(const QString &vocabTitle, const QString &groupTitle, const QString &question, const QSet<QString> &correctAnswer);
    Training::Question createQuestion(const Training::QuestionData &questionData, int roundIndex = -1);
    void handleTraining();
    void handleQuestion();
    void newQuestion();
    void handleNextRound(const QList<int> &questionDataIndices);
    bool allAnswered();
    int getCorrectAnswerCount();
    const QList<int> getQuestionIndicesForWrongAnswers();

    Logger &m_logger;

    int m_totalEntryCount;
    QList<Training::QuestionData> m_questionDatas;
    int m_currentQuestionIndex;
    int m_newQuestionIndex;

    QList<Training::HistoryInfo> m_trainingHistory;
    bool m_trainingFinished;
};

#endif // TRAININGHANDLER_H
