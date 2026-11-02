#ifndef STATISTICS_H
#define STATISTICS_H

#include "error.h"
#include "training.h"

#include <QDateTime>

namespace Statistics
{
    struct AnalyzeTrainingRequest
    {
        QDateTime startedAt;
        qint64 durationMs;
        int longestCorrectStreak;
        QList<Training::QuestionData> data;
        bool save;
    };

    struct AnalyzeTrainingResult
    {
        Error::Code code = Error::Code::Success;
        Training::SessionData trainingSessionData;
    };

    struct InitializeDatabaseRequest
    {
        QString path;
    };

    struct InitializeDatabaseResult
    {
        Error::Code code = Error::Code::Success;
        QString path;
    };

    struct SaveTrainingRequest
    {
        QList<Training::QuestionData> questionDatas;
        Training::SessionData sessionData;
    };

    struct SaveTrainingResult
    {
        Error::Code code = Error::Code::Success;
    };
}

#endif // STATISTICS_H
