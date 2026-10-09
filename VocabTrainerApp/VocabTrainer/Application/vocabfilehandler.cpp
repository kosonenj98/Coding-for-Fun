#include "vocabfilehandler.h"

#include <QFile>
#include <QFileInfo>

namespace
{
    const QString &logTag()
    {
        static const QString tag = QStringLiteral("VocabFileHandler");
        return tag;
    }
}

enum class ParseState
{
    ExpectVocabularyTitle,
    ReadingEntries
};

VocabFileHandler::VocabFileHandler(Logger &logger, QObject *parent)
    : QObject{parent}, m_logger(logger)
{
    m_logger.verbose(logTag(), QStringLiteral("Constructing VocabFileHandler..."));
    m_logger.verbose(logTag(), QStringLiteral("Constructing VocabFileHandler done!"));
}

void VocabFileHandler::read(const ReadVocabFileRequest &request)
{
    m_logger.verbose(logTag(), QStringLiteral("Handling read request..."));
    switch (request.mode)
    {
    case VocabFile::ReadMode::Info:
        handleReadInfo(request.filePath);
        break;
    case VocabFile::ReadMode::Data:
        handleReadData(request.filePath);
        break;
    }
    m_logger.verbose(logTag(), QStringLiteral("Handling read request done!"));
}

void VocabFileHandler::readMultiple(const ReadVocabFilesRequest &request)
{
    m_logger.verbose(logTag(), QStringLiteral("Handling read multiple request..."));

    if (request.filePaths.isEmpty())
    {
        m_logger.error(logTag(), QStringLiteral("No vocab files given!"));
        m_logger.verbose(logTag(), QStringLiteral("Emitting failure signal..."));
        ErrorCode code = ErrorCode::NoVocabFilesGiven;
        switch (request.mode)
        {
            case VocabFile::ReadMode::Info:
            {
                ReadVocabFileInfosResult result;
                result.code = code;
                emit readInfosFinished(result);
                break;
            }

            case VocabFile::ReadMode::Data:
            {
                ReadVocabFileDatasResult result;
                result.code = code;
                emit readDatasFinished(result);
                break;
            }
        }
        return;
    }

    switch (request.mode)
    {
    case VocabFile::ReadMode::Info:
        handleReadInfos(request.filePaths);
        break;
    case VocabFile::ReadMode::Data:
        handleReadDatas(request.filePaths);
        break;
    }

    m_logger.verbose(logTag(), QStringLiteral("Handling read multiple request done!"));
}

void VocabFileHandler::handleReadData(const QString &filePath)
{
    // Read full vocab file
    m_logger.verbose(logTag(), QStringLiteral("Handling file read..."));
    const ReadVocabFileDataResult result = readData(filePath);
    m_logger.verbose(logTag(), QStringLiteral("Handling file read done!"));

    emit readDataFinished(result);
}

void VocabFileHandler::handleReadInfo(const QString &filePath)
{
    // Read only file info (file path and vocab title)
    m_logger.verbose(logTag(), QStringLiteral("Handling file info read..."));
    const ReadVocabFileInfoResult result = readInfo(filePath);
    m_logger.verbose(logTag(), QStringLiteral("Handling file info read done!"));

    emit readInfoFinished(result);
}

void VocabFileHandler::handleReadDatas(const QStringList &filePaths)
{
    m_logger.verbose(logTag(), QStringLiteral("Handling multiple file read..."));
    m_logger.debug(logTag(), QStringLiteral("Trying to read %1 vocab files...").arg(QString::number(filePaths.count())));
    QList<ReadVocabFileDataResult> results;
    for (const QString &filePath : filePaths)
    {
        results.append(readData(filePath));
    }
    m_logger.verbose(logTag(), QStringLiteral("Handling multiple file read done!"));

    ReadVocabFileDatasResult readDatasResult;
    int succeededVocabCount = 0;
    for (const auto &result : results)
    {
        if (result.code != ErrorCode::Success)
        {
            readDatasResult.failedVocabCount++;
            continue;
        }

        readDatasResult.datas.append(result.data);

        if (result.failedEntryCount > 0)
        {
            readDatasResult.partiallySucceededVocabCount++;
            readDatasResult.failedEntryCount += result.failedEntryCount;
            continue;
        }

        succeededVocabCount++;
    }

    if (!(succeededVocabCount + readDatasResult.partiallySucceededVocabCount > 0))
    {
        readDatasResult.code = ErrorCode::NoSucceededVocabs;
    }

    emit readDatasFinished(readDatasResult);
}

