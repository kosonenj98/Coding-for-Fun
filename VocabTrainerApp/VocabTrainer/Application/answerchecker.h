#ifndef ANSWERCHECKER_H
#define ANSWERCHECKER_H

#include "Logging/logger.h"

class AnswerChecker
{
public:
    AnswerChecker(Logger &logger);

    bool checkAnswer(const QString &answer, const QSet<QString> correctAnswers);

private:
    QString normalizeAnswer(const QString &answer);

    Logger &m_logger;
};

#endif // ANSWERCHECKER_H
