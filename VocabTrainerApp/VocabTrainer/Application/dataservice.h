#ifndef DATASERVICE_H
#define DATASERVICE_H

#include "../Logging/logger.h"
#include "vocabdatatypes.h"
#include "errorcode.h"
#include "vocabfiledatatypes.h"

#include <QObject>

class DataService : public QObject
{
    Q_OBJECT
public:
    explicit DataService(Logger &logger, QObject *parent = nullptr);

    void loadVocabData(const QString &filePath);
    void loadVocabDataMultiple(const QStringList &filePaths);

    void getVocabData(const QString &title);

public slots:
    void vocabFileReadSucceeded(const VocabFile::Data &data);
    void vocabFileReadSucceededPartially(const VocabFile::Data &data, int failedEntryCount);
    void vocabFileReadFailed(ErrorCode code);

    void vocabFileReadMultipleSucceeded(const QList<VocabFile::Data> &multipleData);
    void vocabFileReadMultipleSucceededPartially(const QList<VocabFile::Data> &multipleData, int failedVocabCount, int partiallySucceededVocabCount, int failedEntryCount);
    void vocabFileReadMultipleFailed(ErrorCode code);

signals:
    void requestVocabFileRead(const QString &filePath, VocabFile::ReadMode mode);
    void requestVocabFileReadMultiple(const QStringList &filePaths, VocabFile::ReadMode mode);

    void loadVocabDataSucceeded();
    void loadVocabDataSucceededPartially(int failedEntryCount);
    void loadVocabDataFailed(ErrorCode code);

    void loadVocabDataMultipleSucceeded();
    void loadVocabDataMultipleSucceededPartially(int failedVocabCount, int partiallySucceededVocabCount, int failedEntryCount);
    void loadVocabDataMultipleFailed(ErrorCode code);

private:
    Logger &m_logger;
    QList<VocabFile::Data> m_VocabFileData;
    Vocab::Data m_VocabData;
};

#endif // DATASERVICE_H
