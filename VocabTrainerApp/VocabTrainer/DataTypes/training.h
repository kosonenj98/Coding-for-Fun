#ifndef TRAINING_H
#define TRAINING_H

#include "error.h"
#include "vocab.h"

#include <QString>
#include <QList>
#include <QSet>

namespace Training
{
    struct StartRequest
    {
        QList<Vocab::Filter> vocabFilters;
    };

    struct Result
    {
        Error::Code code = Error::Code::Success;
    };

    struct VocabContext
    {
        QString vocabTitle;
        QString groupTitle;
    };

    enum class QuestionState
    {
        Unanswered,
        Answered
    };

    struct RoundContext
    {
        int roundNumber;
        int questionNumber;
        int roundEntryCount;
        QuestionState state;
        QString answer;
        bool isCorrect;
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
