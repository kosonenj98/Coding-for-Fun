#include "statisticsservice.h"

#include <QtConcurrent/QtConcurrentRun>
#include <QFuture>

namespace
{
    const QString &logTag()
    {
        static const QString tag = QStringLiteral("StatisticsService");
        return tag;
    }
}

StatisticsService::StatisticsService(Logger &logger, QObject *parent)
    : QObject{parent}, m_logger(logger), m_analyzer(logger)
{
    m_logger.debug(logTag(), QStringLiteral("Initializing statistics service..."));
    m_logger.debug(logTag(), QStringLiteral("Initializing statistics service done!"));
}

void StatisticsService::handleAnalyzeTrainingRequest(const Statistics::AnalyzeTrainingRequest &request)
{
    m_logger.debug(logTag(), QStringLiteral("Analyzing training session..."));

    Statistics::AnalyzeTrainingResult result;
    int top = 5; // TODO: Setting?

    // Start independent analyses concurrently
    auto roundCountFuture = QtConcurrent::run([this, &request]() { return m_analyzer.getRoundCount(request.data); });
    auto questionCountFuture = QtConcurrent::run([this, &request]() { return m_analyzer.getQuestionCount(request.data); });
    auto totalEntryCountFuture = QtConcurrent::run([this, &request]() { return m_analyzer.getTotalEntryCount(request.data); });
    auto vocabsFuture = QtConcurrent::run([this, &request]() { return m_analyzer.getVocabs(request.data); });
    auto answerCountFuture = QtConcurrent::run([this, &request]() { return m_analyzer.getAnswerCount(request.data); });
    auto correctAnswerCountFuture = QtConcurrent::run([this, &request]() { return m_analyzer.getCorrectAnswerCount(request.data); });
    auto wrongAnswerCountFuture = QtConcurrent::run([this, &request]() { return m_analyzer.getWrongAnswerCount(request.data); });
    auto correctAnswerCountPerRoundFuture = QtConcurrent::run([this, &request]() { return m_analyzer.getCorrectAnswerCountPerRound(request.data); });
    auto remainsWrongOrUnansweredCountFuture = QtConcurrent::run([this, &request]() { return m_analyzer.getRemainsWrongOrUnansweredCount(request.data); });
    auto topBestFuture = QtConcurrent::run([this, &request, &top]() { return m_analyzer.getTopBest(request.data, top); });
    auto topWorstFuture = QtConcurrent::run([this, &request, &top]() { return m_analyzer.getTopWorst(request.data, top); });

    Training::RankingInfo info;
    info.topBest = topBestFuture.result();
    info.topWorst = topWorstFuture.result();
    // TODO: Construct all time top best and top worst

    Training::SessionData data;
    data.startedAt = request.startedAt;
    data.durationMs = request.durationMs;
    data.roundCount = roundCountFuture.result();
    data.questionCount = questionCountFuture.result();
    data.totalEntryCount = totalEntryCountFuture.result();
    data.vocabs = vocabsFuture.result();
    data.answerCount = answerCountFuture.result();
    data.correctAnswerCount = correctAnswerCountFuture.result();
    data.wrongAnswerCount = wrongAnswerCountFuture.result();
    data.longestCorrectStreak = request.longestCorrectStreak;
    data.correctAnswerCountPerRound = correctAnswerCountPerRoundFuture.result();
    data.remainsWrongOrUnansweredCount = remainsWrongOrUnansweredCountFuture.result();

    data.rankingInfo = info;

    result.trainingSessionData = data;

    m_logger.debug(logTag(), QStringLiteral("Analyzing training session done!"));
    emit analyzeTrainingFinished(result);
}
