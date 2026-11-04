#include "statisticsdbhandler.h"

#include <QDir>
#include <QSqlError>
#include <QSqlQuery>

namespace
{
    const QString &logTag()
    {
        static const QString tag = QStringLiteral("StatisticsDBHandler");
        return tag;
    }
}

StatisticsDBHandler::StatisticsDBHandler(Logger &logger)
    : m_logger(logger)
{
    m_logger.debug(logTag(), QStringLiteral("Initializing statistics database handler..."));
    m_logger.debug(logTag(), QStringLiteral("Initializing statistics database handler done!"));
}

StatisticsDBHandler::~StatisticsDBHandler()
{
    if (m_database.isOpen())
    {
        m_database.close();
    }
}

Statistics::InitializeDatabaseResult StatisticsDBHandler::initializeDatabase(const Statistics::InitializeDatabaseRequest &request)
{
    Statistics::InitializeDatabaseResult result;
    result.path = request.path;

    Error::Code code = connectToDatabase(request.path);
    if (code != Error::Code::Success)
    {
        m_logger.error(logTag(), QStringLiteral("Failed to connect to database '%1'. ErrorCode %2").arg(request.path, Error::codeToQString(code)));
        result.code = code;
        return result;
    }

    code = createTables();
    if (code != Error::Code::Success)
    {
        m_logger.error(logTag(), QStringLiteral("Failed to create database tables. ErrorCode %1").arg(Error::codeToQString(code)));
        result.code = code;
        return result;
    }

    return result;
}

Statistics::SaveTrainingResult StatisticsDBHandler::saveTraining(const Statistics::SaveTrainingRequest &request)
{
    m_logger.debug(logTag(), QStringLiteral("Saving training to database..."));
    Statistics::SaveTrainingResult result;

    if (!m_database.transaction())
    {
        m_logger.error(logTag(), QStringLiteral("Failed to begin transaction for database! Possible reason: '%1'").arg(m_database.lastError().text()));
        result.code = Error::Code::CannotBeginTransaction;
        return result;
    }

    qint64 trainingId = insertTrainingSession(request.sessionData);
    result.trainingNumber = trainingId;
    if (trainingId < 0)
    {
        result.code = Error::Code::CannotSaveToDB;
    }
    else
    {
        for (const auto &questionData : request.questionDatas)
        {


            qint64 wordId = getOrInsertWord(questionData.question, questionData.vocabContext);
            if (wordId < 0)
            {
                result.code = Error::Code::CannotSaveToDB;
                break;
            }

            for (const auto &answer : questionData.correctAnswer)
            {
                if (!insertCorrectAnswer(wordId, answer))
                {
                    result.code = Error::Code::CannotSaveToDB;
                    break;
                }
            }
            if (result.code != Error::Code::Success)
            {
                break;
            }

            qint64 trainingWordId = insertTrainingWord(trainingId, wordId, questionData);
            if (trainingWordId < 0)
            {
                result.code = Error::Code::CannotSaveToDB;
                break;
            }

            for (const auto &roundContext : questionData.roundContexts)
            {
                if (insertTrainingRound(trainingWordId, roundContext) < 0)
                {
                    result.code = Error::Code::CannotSaveToDB;
                    break;
                }
            }
            if (result.code != Error::Code::Success)
            {
                break;
            }
        }
    }

    if (result.code != Error::Code::Success || !m_database.commit())
    {
        if (result.code == Error::Code::Success)
        {
            m_logger.error(logTag(), QStringLiteral("Failed to commit save to database! Possible reason: '%1'").arg(m_database.lastError().text()));
            result.code = Error::Code::CannotCommitDBTransaction;
        }

        m_logger.info(logTag(), QStringLiteral("Rolling back saving database due to error..."));
        if (!m_database.rollback())
        {
            m_logger.error(logTag(), QStringLiteral("Failed to rollback save database! Possible reason: '%1'").arg(m_database.lastError().text()));
        }
        else
        {
            m_logger.info(logTag(), QStringLiteral("Rolling back save database succeeded!"));
        }
    }

    m_logger.debug(logTag(), QStringLiteral("Saving training to database done!"));
    return result;
}

