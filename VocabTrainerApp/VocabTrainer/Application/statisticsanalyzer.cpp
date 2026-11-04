#include "statisticsanalyzer.h"

namespace
{
    const QString &logTag()
    {
        static const QString tag = QStringLiteral("StatisticsAnalyzer");
        return tag;
    }
}

StatisticsAnalyzer::StatisticsAnalyzer(Logger &logger)
    : m_logger(logger)
{
    m_logger.debug(logTag(), QStringLiteral("Initializing statistics analyzer..."));
    m_logger.debug(logTag(), QStringLiteral("Initializing statistics analyzer done!"));
}

int StatisticsAnalyzer::getRoundCount(const QList<Training::QuestionData> &data)
{
    m_logger.debug(logTag(), QStringLiteral("Getting round count..."));
    int result = 0;
    for (const auto &questionData : data)
    {
        for (const auto &roundContext : questionData.roundContexts)
        {
            result = qMax(result, roundContext.roundNumber);
        }
    }
    m_logger.debug(logTag(), QStringLiteral("Getting round count done!"));
    return result;
}

int StatisticsAnalyzer::getQuestionCount(const QList<Training::QuestionData> &data)
{
    m_logger.debug(logTag(), QStringLiteral("Getting question count..."));
    int result = 0;
    for (const auto &questionData : data)
    {
        result += questionData.roundContexts.count();
    }
    m_logger.debug(logTag(), QStringLiteral("Getting question count done!"));
    return result;
}

int StatisticsAnalyzer::getTotalEntryCount(const QList<Training::QuestionData> &data)
{
    m_logger.debug(logTag(), QStringLiteral("Getting total entry count..."));
    int result = data.count();
    m_logger.debug(logTag(), QStringLiteral("Getting total entry count done!"));
    return result;
}

QMap<QString, QSet<QString>> StatisticsAnalyzer::getVocabs(const QList<Training::QuestionData> &data)
{
    m_logger.debug(logTag(), QStringLiteral("Getting vocabs..."));
    QMap<QString, QSet<QString>> result;
    for (const auto &questionData : data)
    {
        result[questionData.vocabContext.vocabTitle].unite({questionData.vocabContext.groupTitle});
    }
    m_logger.debug(logTag(), QStringLiteral("Getting vocabs done!"));
    return result;
}

int StatisticsAnalyzer::getAnswerCount(const QList<Training::QuestionData> &data)
{
    m_logger.debug(logTag(), QStringLiteral("Getting answered count..."));
    int result = 0;
    for (const auto &questionData : data)
    {
        for (const auto &roundContext : questionData.roundContexts)
        {
            if (roundContext.state != Training::QuestionState::Unanswered)
            {
                result++;
            }
        }
    }
    m_logger.debug(logTag(), QStringLiteral("Getting answered count done!"));
    return result;
}

int StatisticsAnalyzer::getCorrectAnswerCount(const QList<Training::QuestionData> &data)
{
    m_logger.debug(logTag(), QStringLiteral("Getting correct answer count..."));
    int result = 0;
    for (const auto &questionData : data)
    {
        for (const auto &roundContext : questionData.roundContexts)
        {
            if (roundContext.state == Training::QuestionState::Correct)
            {
                result++;
            }
        }
    }
    m_logger.debug(logTag(), QStringLiteral("Getting correct answer count done!"));
    return result;
}

int StatisticsAnalyzer::getWrongAnswerCount(const QList<Training::QuestionData> &data)
{
    m_logger.debug(logTag(), QStringLiteral("Getting wrong answer count..."));
    int result = 0;
    for (const auto &questionData : data)
    {
        for (const auto &roundContext : questionData.roundContexts)
        {
            if (roundContext.state == Training::QuestionState::Wrong)
            {
                result++;
            }
        }
    }
    m_logger.debug(logTag(), QStringLiteral("Getting wrong answer count done!"));
    return result;
}

QList<int> StatisticsAnalyzer::getCorrectAnswerCountPerAttempt(const QList<Training::QuestionData> &data)
{
    m_logger.debug(logTag(), QStringLiteral("Getting correct answer count per attempt..."));
    QList<int> result;
    int maxAttemptCount = getRoundCount(data);
    for (int attemptNumber = 1; attemptNumber < maxAttemptCount + 1; attemptNumber++)
    {
        int correctAnswerCount = 0;
        for (const auto &questionData : data)
        {
            int firstCorrectAttempt = -1;
            for (const auto &roundContext : questionData.roundContexts)
            {
                if (roundContext.state == Training::QuestionState::Correct)
                {
                    if (firstCorrectAttempt == -1 || roundContext.roundNumber < firstCorrectAttempt)
                    {
                        firstCorrectAttempt = roundContext.roundNumber;
                    }
                }
            }
            if (firstCorrectAttempt == attemptNumber)
            {
                correctAnswerCount++;
            }
        }
        result.append(correctAnswerCount);
    }
    m_logger.debug(logTag(), QStringLiteral("Getting correct answer count per attempt done!"));
    return result;
}

