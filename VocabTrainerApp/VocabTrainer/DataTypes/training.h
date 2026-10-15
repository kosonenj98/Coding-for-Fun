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

    struct Context
    {
        int round;
        int totalEntryCount;
        int correctAnswerCount;
    };

    enum class QuestionState
    {
        Unanswered,
        Answered
    };

    struct QuestionData
    {
        Context context;
        int number;
        QuestionState state;
        QString question;
        QString answer;
        QSet<QString> correctAnswer;
        bool isCorrect;
    };

    struct QuestionWithVocabContext
    {
        QuestionData questionData;
        QString vocabTitle;
        QString groupTitle;
    };

    enum class QuestionChange
    {
        Next,
        Previous
    };

    struct QuestionChangeRequest
    {
        QuestionChange change;
    };

    struct QuestionChangeResult
    {
        Error::Code code = Error::Code::Success;
    };
}

#endif // TRAINING_H