QList<Training::VocabEntryStat> StatisticsDBHandler::getTopBestAllTime(int limit)
{
    m_logger.debug(logTag(), QStringLiteral("Getting top %1 all time best words...").arg(QString::number(limit)));
    bool best = true;
    QList<Training::VocabEntryStat> result = getTopAllTime(limit, best);
    m_logger.debug(logTag(), QStringLiteral("Getting top %1 all time best words done!").arg(QString::number(limit)));
    return result;
}

QList<Training::VocabEntryStat> StatisticsDBHandler::getTopWorstAllTime(int limit)
{
    m_logger.debug(logTag(), QStringLiteral("Getting top %1 all time worst words...").arg(QString::number(limit)));
    bool worst = false;
    QList<Training::VocabEntryStat> result = getTopAllTime(limit, worst);
    m_logger.debug(logTag(), QStringLiteral("Getting top %1 all time worst words done!").arg(QString::number(limit)));
    return result;
}

bool StatisticsDBHandler::isDatabaseAvailable()
{
    return m_database.isOpen();
}

Error::Code StatisticsDBHandler::connectToDatabase(const QString &path)
{
    const QString databasePath = path;

    if (databasePath.isEmpty())
    {
        m_logger.error(logTag(), QStringLiteral("Cannot initialize database with empty path!"));
        return Error::Code::EmptyDBPath;
    }

    if (isDatabaseAvailable())
    {
        return Error::Code::DBAlreadyInitialized;
    }

    const QFileInfo databaseInfo(databasePath);
    const QString databaseDirectory = databaseInfo.absolutePath();
    if (!QDir().mkpath(databaseDirectory))
    {
        const QFileInfo dirInfo(databaseDirectory);
        m_logger.error(logTag(), QStringLiteral("Failed to create database directory '%1'. Exists: %2, Is directory: %3, Parent writable: %4")
                                     .arg(databaseDirectory)
                                     .arg(dirInfo.exists())
                                     .arg(dirInfo.isDir())
                                     .arg(dirInfo.isWritable()));
        return Error::Code::CannotMakeDBDir;
    }

    m_database = QSqlDatabase::addDatabase("QSQLITE", "StatisticsConnection");
    m_database.setDatabaseName(databasePath);

    if (!m_database.open())
    {
        m_logger.error(logTag(), QStringLiteral("Failed to open database '%1': '%2'").arg(databasePath, m_database.lastError().text()));
        return Error::Code::CannotOpenDB;
    }

    // Enable foreign key enforcement
    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral("PRAGMA foreign_keys = ON")))
    {
        m_logger.error(logTag(), QStringLiteral("Failed to enable foreign keys: %1").arg(query.lastError().text()));
        m_database.close();
        return Error::Code::CannotEnableForeignDBKeys;
    }

    m_logger.info(logTag(), QStringLiteral("Successfully opened database '%1'").arg(databasePath));
    return Error::Code::Success;
}

