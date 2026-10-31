#ifndef TRAINING_H
#define TRAINING_H

#include "error.h"
#include "vocab.h"

#include <QString>
#include <QList>
#include <QSet>
#include <QDateTime>

namespace Training
{
    struct VocabEntry
    {
        QString vocabTitle;
        QString groupTitle;
        QString entry;
    };

    struct VocabEntryStat
    {
        VocabEntry entry;
        int correctAnswerCount = 0;
        int answerCount = 0;
    };

    struct RankingInfo
    {
        QList<VocabEntryStat> topBest;
        QList<VocabEntryStat> topWorst;
        QList<VocabEntryStat> topBestAllTime;
        QList<VocabEntryStat> topWorstAllTime;
    };

    struct SessionData
    {
        QDateTime startedAt;
        qint64 durationMs = 0;
        int roundCount = 0;
        int questionCount = 0;
        int totalEntryCount = 0;

        QMap<QString, QSet<QString>> vocabs;

        int answerCount = 0;
        int correctAnswerCount = 0;
        int wrongAnswerCount = 0;

        int longestCorrectStreak = 0;
        QList<int> correctAnswerCountPerRound;
        int remainsWrongOrUnansweredCount = 0;

        RankingInfo rankingInfo;
    };

    struct StartRequest
    {
        QList<Vocab::Filter> vocabFilters;
        // TODO: Heurestics settings?
    };

    struct FinishedResult
    {
        Error::Code code = Error::Code::Success;
    };

    struct ContinueRequest
    {
        // TODO: Heurestics settings?
    };

    enum class StopCommand
    {
        Stop,
        SaveAndStop,
        OnlyStop,
        ForcedStop
    };

    struct StopRequest
    {
        StopCommand command = StopCommand::Stop;
    };

    enum class StopFailureReason
    {
        None,
        UnsavedData,
        SaveFailed,
        StopFailed
    };

    struct StopResult
    {
        Error::Code code = Error::Code::Success;
        StopFailureReason reason = StopFailureReason::None;
        SessionData sessionData;
    };

    struct VocabContext
    {
        QString vocabTitle;
        QString groupTitle;
    };

    enum class QuestionState
    {
        Unanswered,
        Answered,
        Correct,
        Wrong
    };

    struct RoundContext
    {
        int roundNumber;
        int questionNumber;
        int roundEntryCount;
        QuestionState state;
        QString answer;
    };

    struct QuestionData
    {
        int index;
        VocabContext vocabContext;
        QList<RoundContext> roundContexts;
        QString question;
        QSet<QString> correctAnswer;
        int wrongCount;
        int correctCount;
    };

    struct TrainingContext
    {
        int totalEntryCount;
        int correctAnswerCount;
    };

    struct Question
    {
        int index;
        TrainingContext trainingContext;
        RoundContext roundContext;
        QString question;
        QSet<QString> correctAnswer;
    };

    struct Answer
    {
        QuestionState state;
        QString answer;
    };

    enum class ChangeQuestion
    {
        Next,
        Previous,
        New
    };

    enum class QuestionOrder
    {
        First,
        Middle,
        Last
    };

    struct ChangeQuestionRequest
    {
        ChangeQuestion change;
    };

    struct ChangeQuestionResult
    {
        Error::Code code = Error::Code::Success;
        QuestionOrder order;
    };

    struct HistoryInfo
    {
        int questionIndex;
        int roundIndex;
    };
}

#endif // TRAINING_H
