#include "dataservice.h"

namespace
{
    const QString &logTag()
    {
        static const QString tag = QStringLiteral("DataService");
        return tag;
    }
}

DataService::DataService(Logger &logger, QObject *parent)
    : QObject{parent}, m_logger(logger)
{
    m_logger.verbose(logTag(), QStringLiteral("Initializing data service..."));
    m_logger.verbose(logTag(), QStringLiteral("Initializing data service done!"));
}

void DataService::loadVocabData(const QString &filePath)
{
    m_logger.verbose(logTag(), QStringLiteral("Requesting vocab file read..."));
    emit requestVocabFileRead(filePath, VocabFile::ReadMode::Full);
}

void DataService::loadVocabDataMultiple(const QStringList &filePaths)
{
    m_logger.verbose(logTag(), QStringLiteral("Requesting multiple vocab file read..."));
    emit requestVocabFileReadMultiple(filePaths, VocabFile::ReadMode::Full);
}

void DataService::vocabFileReadSucceeded(const VocabFile::Data &data)
{
    m_logger.debug(logTag(), QStringLiteral("Vocab file read succeeded!"));
    m_VocabFileData = {data};
    m_logger.verbose(logTag(), QStringLiteral("Emitting success signal..."));
    emit loadVocabDataSucceeded();
}

void DataService::vocabFileReadSucceededPartially(const VocabFile::Data &data, int failedEntryCount)
{
    m_logger.debug(logTag(), QStringLiteral("Vocab file read succeeded partially. Skipped %1 faulty entries.").arg(QString::number(failedEntryCount)));
    m_VocabFileData = {data};
    m_logger.verbose(logTag(), QStringLiteral("Emitting partial success signal..."));
    emit loadVocabDataSucceededPartially(failedEntryCount);
}

void DataService::vocabFileReadFailed(ErrorCode code)
{
    m_logger.error(logTag(), QStringLiteral("Vocab file read failed! ErrorCode: %1").arg(errorCodeToString(code)));
    m_logger.verbose(logTag(), QStringLiteral("Emitting failure signal..."));
    emit loadVocabDataFailed(code);
}

void DataService::vocabFileReadMultipleSucceeded(const QList<VocabFile::Data> &multipleData)
{
    m_logger.debug(logTag(), QStringLiteral("Multiple vocab file read succeeded!"));
    m_VocabFileData.clear();
    for (const auto &data : multipleData)
    {
        m_VocabFileData.append(data);
    }
    m_logger.verbose(logTag(), QStringLiteral("Emitting success signal..."));
    emit loadVocabDataMultipleSucceeded();
}

void DataService::vocabFileReadMultipleSucceededPartially(const QList<VocabFile::Data> &multipleData, int failedVocabCount, int partiallySucceededVocabCount, int failedEntryCount)
{
    m_logger.debug(logTag(), QStringLiteral("Multiple vocab file read succeeded partially. Skipped %1 faulty vocabs and %2 faulty entries in %3 vocabs.")
                                 .arg(QString::number(failedVocabCount), QString::number(failedEntryCount), QString::number(partiallySucceededVocabCount)));
    m_VocabFileData.clear();
    for (const auto &data : multipleData)
    {
        m_VocabFileData.append(data);
    }
    m_logger.verbose(logTag(), QStringLiteral("Emitting partial success signal..."));
    emit loadVocabDataMultipleSucceededPartially(failedVocabCount, partiallySucceededVocabCount, failedEntryCount);
}

void DataService::vocabFileReadMultipleFailed(ErrorCode code)
{
    m_logger.error(logTag(), QStringLiteral("Multiple vocab file read failed! ErrorCode: %1").arg(errorCodeToString(code)));
    m_logger.verbose(logTag(), QStringLiteral("Emitting failure signal..."));
    emit loadVocabDataMultipleFailed(code);
}
