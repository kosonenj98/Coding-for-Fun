#ifndef VOCABFILEHANDLER_H
#define VOCABFILEHANDLER_H

#include "Logging/logger.h"
#include "DataTypes/vocabfile.h"

#include <QObject>

class VocabFileHandler : public QObject
{
    Q_OBJECT
public:
    explicit VocabFileHandler(Logger &logger, QObject *parent = nullptr);

public slots:
    void handleReadRequest(const VocabFile::ReadRequest &request);

signals: 
    void readFinished(const VocabFile::ReadResult &result);

private:
    const VocabFile::ReadFileResult readFile(const QString &filePath, const Vocab::Direction direction);

    Logger &m_logger;
};

#endif // VOCABFILEHANDLER_H
