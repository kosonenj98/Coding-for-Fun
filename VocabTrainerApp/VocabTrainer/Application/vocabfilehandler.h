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

public slots:
    void read(const QString &filePath, VocabFile::ReadMode mode);
    void readMultiple(const QStringList &filePaths, VocabFile::ReadMode mode);

signals:
    void readInfoSucceeded(const VocabFile::Info &info);
    void readInfoFailed(ErrorCode code);

    void readInfoMultipleSucceeded(const QList<VocabFile::Info> &multipleInfo);
    void readInfoMultipleSucceededPartially(const QList<VocabFile::Info> &multipleInfo, int failedVocabCount);
    void readInfoMultipleFailed(ErrorCode code);

    void readSucceeded(const VocabFile::Data &data);
    void readSucceededPartially(const VocabFile::Data &data, int failedEntryCount);
    void readFailed(ErrorCode code);

    void readMultipleSucceeded(const QList<VocabFile::Data> &multipleData);
    void readMultipleSucceededPartially(const QList<VocabFile::Data> &multipleData, int failedVocabCount, int partiallySucceededVocabCount, int failedEntryCount);
    void readMultipleFailed(ErrorCode code);

private:
    void handleReadFull(const QString &filePath);
    void handleReadInfo(const QString &filePath);
    void handleReadFullMultiple(const QStringList &filePaths);
    void handleReadInfoMultiple(const QStringList &filePaths);

    VocabFile::InfoResult readInfo(const QString &filePath);
    VocabFile::ReadResult readFile(const QString &filePath);

    Logger &m_logger;
};

#endif // VOCABFILEHANDLER_H
