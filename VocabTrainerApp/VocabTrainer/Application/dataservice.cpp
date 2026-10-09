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

void DataService::loadVocab(const LoadVocabRequest &request)
{
    ReadVocabFileRequest readRequest;
    readRequest.filePath = request.info.filePath;
    readRequest.mode = request.mode;
    emit readVocabFile(readRequest);
}

void DataService::loadVocabs(const LoadVocabsRequest &request)
{
    ReadVocabFilesRequest readMultipleRequest;
    readMultipleRequest.mode = request.mode;
    for (const auto &info : request.infos)
    {
        readMultipleRequest.filePaths.append(info.filePath);
    }
    emit readVocabFiles(readMultipleRequest);
}


void DataService::handleVocabFileReadInfoFinished(const ReadVocabFileInfoResult &result)
{
    LoadVocabInfoResult loadVocabInfoResult;
    if (result.code != ErrorCode::Success)
    {
        m_logger.error(logTag(), QStringLiteral("Failed to load vocab file '%1' (%2)").arg(result.info.filePath, errorCodeToString(result.code)));
        loadVocabInfoResult.code = result.code;
        emit loadVocabInfoFinished(loadVocabInfoResult);
        return;
    }

    loadVocabInfoResult.info = result.info;
    emit loadVocabInfoFinished(loadVocabInfoResult);
}

void DataService::handleVocabFileReadInfosFinished(const ReadVocabFileInfosResult &result)
{
    LoadVocabInfosResult loadVocabInfosResult;
    if (result.code != ErrorCode::Success)
    {
        m_logger.error(logTag(), QStringLiteral("Failed to load vocab files (%1)").arg(errorCodeToString(result.code)));
        loadVocabInfosResult.code = result.code;
        emit loadVocabInfosFinished(loadVocabInfosResult);
        return;
    }

    for (const auto &info : result.infos)
    {
        loadVocabInfosResult.infos.append(info);
    }

    emit loadVocabInfosFinished(loadVocabInfosResult);
}

void DataService::handleVocabFileReadDataFinished(const ReadVocabFileDataResult &result)
{
    LoadVocabDataResult loadVocabDataResult;
    if (result.code != ErrorCode::Success)
    {
        m_logger.error(logTag(), QStringLiteral("Failed to load vocab file '%1' (%2)").arg(result.data.info.filePath, errorCodeToString(result.code)));
        loadVocabDataResult.code = result.code;
        emit loadVocabDataFinished(loadVocabDataResult);
        return;
    }

    if (result.failedEntryCount > 0)
    {
        m_logger.warning(logTag(), QStringLiteral("Skipped %1 faulty entries in vocab '%2' (%3) during reading").arg(QString::number(result.failedEntryCount), result.data.info.vocabTitle, result.data.info.filePath));
    }
    loadVocabDataResult.failedEntryCount = result.failedEntryCount;

    CreateVocabDataResult createVocabDataResult = createVocabData(result.data);

    if (createVocabDataResult.code != ErrorCode::Success)
    {
        m_logger.error(logTag(), QStringLiteral("Failed to create data for vocab '%1' (%2)").arg(result.data.info.vocabTitle, errorCodeToString(createVocabDataResult.code)));
        loadVocabDataResult.code = createVocabDataResult.code;
        emit loadVocabDataFinished(loadVocabDataResult);
        return;
    }

    if (createVocabDataResult.failedEntryCount > 0)
    {
        m_logger.warning(logTag(), QStringLiteral("Skipped %1 faulty entries in vocab '%2' (%3) during data generation.").arg(QString::number(result.failedEntryCount), result.data.info.vocabTitle, result.data.info.filePath));
    }
    loadVocabDataResult.failedEntryCount += createVocabDataResult.failedEntryCount;

    // Update vocab data
    m_vocabDatas.clear();
    m_vocabDatas.insert(result.data.info.vocabTitle, createVocabDataResult.data);

    emit loadVocabDataFinished(loadVocabDataResult);
}

void DataService::handleVocabFileReadDatasFinished(const ReadVocabFileDatasResult &result)
{
    LoadVocabDatasResult loadVocabDatasResult;
    if (result.code != ErrorCode::Success)
    {
        loadVocabDatasResult.code = result.code;
        emit loadVocabDatasFinished(loadVocabDatasResult);
    }

    if (result.failedVocabCount > 0 || result.partiallySucceededVocabCount > 0 || result.failedEntryCount > 0)
    {
        m_logger.warning(logTag(), QStringLiteral("Skipped %1 faulty vocabs and %2 faulty entries in %3 vocabs.").arg(QString::number(result.failedVocabCount), QString::number(result.failedEntryCount), QString::number(result.partiallySucceededVocabCount)));
    }
    loadVocabDatasResult.failedVocabCount = result.failedVocabCount;
    loadVocabDatasResult.partiallySucceededVocabCount = result.partiallySucceededVocabCount;
    loadVocabDatasResult.failedEntryCount = result.failedEntryCount;

    QMap<QString,Vocab::Data> vocabDatas;
    for (const auto &data : result.datas)
    {
        CreateVocabDataResult createVocabDataResult = createVocabData(data);

        if (createVocabDataResult.code != ErrorCode::Success)
        {
            m_logger.error(logTag(), QStringLiteral("Failed to create data for vocab '%1' (%2)").arg(data.info.vocabTitle, errorCodeToString(createVocabDataResult.code)));
            loadVocabDatasResult.failedVocabCount++;
            continue;
        }

        if (createVocabDataResult.failedEntryCount > 0)
        {
            m_logger.warning(logTag(), QStringLiteral("Skipped %1 faulty entries in vocab '%2' (%3) during data generation.").arg(QString::number(createVocabDataResult.failedEntryCount), data.info.vocabTitle, data.info.filePath));
            loadVocabDatasResult.partiallySucceededVocabCount++;
        }
        loadVocabDatasResult.failedEntryCount += createVocabDataResult.failedEntryCount;

        vocabDatas.insert(data.info.vocabTitle, createVocabDataResult.data);
    }

    int successCount = result.datas.count() - loadVocabDatasResult.failedVocabCount;
    if (!(successCount > 0))
    {
        m_logger.error(logTag(), QStringLiteral("Failed to load all vocabs!"));
        loadVocabDatasResult.code = ErrorCode::NoSucceededVocabs;
        emit loadVocabDatasFinished(loadVocabDatasResult);
        return;
    }

    m_vocabDatas.clear();
    m_vocabDatas = vocabDatas;

    emit loadVocabDatasFinished(loadVocabDatasResult);
}

const CreateVocabDataResult DataService::createVocabData(const VocabFile::Data &data)
{
    CreateVocabDataResult result;
    return result;
}
