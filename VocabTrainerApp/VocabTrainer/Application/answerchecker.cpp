#include "answerchecker.h"

#include <QRegularExpression>

namespace
{
    const QString &logTag()
    {
        static const QString tag = QStringLiteral("AnswerChecker");
        return tag;
    }
}

AnswerChecker::AnswerChecker(Logger &logger)
    : m_logger(logger)
{
    m_logger.debug(logTag(), QStringLiteral("Initializing AnswerChecker..."));
    m_logger.debug(logTag(), QStringLiteral("Initializing AnswerChecker done!"));
}

bool AnswerChecker::checkAnswer(const QString &answer, const QSet<QString> correctAnswers)
{
    const QString normalizedAnswer = normalizeAnswer(answer);

    for (const QString &correctAnswer : correctAnswers)
    {
        if (normalizedAnswer == normalizeAnswer(correctAnswer))
        {
            return true;
        }
    }

    return false;
}

QString AnswerChecker::normalizeAnswer(const QString &answer)
{
    static const QRegularExpression contextRegex(R"(<[^<>]*>)");

    // Remove context-only sections
    QString preprocessedAnswer = answer;
    preprocessedAnswer.remove(contextRegex);

    // Normalize apostrophes, case and whitespace
    QChar typographicApostrophe(0x2019);
    QChar straightApostrophe('\'');
    preprocessedAnswer.replace(typographicApostrophe, straightApostrophe);
    preprocessedAnswer = preprocessedAnswer.toLower().simplified();

    QString result;
    result.reserve(answer.size());
    for (const QChar ch : std::as_const(preprocessedAnswer))
    {
        // Ignore punctuation and format marks
        if (ch.isLetterOrNumber() || ch.isSpace() || ch == straightApostrophe)
        {
            result.append(ch);
        }
    }

    return result;
}
