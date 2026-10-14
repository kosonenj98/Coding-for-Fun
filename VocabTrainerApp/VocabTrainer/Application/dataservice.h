#ifndef DATASERVICE_H
#define DATASERVICE_H

#include "Logging/logger.h"
#include "DataTypes/vocabfile.h"
#include "DataTypes/vocab.h"
#include "vocabgenerator.h"

#include <QObject>

class DataService : public QObject
{
    Q_OBJECT
public:
    explicit DataService(Logger &logger, QObject *parent = nullptr);

    void loadVocab(const Vocab::LoadRequest &request);
    void clearVocab(const Vocab::ClearRequest &request);

public slots:
    void handleVocabFileReadFinished(const VocabFile::ReadResult &result);

signals:
    void requestVocabFileRead(const VocabFile::ReadRequest &request);
    void loadVocabFinished(const Vocab::LoadResult &result);
    void clearVocabFinished(const Vocab::ClearResult &result);

private:
    const VocabGeneration::GenerationResult generateVocabData(const VocabFile::Data &data);

    Logger &m_logger;
    VocabGenerator m_vocabGenerator;
    QMap<QString,Vocab::Data> m_vocabDatas;
};

#endif // DATASERVICE_H