Error::Code StatisticsDBHandler::createTables()
{
    m_logger.debug(logTag(), QStringLiteral("Creating data base tables..."));
    if (!m_database.transaction())
    {
        m_logger.error(logTag(), QStringLiteral("Cannot begin transaction on data base! Possible reason: '%1'").arg(m_database.lastError().text()));
        return Error::Code::CannotBeginTransaction;
    }

    Error::Code code = Error::Code::CannotCreateDBTables;

    if (!createWordsTable())
    {
        m_logger.error(logTag(), QStringLiteral("Failed to create words table!"));
    }
    else if (!createCorrectAnswersTable())
    {
        m_logger.error(logTag(), QStringLiteral("Failed to create correct answers table!"));
    }
    else if (!createTrainingSessionsTable())
    {
        m_logger.error(logTag(), QStringLiteral("Failed to create training sessions table!"));
    }
    else if (!createTrainingWordsTable())
    {
        m_logger.error(logTag(), QStringLiteral("Failed to create training words table!"));
    }
    else if (!createTrainingRoundsTable())
    {
        m_logger.error(logTag(), QStringLiteral("Failed to create training rounds table!"));
    }
    else
    {
        m_logger.info(logTag(), QStringLiteral("All database tables created successfully!"));
        code = Error::Code::Success;
    }

    if (code != Error::Code::Success || !m_database.commit())
    {
        if (code == Error::Code::Success)
        {
            m_logger.error(logTag(), QStringLiteral("Failed to commit to database! Possible reason: '%1'").arg(m_database.lastError().text()));
            code = Error::Code::CannotCommitDBTransaction;
        }

        m_logger.info(logTag(), QStringLiteral("Rolling back database due to error..."));
        if (!m_database.rollback())
        {
            m_logger.error(logTag(), QStringLiteral("Failed to rollback database! Possible reason: '%1'").arg(m_database.lastError().text()));
        }
        else
        {
            m_logger.info(logTag(), QStringLiteral("Rolling back database succeeded!"));
        }
    }

    m_logger.debug(logTag(), QStringLiteral("Creating data base tables done!"));
    return code;
}

bool StatisticsDBHandler::createWordsTable()
{
    QSqlQuery query(m_database);

    const QString sql = QStringLiteral(
        R"(
            CREATE TABLE IF NOT EXISTS words (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                word TEXT NOT NULL,
                vocab_title TEXT NOT NULL,
                group_title TEXT NOT NULL,
                UNIQUE (word, vocab_title, group_title)
            )
        )"
    );

    if (!query.exec(sql))
    {
        m_logger.error(logTag(), QStringLiteral("Failed to create words table: %1").arg(query.lastError().text()));
        return false;
    }

    return true;
}

bool StatisticsDBHandler::createCorrectAnswersTable()
{
    QSqlQuery query(m_database);

    const QString sql = QStringLiteral(
        R"(
            CREATE TABLE IF NOT EXISTS correct_answers (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                word_id INTEGER NOT NULL,
                answer TEXT NOT NULL,
                FOREIGN KEY (word_id) REFERENCES words(id),
                UNIQUE (word_id, answer)
            )
        )"
    );

    if (!query.exec(sql))
    {
        m_logger.error(logTag(), QStringLiteral("Failed to create correct_answers table: %1").arg(query.lastError().text()));
        return false;
    }

    return true;
}

bool StatisticsDBHandler::createTrainingSessionsTable()
{
    QSqlQuery query(m_database);

    const QString sql = QStringLiteral(
        R"(
            CREATE TABLE IF NOT EXISTS training_sessions (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                started_at TEXT NOT NULL,
                duration_ms INTEGER NOT NULL,
                round_count INTEGER NOT NULL,
                question_count INTEGER NOT NULL,
                total_entry_count INTEGER NOT NULL,
                answer_count INTEGER NOT NULL,
                correct_answer_count INTEGER NOT NULL,
                wrong_answer_count INTEGER NOT NULL,
                longest_correct_streak INTEGER NOT NULL,
                remains_wrong_or_unanswered_count INTEGER NOT NULL
            )
        )"
    );

    if (!query.exec(sql))
    {
        m_logger.error(logTag(), QStringLiteral("Failed to create training_sessions table: %1").arg(query.lastError().text()));
        return false;
    }

    return true;
}

bool StatisticsDBHandler::createTrainingWordsTable()
{
    QSqlQuery query(m_database);

    const QString sql = QStringLiteral(
        R"(
            CREATE TABLE IF NOT EXISTS training_words (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                training_id INTEGER NOT NULL,
                word_id INTEGER NOT NULL,
                correct_count INTEGER NOT NULL DEFAULT 0,
                wrong_count INTEGER NOT NULL DEFAULT 0,
                FOREIGN KEY (training_id) REFERENCES training_sessions(id),
                FOREIGN KEY (word_id) REFERENCES words(id),
                UNIQUE (training_id, word_id)
            )
        )"
    );

    if (!query.exec(sql))
    {
        m_logger.error(logTag(), QStringLiteral("Failed to create training_words table: %1").arg(query.lastError().text()));
        return false;
    }

    return true;
}

