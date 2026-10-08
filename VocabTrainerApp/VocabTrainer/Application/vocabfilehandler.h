#ifndef VOCABFILEHANDLER_H
#define VOCABFILEHANDLER_H

#include "Logging/logger.h"
#include "DataTypes/requests.h"
#include "DataTypes/results.h"

#include <QObject>

class VocabFileHandler : public QObject
{
    Q_OBJECT
public:
    explicit VocabFileHandler(Logger &logger, QObject *parent = nullptr);

public slots:
    void read(const ReadVocabFileRequest &request);
    void readMultiple(const ReadVocabFilesRequest &request);

signals:
    void readInfoFinished(const ReadVocabFileInfoResult &result);
    void readInfosFinished(const ReadVocabFileInfosResult &result);

    void readDataFinished(const ReadVocabFileDataResult &result);
    void readDatasFinished(const ReadVocabFileDatasResult &result);

private:
    void handleReadData(const QString &filePath);
    void handleReadInfo(const QString &filePath);
    void handleReadDatas(const QStringList &filePaths);
    void handleReadInfos(const QStringList &filePaths);

    const ReadVocabFileInfoResult readInfo(const QString &filePath);
    const ReadVocabFileDataResult readData(const QString &filePath);

    Logger &m_logger;
};

#endif // VOCABFILEHANDLER_H
