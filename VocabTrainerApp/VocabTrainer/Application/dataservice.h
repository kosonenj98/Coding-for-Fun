#ifndef DATASERVICE_H
#define DATASERVICE_H

#include "Logging/logger.h"
#include "DataTypes/requests.h"
#include "DataTypes/results.h"
#include "vocabgenerator.h"

#include <QObject>

class DataService : public QObject
{
    Q_OBJECT
public:
    explicit DataService(Logger &logger, QObject *parent = nullptr);

    void loadVocab(const LoadVocabRequest &request);
    void loadVocabs(const LoadVocabsRequest &request);

public slots:
    void handleVocabFileReadInfoFinished(const ReadVocabFileInfoResult &result);
    void handleVocabFileReadInfosFinished(const ReadVocabFileInfosResult &result);

    void handleVocabFileReadDataFinished(const ReadVocabFileDataResult &result);
    void handleVocabFileReadDatasFinished(const ReadVocabFileDatasResult &result);

signals:
    void readVocabFile(const ReadVocabFileRequest &request);
    void readVocabFiles(const ReadVocabFilesRequest &request);

    void loadVocabInfoFinished(const LoadVocabInfoResult &result);
    void loadVocabInfosFinished(const LoadVocabInfosResult &result);

    void loadVocabDataFinished(const LoadVocabDataResult &result);
    void loadVocabDatasFinished(const LoadVocabDatasResult &result);

private:
    const GenerateVocabDataResult createVocabData(const VocabFile::Data &data);

    Logger &m_logger;
    VocabGenerator m_vocabGenerator;
    QMap<QString,Vocab::Data> m_vocabDatas;
};

#endif // DATASERVICE_H