bool StatisticsDBHandler::createTrainingRoundsTable()
{
    QSqlQuery query(m_database);

    const QString sql = QStringLiteral(
        R"(
            CREATE TABLE IF NOT EXISTS training_rounds (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                training_word_id INTEGER NOT NULL,
                round_number INTEGER NOT NULL,
                question_number INTEGER NOT NULL,
                state INTEGER NOT NULL,
                answer TEXT,
                FOREIGN KEY (training_word_id) REFERENCES training_words(id)
            )
        )"
    );

    if (!query.exec(sql))
    {
        m_logger.error(logTag(), QStringLiteral("Failed to create training_rounds table: %1").arg(query.lastError().text()));
        return false;
    }

    return true;
}

qint64 StatisticsDBHandler::insertTrainingSession(const Training::SessionData &data)
{
    m_logger.debug(logTag(), QStringLiteral("Saving training session to database..."));
    QSqlQuery query(m_database);

    const QString sql = QStringLiteral(
        R"(
            INSERT INTO training_sessions (
                started_at,
                duration_ms,
                round_count,
                question_count,
                total_entry_count,
                answer_count,
                correct_answer_count,
                wrong_answer_count,
                longest_correct_streak,
                remains_wrong_or_unanswered_count
            )
            VALUES (
                :started_at,
                :duration_ms,
                :round_count,
                :question_count,
                :total_entry_count,
                :answer_count,
                :correct_answer_count,
                :wrong_answer_count,
                :longest_correct_streak,
                :remains_wrong_or_unanswered_count
            )
        )"
    );

    if (!query.prepare(sql))
    {
        m_logger.error(logTag(), QStringLiteral("Failed to prepare training session SQL query! Possible reason: '%1'").arg(query.lastError().text()));
        return -1;
    }

    query.bindValue(":started_at", data.startedAt.toString(Qt::ISODateWithMs));
    query.bindValue(":duration_ms", data.durationMs);
    query.bindValue(":round_count", data.roundCount);
    query.bindValue(":question_count", data.questionCount);
    query.bindValue(":total_entry_count", data.totalEntryCount);
    query.bindValue(":answer_count", data.answerCount);
    query.bindValue(":correct_answer_count", data.correctAnswerCount);
    query.bindValue(":wrong_answer_count", data.wrongAnswerCount);
    query.bindValue(":longest_correct_streak", data.longestCorrectStreak);
    query.bindValue(":remains_wrong_or_unanswered_count", data.remainsWrongOrUnansweredCount);

    if (!query.exec())
    {
        m_logger.error(logTag(), QStringLiteral("Failed to insert training session: %1").arg(query.lastError().text()));
        return -1;
    }

    bool ok = false;
    const qint64 id = query.lastInsertId().toLongLong(&ok);
    if (!ok || id < 0)
    {
        m_logger.error(logTag(), QStringLiteral("Failed to retrieve training session ID!"));
        return -1;
    }

    m_logger.debug(logTag(), QStringLiteral("Saving training session to database done!"));
    return id;
}

