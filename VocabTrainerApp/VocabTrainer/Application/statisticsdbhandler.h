#ifndef STATISTICSDBHANDLER_H
#define STATISTICSDBHANDLER_H

#include "Logging/logger.h"
#include "DataTypes/statistics.h"

#include <QSqlDatabase>

class StatisticsDBHandler
{
public:
    StatisticsDBHandler(Logger &logger);
    ~StatisticsDBHandler();

    Statistics::InitializeDatabaseResult initializeDatabase(const Statistics::InitializeDatabaseRequest &request);
    Statistics::SaveTrainingResult saveTraining(const Statistics::SaveTrainingRequest &request);
    QList<Training::VocabEntryStat> getTopBestAllTime(int limit);
    QList<Training::VocabEntryStat> getTopWorstAllTime(int limit);

private:
    bool isDatabaseAvailable();
    Error::Code connectToDatabase(const QString &path);
    Error::Code createTables();
    bool createWordsTable();
    bool createCorrectAnswersTable();
    bool createTrainingSessionsTable();
    bool createTrainingWordsTable();
    bool createTrainingRoundsTable();
    qint64 insertTrainingSession(const Training::SessionData &data);
    qint64 getOrInsertWord(const QString &word, const Training::VocabContext &vocabContext);
    qint64 insertTrainingWord(qint64 trainingId, qint64 wordId, const Training::QuestionData &data);
    qint64 insertTrainingRound(qint64 trainingWordId, const Training::RoundContext &context);
    bool insertCorrectAnswer(qint64 wordId, const QString &answer);
    QList<Training::VocabEntryStat> getTopAllTime(int limit, bool best);

    Logger &m_logger;
    QSqlDatabase m_database;
};

#endif // STATISTICSDBHANDLER_H
