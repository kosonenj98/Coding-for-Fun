#ifndef STATISTICSANALYZER_H
#define STATISTICSANALYZER_H

#include "Logging/logger.h"
#include "DataTypes/training.h"

class StatisticsAnalyzer
{
public:
    StatisticsAnalyzer(Logger &logger);

    int getRoundCount(const QList<Training::QuestionData> &data);
    int getQuestionCount(const QList<Training::QuestionData> &data);
    int getTotalEntryCount(const QList<Training::QuestionData> &data);
    QMap<QString, QSet<QString>> getVocabs(const QList<Training::QuestionData> &data);
    int getAnswerCount(const QList<Training::QuestionData> &data);
    int getCorrectAnswerCount(const QList<Training::QuestionData> &data);
    int getWrongAnswerCount(const QList<Training::QuestionData> &data);
    QList<int> getCorrectAnswerCountPerAttempt(const QList<Training::QuestionData> &data);
    int getRemainsWrongOrUnansweredCount(const QList<Training::QuestionData> &data);
    QList<int> getCorrectAnswerCountPerRound(const QList<Training::QuestionData> &data);
    QList<int> getAnswerCountPerRound(const QList<Training::QuestionData> &data);
    QList<Training::VocabEntryStat> getTopBest(const QList<Training::QuestionData> &data, int top);
    QList<Training::VocabEntryStat> getTopWorst(const QList<Training::QuestionData> &data, int top);

private:
    Logger &m_logger;

    QList<Training::VocabEntryStat> constructAllVocabEntryStats(const QList<Training::QuestionData> &data);
};

#endif // STATISTICSANALYZER_H