qint64 StatisticsDBHandler::getOrInsertWord(const QString &word, const Training::VocabContext &vocabContext)
{
    QSqlQuery query(m_database);

    // Check if the word already exists
    const QString selectSql = QStringLiteral(
        R"(
            SELECT id
            FROM words
            WHERE word = :word
              AND vocab_title = :vocab_title
              AND group_title = :group_title
        )"
    );

    if (!query.prepare(selectSql))
    {
        m_logger.error(logTag(), QStringLiteral("Failed to prepare select word SQL query! Possible reason: '%1'").arg(query.lastError().text()));
        return -1;
    }

    query.bindValue(":word", word);
    query.bindValue(":vocab_title", vocabContext.vocabTitle);
    query.bindValue(":group_title", vocabContext.groupTitle);

    if (!query.exec())
    {
        m_logger.error(logTag(), QStringLiteral("Failed to find word: %1").arg(query.lastError().text()));
        return -1;
    }

    // Return existing word ID
    if (query.next())
    {
        return query.value("id").toLongLong();
    }

    // Word does not exist, insert it
    const QString insertSql = QStringLiteral(
        R"(
            INSERT INTO words (
                word,
                vocab_title,
                group_title
            )
            VALUES (
                :word,
                :vocab_title,
                :group_title
            )
        )"
    );

    if (!query.prepare(insertSql))
    {
        m_logger.error(logTag(), QStringLiteral("Failed to prepare insert word SQL query! Possible reason: '%1'").arg(query.lastError().text()));
        return -1;
    }

    query.bindValue(":word", word);
    query.bindValue(":vocab_title", vocabContext.vocabTitle);
    query.bindValue(":group_title", vocabContext.groupTitle);

    if (!query.exec())
    {
        m_logger.error(logTag(),QStringLiteral("Failed to insert word: %1").arg(query.lastError().text()));
        return -1;
    }

    bool ok = false;
    const qint64 id = query.lastInsertId().toLongLong(&ok);
    if (!ok || id < 0)
    {
        m_logger.error(logTag(), QStringLiteral("Failed to retrieve word ID!"));
        return -1;
    }

    return id;
}

qint64 StatisticsDBHandler::insertTrainingWord(qint64 trainingId, qint64 wordId, const Training::QuestionData &data)
{
    QSqlQuery query(m_database);

    const QString sql = QStringLiteral(
        R"(
            INSERT INTO training_words (
                training_id,
                word_id,
                correct_count,
                wrong_count
            )
            VALUES (
                :training_id,
                :word_id,
                :correct_count,
                :wrong_count
            )
        )"
    );

    if (!query.prepare(sql))
    {
        m_logger.error(logTag(), QStringLiteral("Failed to prepare insert training word SQL query! Possible reason: '%1'").arg(query.lastError().text()));
        return -1;
    }

    query.bindValue(":training_id", trainingId);
    query.bindValue(":word_id", wordId);
    query.bindValue(":correct_count", data.correctCount);
    query.bindValue(":wrong_count", data.wrongCount);

    if (!query.exec())
    {
        m_logger.error(logTag(), QStringLiteral("Failed to insert training word: %1").arg(query.lastError().text()));
        return -1;
    }

    bool ok = false;
    const qint64 id = query.lastInsertId().toLongLong(&ok);
    if (!ok || id < 0)
    {
        m_logger.error(logTag(), QStringLiteral("Failed to retrieve training word ID!"));
        return -1;
    }

    return id;
}

qint64 StatisticsDBHandler::insertTrainingRound(qint64 trainingWordId, const Training::RoundContext &context)
{
    QSqlQuery query(m_database);

    const QString sql = QStringLiteral(
        R"(
            INSERT INTO training_rounds (
                training_word_id,
                round_number,
                question_number,
                state,
                answer
            )
            VALUES (
                :training_word_id,
                :round_number,
                :question_number,
                :state,
                :answer
            )
        )"
    );

    if (!query.prepare(sql))
    {
        m_logger.error(logTag(), QStringLiteral("Failed to prepare insert training round SQL query! Possible reason: '%1'").arg(query.lastError().text()));
        return -1;
    }

    query.bindValue(":training_word_id", trainingWordId);
    query.bindValue(":round_number", context.roundNumber);
    query.bindValue(":question_number", context.questionNumber);
    query.bindValue(":state", static_cast<int>(context.state));
    query.bindValue(":answer", context.answer);

    if (!query.exec())
    {
        m_logger.error(logTag(), QStringLiteral("Failed to insert training round: %1").arg(query.lastError().text()));
        return -1;
    }

    bool ok = false;
    const qint64 id = query.lastInsertId().toLongLong(&ok);
    if (!ok || id < 0)
    {
        m_logger.error(logTag(), QStringLiteral("Failed to retrieve training round ID!"));
        return -1;
    }

    return id;
}