void VocabFileHandler::handleReadInfos(const QStringList &filePaths)
{
    m_logger.verbose(logTag(), QStringLiteral("Handling multiple file info read..."));
    m_logger.debug(logTag(), QStringLiteral("Trying to read %1 vocab file infos...").arg(QString::number(filePaths.count())));
    QList<ReadVocabFileInfoResult> results;
    for (const QString &filePath : filePaths)
    {
        results.append(readInfo(filePath));
    }
    m_logger.verbose(logTag(), QStringLiteral("Handling multiple file info read done!"));

    ReadVocabFileInfosResult readInfosResult;
    for (const auto &result : results)
    {
        if (result.code != ErrorCode::Success)
        {
            readInfosResult.failedVocabCount++;
            continue;
        }

        readInfosResult.infos.append(result.info);
    }

    int succeededInfoCount = results.count() - readInfosResult.failedVocabCount;
    if (!(succeededInfoCount > 0))
    {
        readInfosResult.code = ErrorCode::NoSucceededVocabs;
    }

    emit readInfosFinished(readInfosResult);
}

const ReadVocabFileInfoResult VocabFileHandler::readInfo(const QString &filePath)
{
    m_logger.verbose(logTag(), QStringLiteral("Reading vocab file '%1' info...").arg(filePath));
    ReadVocabFileInfoResult result;
    result.info.filePath = filePath;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        m_logger.error(logTag(), QStringLiteral("Failed to open vocab file '%1': '%2' (%3)").arg(filePath, file.errorString(), QString::number(file.error())));
        result.code = ErrorCode::FileOpenFailed;
        return result;
    }

    // Try to find vocab file title
    const QStringList lines = QString::fromUtf8(file.readAll()).split('\n');
    ParseState state = ParseState::ExpectVocabularyTitle;
    for (const QString &line : lines)
    {
        QString processedLine = line.trimmed();
        const int commentIndex = processedLine.indexOf('#'); // Comments start with "#"
        if (commentIndex >= 0)
        {
            // Ignore comment
            processedLine = processedLine.left(commentIndex);
        }

        if (processedLine.isEmpty())
        {
            continue;
        }

        if (!processedLine.contains("="))
        {
            // Line is either vocab or group title
            if (state == ParseState::ExpectVocabularyTitle)
            {
                m_logger.verbose(logTag(), QStringLiteral("Found vocabulary title: '%1'").arg(processedLine));
                result.info.vocabTitle = processedLine;
                break;
            }
        }

        // Vocab file has no title
        m_logger.warning(logTag(), QStringLiteral("Vocab file '%1' has no explicit title. Using file name instead...").arg(filePath));
        result.info.vocabTitle = QFileInfo(filePath).completeBaseName();
        break;
    }

    return result;
}

