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

    void handleAnswer(const Training::QuestionData &answerData);

    void changeQuestion(const Training::QuestionChangeRequest &request);

public slots:
    void handleGetDataFinished(const Vocab::GetResult &result);

signals:
    void requestGetData(const Vocab::GetRequest &request);

    void currentQuestion(const Training::QuestionData &question);

    void changeQuestionFinished(const Training::QuestionChangeResult &result);

    void trainingFinished(const Training::Result &result);

private:
    void createQuestions();
    Training::QuestionWithVocabContext createQuestion(const QString &vocabTitle, const QString &groupTitle, const QString &question, const QSet<QString> &correctAnswer);

    void handleQuestions();

    Logger &m_logger;
    int m_round;
    int m_totalEntryCount;
    int m_correctAnswerCount;
    QList<Vocab::Data> m_datas;
    QList<Training::QuestionWithVocabContext> m_questionDatas;
    int m_currentQuestionIndex;
};

#endif // TRAININGHANDLER_H