bool StatisticsDBHandler::insertCorrectAnswer(qint64 wordId, const QString &answer)
{
    QSqlQuery query(m_database);

    // Check if answer already exists
    const QString selectSql = QStringLiteral(
        R"(
            SELECT id
            FROM correct_answers
            WHERE word_id = :word_id
              AND answer = :answer
        )"
    );

    if (!query.prepare(selectSql))
    {
        m_logger.error(logTag(), QStringLiteral("Failed to prepare select word SQL query! Possible reason: '%1'").arg(query.lastError().text()));
        return false;
    }

    query.bindValue(":word_id", wordId);
    query.bindValue(":answer", answer);

    if (!query.exec())
    {
        m_logger.error(logTag(), QStringLiteral("Failed to find answer: %1").arg(query.lastError().text()));
        return false;
    }

    // Return existing answer ID
    if (query.next())
    {
        return true;
    }

    const QString insertSql = QStringLiteral(
        R"(
            INSERT INTO correct_answers (
                word_id,
                answer
            )
            VALUES (
                :word_id,
                :answer
            )
        )"
    );

    if (!query.prepare(insertSql))
    {
        m_logger.error(logTag(), QStringLiteral("Failed to prepare insert correct answer SQL query! Possible reason: '%1'").arg(query.lastError().text()));
        return false;
    }

    query.bindValue(":word_id", wordId);
    query.bindValue(":answer", answer);

    if (!query.exec())
    {
        m_logger.error(logTag(),QStringLiteral("Failed to insert correct answer: %1").arg(query.lastError().text()));
        return false;
    }

    return true;
}

QList<Training::VocabEntryStat> StatisticsDBHandler::getTopAllTime(int limit, bool best)
{
    QList<Training::VocabEntryStat> result;
    const QString order = best ? QStringLiteral("DESC") : QStringLiteral("ASC");

    if (!(limit > 0))
    {
        m_logger.error(logTag(), QStringLiteral("Invalid limit '%1' in top all time SQL query!").arg(QString::number(limit)));
        return result;
    }

    QSqlQuery query(m_database);

    const QString sql = QStringLiteral(
        R"(
            SELECT
                w.word,
                w.vocab_title,
                w.group_title,
                SUM(tw.correct_count) AS correct_count,
                SUM(tw.correct_count + tw.wrong_count) AS answer_count
            FROM words w
            JOIN training_words tw ON tw.word_id = w.id
            WHERE w.vocab_title NOT LIKE '%debug%'
            GROUP BY w.id
            HAVING answer_count > 0
            ORDER BY
                CAST(correct_count AS REAL) / answer_count %1,
                answer_count %1
            LIMIT :limit
        )"
    ).arg(order);

    if (!query.prepare(sql))
    {
        m_logger.error(logTag(), QStringLiteral("Failed to prepare top all time SQL query: %1").arg(query.lastError().text()));
        return result;
    }

    query.bindValue(":limit", limit);

    if (!query.exec())
    {
        m_logger.error(logTag(), QStringLiteral("Failed to get top all time stats: %1").arg(query.lastError().text()));
        return result;
    }

    while (query.next())
    {
        Training::VocabEntry entry;
        entry.vocabTitle = query.value("vocab_title").toString();
        entry.groupTitle = query.value("group_title").toString();
        entry.entry = query.value("word").toString();

        Training::VocabEntryStat stat;
        stat.entry = entry;
        stat.correctAnswerCount = query.value("correct_count").toInt();
        stat.answerCount = query.value("answer_count").toInt();

        result.append(stat);
    }

    return result;
}