int StatisticsAnalyzer::getRemainsWrongOrUnansweredCount(const QList<Training::QuestionData> &data)
{
    m_logger.debug(logTag(), QStringLiteral("Getting wrong answer count..."));
    int result = 0;
    for (const auto &questionData : data)
    {
        const auto &lastRoundContext = questionData.roundContexts.last();
        if (lastRoundContext.state != Training::QuestionState::Correct)
        {
            result++;
        }
    }
    m_logger.debug(logTag(), QStringLiteral("Getting wrong answer count done!"));
    return result;
}

QList<int> StatisticsAnalyzer::getCorrectAnswerCountPerRound(const QList<Training::QuestionData> &data)
{
    m_logger.debug(logTag(), QStringLiteral("Getting correct answer count per round..."));
    QList<int> result;
    int maxRoundCount = getRoundCount(data);
    for (int roundNumber = 1; roundNumber < maxRoundCount + 1; roundNumber++)
    {
        int correctAnswerCount = 0;
        for (const auto &questionData : data)
        {
            for (const auto &roundContext : questionData.roundContexts)
            {
                if (roundContext.roundNumber == roundNumber && roundContext.state == Training::QuestionState::Correct)
                {
                    correctAnswerCount++;
                }
            }
        }
        result.append(correctAnswerCount);
    }
    m_logger.debug(logTag(), QStringLiteral("Getting correct answer count per round done!"));
    return result;
}

QList<int> StatisticsAnalyzer::getAnswerCountPerRound(const QList<Training::QuestionData> &data)
{
    m_logger.debug(logTag(), QStringLiteral("Getting answer count per round..."));
    QList<int> result;
    int maxRoundCount = getRoundCount(data);
    for (int roundNumber = 1; roundNumber < maxRoundCount + 1; roundNumber++)
    {
        int answerCount = 0;
        for (const auto &questionData : data)
        {
            for (const auto &roundContext : questionData.roundContexts)
            {
                if (roundContext.roundNumber == roundNumber)
                {
                    answerCount++;
                }
            }
        }
        result.append(answerCount);
    }
    m_logger.debug(logTag(), QStringLiteral("Getting answer count per round done!"));
    return result;
}

QList<Training::VocabEntryStat> StatisticsAnalyzer::getTopBest(const QList<Training::QuestionData> &data, int top)
{
    m_logger.debug(logTag(), QStringLiteral("Getting top %1 best...").arg(QString::number(top)));
    QList<Training::VocabEntryStat> allVocabEntryStats = constructAllVocabEntryStats(data);
    std::sort(allVocabEntryStats.begin(), allVocabEntryStats.end(),
              [] (const Training::VocabEntryStat &a, const Training::VocabEntryStat &b)
              {
                if (a.answerCount == 0 || b.answerCount == 0)
                {
                    if (a.answerCount == 0 && b.answerCount == 0)
                    {
                        return false;
                    }
                    return a.answerCount != 0;
                }

                // a/b > c/d  <=> ad > bc
                const qint64 left = static_cast<qint64>(a.correctAnswerCount) * b.answerCount;
                const qint64 right = static_cast<qint64>(b.correctAnswerCount) * a.answerCount;

                if (left != right)
                {
                    return left > right;
                }

                return a.answerCount > b.answerCount;
              });
    m_logger.debug(logTag(), QStringLiteral("Getting top %1 best done!").arg(QString::number(top)));
    return allVocabEntryStats.sliced(0, qMin(top, allVocabEntryStats.count()));
}

QList<Training::VocabEntryStat> StatisticsAnalyzer::getTopWorst(const QList<Training::QuestionData> &data, int top)
{
    m_logger.debug(logTag(), QStringLiteral("Getting top %1 worst...").arg(QString::number(top)));
    QList<Training::VocabEntryStat> allVocabEntryStats = constructAllVocabEntryStats(data);
    std::sort(allVocabEntryStats.begin(), allVocabEntryStats.end(),
              [] (const Training::VocabEntryStat &a, const Training::VocabEntryStat &b)
              {
                  if (a.answerCount == 0 || b.answerCount == 0)
                  {
                      if (a.answerCount == 0 && b.answerCount == 0)
                      {
                          return false;
                      }
                      return a.answerCount == 0;
                  }

                  // a/b > c/d  <=> ad > bc
                  const qint64 left = static_cast<qint64>(a.correctAnswerCount) * b.answerCount;
                  const qint64 right = static_cast<qint64>(b.correctAnswerCount) * a.answerCount;

                  if (left != right)
                  {
                      return left < right;
                  }

                  return a.answerCount < b.answerCount;
              });
    m_logger.debug(logTag(), QStringLiteral("Getting top %1 worst done!").arg(QString::number(top)));
    return allVocabEntryStats.sliced(0, qMin(top, allVocabEntryStats.count()));
}

QList<Training::VocabEntryStat> StatisticsAnalyzer::constructAllVocabEntryStats(const QList<Training::QuestionData> &data)
{
    QList<Training::VocabEntryStat> result;
    for (const auto &questionData : data)
    {
        Training::VocabEntry entry;
        entry.vocabTitle = questionData.vocabContext.vocabTitle;
        entry.groupTitle = questionData.vocabContext.groupTitle;
        entry.entry = questionData.question;

        Training::VocabEntryStat stat;
        stat.entry = entry;
        stat.answerCount = questionData.correctCount + questionData.wrongCount;
        stat.correctAnswerCount = questionData.correctCount;

        result.append(stat);
    }
    return result;
}