const ReadVocabFileDataResult VocabFileHandler::readData(const QString &filePath)
{
    m_logger.verbose(logTag(), QStringLiteral("Reading vocab file '%1'...").arg(filePath));
    ReadVocabFileDataResult result;
    result.data.info.filePath = filePath;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        m_logger.error(logTag(), QStringLiteral("Failed to open vocab file '%1': '%2' (%3)").arg(filePath, file.errorString(), QString::number(file.error())));
        result.code = ErrorCode::FileOpenFailed;
        return result;
    }

    // Parse vocab file data
    const QStringList lines = QString::fromUtf8(file.readAll()).split('\n');
    ParseState state = ParseState::ExpectVocabularyTitle;
    int entryCount = 0;
    for (const QString &line : lines)
    {
        QString processedLine = line.trimmed();
        const int commentIndex = processedLine.indexOf('#'); // Comments start with "#"
        if (commentIndex >= 0)
        {
            // Ignore comment
            processedLine = processedLine.left(commentIndex);
        }

        if (processedLine.isEmpty())
        {
            continue;
        }

        if (!processedLine.contains("="))
        {
            // Line is either vocab or group title
            if (state == ParseState::ExpectVocabularyTitle)
            {
                m_logger.verbose(logTag(), QStringLiteral("Set vocabulary title: '%1'").arg(processedLine));
                result.data.info.vocabTitle = processedLine;
                state = ParseState::ReadingEntries;
                continue;
            }

            // Line is group title
            m_logger.verbose(logTag(), QStringLiteral("Set group title: '%1'").arg(processedLine));
            VocabFile::Group group;
            group.title = processedLine;
            result.data.groups.append(group);
            continue;
        }

        if (state == ParseState::ExpectVocabularyTitle)
        {
            // Vocab file has no title
            m_logger.warning(logTag(), QStringLiteral("Vocab file '%1' has no explicit title. Using file name instead...").arg(filePath));
            result.data.info.vocabTitle = QFileInfo(filePath).completeBaseName();
            state = ParseState::ReadingEntries;
        }

        const QStringList parts = processedLine.split("=");
        if (parts.size() != 2)
        {
            // Invalid syntax
            m_logger.warning(logTag(), QStringLiteral("Cannot parse entry: '%1'. Skipping...").arg(processedLine));
            result.failedEntryCount++;
            continue;
        }

        const QString &key = parts.first().trimmed();
        const QString &value = parts.last().trimmed();
        if (key.isEmpty() || value.isEmpty())
        {
            // Invalid syntax
            m_logger.warning(logTag(), QStringLiteral("Empty values in entry: '%1'. Skipping...").arg(processedLine));
            result.failedEntryCount++;
            continue;
        }

        const VocabFile::Entry vocabFileEntry = {key, value};
        if (result.data.groups.isEmpty())
        {
            // Vocab has not yet any groups. Append pair to entries
            m_logger.verbose(logTag(), QStringLiteral("Adding new entry '%1' in '%2'").arg(processedLine, result.data.info.vocabTitle));
            result.data.entries.append(vocabFileEntry);
        }
        else
        {
            // Append pair to most recent group
            VocabFile::Group *mostRecentGroup = &result.data.groups.last();
            m_logger.verbose(logTag(), QStringLiteral("Adding new entry '%1' in '%2':'%3'").arg(processedLine, result.data.info.vocabTitle, mostRecentGroup->title));
            mostRecentGroup->entries.append(vocabFileEntry);
        }

        entryCount++;
    }

    m_logger.debug(logTag(), QStringLiteral("Reading vocab file '%1' succeeded!").arg(filePath));

    if (!(entryCount > 0))
    {
        // Vocab contained no entries
        m_logger.error(logTag(), QStringLiteral("No valid entries found in vocab '%1'.").arg(result.data.info.vocabTitle));
        m_logger.verbose(logTag(), QStringLiteral("Emitting failure signal..."));
        result.code = ErrorCode::VocabFileNoEntries;
        return result;
    }

    m_logger.debug(logTag(), QStringLiteral("Added %1 entries in vocab '%2'").arg(QString::number(entryCount), result.data.info.vocabTitle));

    if (result.failedEntryCount > 0)
    {
        m_logger.warning(logTag(), QStringLiteral("Skipped %1 faulty entries in vocab '%2'.").arg(QString::number(result.failedEntryCount), result.data.info.vocabTitle));
        m_logger.verbose(logTag(), QStringLiteral("Emitting partial success signal..."));
        return result;
    }

    m_logger.verbose(logTag(), QStringLiteral("Reading vocab file '%1' done!").arg(filePath));
    m_logger.verbose(logTag(), QStringLiteral("Emitting success signal..."));
    return result;
}
