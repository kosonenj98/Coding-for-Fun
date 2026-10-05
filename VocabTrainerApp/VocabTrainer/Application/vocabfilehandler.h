#ifndef VOCABFILEHANDLER_H
#define VOCABFILEHANDLER_H

#include "../Logging/logger.h"
#include "errorcode.h"
#include "vocabfiledatatypes.h"

#include <QObject>

class VocabFileHandler : public QObject
{
    Q_OBJECT
public:
    explicit VocabFileHandler(Logger &logger, QObject *parent = nullptr);

    void read(const QString &filePath);

signals:
    void readSucceeded(const VocabFileData &data);
    void readSucceededPartially(const VocabFileData &data, int failedEntryCount);
    void readFailed(ErrorCode code);

private:
    Logger &m_logger;
};

#endif // VOCABFILEHANDLER_H
