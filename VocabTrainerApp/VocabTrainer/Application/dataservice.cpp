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
    : QObject{parent}, m_logger(logger), m_vocabGenerator(logger)
{
    m_logger.verbose(logTag(), QStringLiteral("Initializing data service..."));
    m_logger.verbose(logTag(), QStringLiteral("Initializing data service done!"));
}

void DataService::loadVocab(const Vocab::LoadRequest &request)
{
    m_logger.debug(logTag(), QStringLiteral("Handling loading vocabs..."));
    VocabFile::ReadRequest readRequest;
    readRequest.filePaths = request.filePaths;
    readRequest.direction = request.direction;
    emit requestVocabFileRead(readRequest);
    m_logger.debug(logTag(), QStringLiteral("Handling loading vocabs done!"));
}

void DataService::clearVocab(const Vocab::ClearRequest &request)
{
    Q_UNUSED(request);
    m_logger.debug(logTag(), QStringLiteral("Clearing vocab..."));

    Vocab::ClearResult result;
    m_vocabDatas.clear();

    m_logger.debug(logTag(), QStringLiteral("Clearing vocab done!"));

    emit clearVocabFinished(result);
}

void DataService::handleVocabFileReadFinished(const VocabFile::ReadResult &result)
{
    Vocab::LoadResult loadVocabsResult;
    if (result.code != Error::Code::Success)
    {
        loadVocabsResult.code = result.code;
        emit loadVocabFinished(loadVocabsResult);
        return;
    }

    if (result.failedVocabCount > 0 || result.partiallySucceededVocabCount > 0 || result.failedEntryCount > 0)
    {
        m_logger.warning(logTag(), QStringLiteral("Skipped %1 faulty vocabs and %2 faulty entries in %3 vocabs.").arg(QString::number(result.failedVocabCount), QString::number(result.failedEntryCount), QString::number(result.partiallySucceededVocabCount)));
    }
    loadVocabsResult.failedVocabCount = result.failedVocabCount;
    loadVocabsResult.partiallySucceededVocabCount = result.partiallySucceededVocabCount;
    loadVocabsResult.failedEntryCount = result.failedEntryCount;

    QMap<QString, Vocab::Data> vocabDatas;
    for (const auto &data : result.datas)
    {
        VocabGeneration::GenerationResult generateVocabDataResult = generateVocabData(data);

        if (generateVocabDataResult.code != Error::Code::Success)
        {
            m_logger.error(logTag(), QStringLiteral("Failed to create data for vocab '%1' (%2)").arg(data.title, Error::codeToQString(generateVocabDataResult.code)));
            loadVocabsResult.failedVocabCount++;
            continue;
        }

        if (generateVocabDataResult.failedEntryCount > 0)
        {
            m_logger.warning(logTag(), QStringLiteral("Skipped %1 faulty entries in vocab '%2' during data generation.").arg(QString::number(generateVocabDataResult.failedEntryCount), data.title));
            loadVocabsResult.partiallySucceededVocabCount++;
        }
        loadVocabsResult.failedEntryCount += generateVocabDataResult.failedEntryCount;

        vocabDatas.insert(data.title, generateVocabDataResult.data);
    }

    int successCount = result.datas.count() - loadVocabsResult.failedVocabCount;
    if (!(successCount > 0))
    {
        m_logger.error(logTag(), QStringLiteral("Failed to load any vocabs!"));
        loadVocabsResult.code = Error::Code::NoSucceededVocabs;
        emit loadVocabFinished(loadVocabsResult);
        return;
    }

    if (result.failedVocabCount > 0 || result.partiallySucceededVocabCount > 0 || result.failedEntryCount > 0)
    {
        m_logger.warning(logTag(), QStringLiteral("Skipped %1 faulty vocabs and %2 faulty entries in %3 vocabs.").arg(QString::number(result.failedVocabCount), QString::number(result.failedEntryCount), QString::number(result.partiallySucceededVocabCount)));
    }

    m_vocabDatas.clear();
    m_vocabDatas = vocabDatas;

    // Formulate content info for each vocab
    for (const auto &data : std::as_const(m_vocabDatas))
    {
        Vocab::ContentInfo info;
        info.title = data.title;
        info.entryCount = data.entries.count();
        info.totalEntryCount = info.entryCount;

        for (auto it = data.groupEntries.cbegin(); it != data.groupEntries.cend(); it++)
        {
            Vocab::GroupContentInfo groupInfo;
            groupInfo.title = it.key();
            groupInfo.entryCount = it.value().count();
            info.groupInfos.append(groupInfo);
            info.totalEntryCount += groupInfo.entryCount;
        }

        loadVocabsResult.contentInfos.append(info);
    }

    emit loadVocabFinished(loadVocabsResult);
}

void DataService::handleGetDataRequest(const Vocab::GetDataRequest &request)
{
    Vocab::GetDataResult result;

    for (const auto &filter : request.filters)
    {
        if (!m_vocabDatas.contains(filter.title))
        {
            continue;
        }

        Vocab::Data &referenceData = m_vocabDatas[filter.title];

        Vocab::Data data;
        data.title = referenceData.title;

        if (filter.includeEntriesWithoutGroup)
        {
            result.totalEntryCount += referenceData.entries.count();
            data.entries = referenceData.entries;
        }

        for (const auto &groupTitle : filter.groupTitles)
        {
            if (!referenceData.groupEntries.contains(groupTitle))
            {
                continue;
            }

            result.totalEntryCount += referenceData.groupEntries[groupTitle].count();
            data.groupEntries[groupTitle] = referenceData.groupEntries[groupTitle];
        }

        result.datas.append(data);
    }

    emit getDataFinished(result);
}

const VocabGeneration::GenerationResult DataService::generateVocabData(const VocabFile::Data &data)
{
    m_logger.debug(logTag(), QStringLiteral("Handling vocab data generation..."));
    VocabGeneration::GenerationResult result = m_vocabGenerator.generate(data);
    m_logger.debug(logTag(), QStringLiteral("Handling vocab data generation done!"));
    return result;
}
