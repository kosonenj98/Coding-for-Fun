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

using namespace VocabFile;

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

void VocabFileHandler::read(const QString &filePath, ReadMode mode)
{
    m_logger.verbose(logTag(), QStringLiteral("Handling read request..."));
    switch (mode)
    {
    case ReadMode::Info:
        handleReadInfo(filePath);
        break;
    case ReadMode::Full:
        handleReadFull(filePath);
        break;
    }
    m_logger.verbose(logTag(), QStringLiteral("Handling read request done!"));
}

void VocabFileHandler::readMultiple(const QStringList &filePaths, ReadMode mode)
{
    m_logger.verbose(logTag(), QStringLiteral("Handling read multiple request..."));

    if (filePaths.isEmpty())
    {
        m_logger.error(logTag(), QStringLiteral("No vocab files given!"));
        m_logger.verbose(logTag(), QStringLiteral("Emitting failure signal..."));
        ErrorCode code = ErrorCode::NoVocabFilesGiven;
        switch (mode)
        {
        case ReadMode::Info:
            emit readInfoMultipleFailed(code);
            break;

        case ReadMode::Full:
            emit readMultipleFailed(code);
            break;
        }
        return;
    }

    switch (mode)
    {
    case ReadMode::Info:
        handleReadInfoMultiple(filePaths);
        break;
    case ReadMode::Full:
        handleReadFullMultiple(filePaths);
        break;
    }

    m_logger.verbose(logTag(), QStringLiteral("Handling read multiple request done!"));
}

void VocabFileHandler::handleReadFull(const QString &filePath)
{
    // Read full vocab file
    m_logger.verbose(logTag(), QStringLiteral("Handling file read..."));
    ReadResult result = readFile(filePath);
    m_logger.verbose(logTag(), QStringLiteral("Handling file read done!"));

    if (result.errorCode != ErrorCode::Success)
    {
        m_logger.error(logTag(), QStringLiteral("Failed to read vocab file '%1' (%2)...").arg(filePath, errorCodeToString(result.errorCode)));
        m_logger.verbose(logTag(), QStringLiteral("Emitting failure signal..."));
        emit readFailed(result.errorCode);
        return;
    }

    if (result.failedEntryCount > 0)
    {
        m_logger.warning(logTag(), QStringLiteral("Skipped %1 faulty entries in vocab file '%2'...").arg(QString::number(result.failedEntryCount), filePath));
        m_logger.verbose(logTag(), QStringLiteral("Emitting partial success signal..."));
        emit readSucceededPartially(result.data, result.failedEntryCount);
        return;
    }

    m_logger.info(logTag(), QStringLiteral("Succesfully read vocab file '%1'!").arg(filePath));
    m_logger.verbose(logTag(), QStringLiteral("Emitting success signal..."));
    emit readSucceeded(result.data);
}

void VocabFileHandler::handleReadInfo(const QString &filePath)
{
    // Read only file info (file path and vocab title)
    m_logger.verbose(logTag(), QStringLiteral("Handling file info read..."));
    InfoResult result = readInfo(filePath);
    m_logger.verbose(logTag(), QStringLiteral("Handling file info read done!"));

    if (result.errorCode != ErrorCode::Success)
    {
        m_logger.error(logTag(), QStringLiteral("Failed to read vocab file '%1' info (%2)...").arg(filePath, errorCodeToString(result.errorCode)));
        m_logger.verbose(logTag(), QStringLiteral("Emitting failure signal..."));
        emit readInfoFailed(result.errorCode);
        return;
    }

    m_logger.info(logTag(), QStringLiteral("Succesfully read vocab file '%1' info!").arg(filePath));
    m_logger.verbose(logTag(), QStringLiteral("Emitting success signal..."));
    emit readInfoSucceeded(result.info);
    return;
}

void VocabFileHandler::handleReadFullMultiple(const QStringList &filePaths)
{
    m_logger.verbose(logTag(), QStringLiteral("Handling multiple file read..."));
    m_logger.debug(logTag(), QStringLiteral("Trying to read %1 vocab files...").arg(QString::number(filePaths.count())));
    QList<ReadResult> results;
    for (const QString &filePath : filePaths)
    {
        results.append(readFile(filePath));
    }
    m_logger.verbose(logTag(), QStringLiteral("Handling multiple file read done!"));

    QList<Data> multipleData;
    int failedVocabCount = 0;
    int partiallySucceededVocabCount = 0;
    int failedEntryCount = 0;
    int succeededVocabCount = 0;
    for (const auto &result : results)
    {
        if (result.errorCode != ErrorCode::Success)
        {
            failedVocabCount++;
            continue;
        }

        multipleData.append(result.data);

        if (result.failedEntryCount > 0)
        {
            partiallySucceededVocabCount++;
            failedEntryCount += result.failedEntryCount;
            continue;
        }

        succeededVocabCount++;
    }

    if (!(succeededVocabCount + partiallySucceededVocabCount > 0))
    {
        m_logger.error(logTag(), QStringLiteral("No valid vocabs found!"));
        m_logger.verbose(logTag(), QStringLiteral("Emitting failure signal..."));
        emit readMultipleFailed(ErrorCode::NoSucceededVocabs);
        return;
    }

    if (failedVocabCount > 0 || partiallySucceededVocabCount > 0 || failedEntryCount > 0)
    {
        m_logger.warning(logTag(), QStringLiteral("Skipped %1 faulty entries from %2 partially succeeded vocabs. Found %3 failed vocabs.")
                                       .arg(QString::number(failedEntryCount),
                                            QString::number(partiallySucceededVocabCount),
                                            QString::number(failedVocabCount)));

        m_logger.verbose(logTag(), QStringLiteral("Emitting partial success signal..."));
        emit readMultipleSucceededPartially(multipleData, failedVocabCount, partiallySucceededVocabCount, failedEntryCount);
        return;
    }

    m_logger.info(logTag(), QStringLiteral("Successfully read multiple vocab files!"));
    m_logger.verbose(logTag(), QStringLiteral("Emitting success signal..."));
    emit readMultipleSucceeded(multipleData);
}

void VocabFileHandler::handleReadInfoMultiple(const QStringList &filePaths)
{
    m_logger.verbose(logTag(), QStringLiteral("Handling multiple file info read..."));
    m_logger.debug(logTag(), QStringLiteral("Trying to read %1 vocab file infos...").arg(QString::number(filePaths.count())));
    QList<InfoResult> results;
    for (const QString &filePath : filePaths)
    {
        results.append(readInfo(filePath));
    }
    m_logger.verbose(logTag(), QStringLiteral("Handling multiple file info read done!"));

    QList<Info> multipleInfos;
    int failedVocabCount = 0;
    for (const auto &result : results)
    {
        if (result.errorCode != ErrorCode::Success)
        {
            failedVocabCount++;
            continue;
        }

        multipleInfos.append(result.info);
    }

    int succeededInfoCount = results.count() - failedVocabCount;
    if (!(succeededInfoCount > 0))
    {
        m_logger.error(logTag(), QStringLiteral("No valid vocabs found!"));
        m_logger.verbose(logTag(), QStringLiteral("Emitting failure signal..."));
        emit readInfoMultipleFailed(ErrorCode::NoSucceededVocabs);
        return;
    }

    if (failedVocabCount > 0)
    {
        m_logger.warning(logTag(), QStringLiteral("Skipped %1 faulty vocabs.").arg(QString::number(failedVocabCount)));
        m_logger.verbose(logTag(), QStringLiteral("Emitting partial success signal..."));
        emit readInfoMultipleSucceededPartially(multipleInfos, failedVocabCount);
        return;
    }

    m_logger.info(logTag(), QStringLiteral("Successfully read multiple vocab infos!"));
    m_logger.verbose(logTag(), QStringLiteral("Emitting success signal..."));
    emit readInfoMultipleSucceeded(multipleInfos);
    return;
}

InfoResult VocabFileHandler::readInfo(const QString &filePath)
{
    m_logger.verbose(logTag(), QStringLiteral("Reading vocab file '%1' info...").arg(filePath));
    InfoResult result;
    result.info.filePath = filePath;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        m_logger.error(logTag(), QStringLiteral("Failed to open vocab file '%1': '%2' (%3)").arg(filePath, file.errorString(), QString::number(file.error())));
        result.errorCode = ErrorCode::FileOpenFailed;
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

ReadResult VocabFileHandler::readFile(const QString &filePath)
{
    m_logger.verbose(logTag(), QStringLiteral("Reading vocab file '%1'...").arg(filePath));
    ReadResult result;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        m_logger.error(logTag(), QStringLiteral("Failed to open vocab file '%1': '%2' (%3)").arg(filePath, file.errorString(), QString::number(file.error())));
        result.errorCode = ErrorCode::FileOpenFailed;
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
                result.data.title = processedLine;
                state = ParseState::ReadingEntries;
                continue;
            }

            // Line is group title
            m_logger.verbose(logTag(), QStringLiteral("Set group title: '%1'").arg(processedLine));
            Group group;
            group.title = processedLine;
            result.data.groups.append(group);
            continue;
        }

        if (state == ParseState::ExpectVocabularyTitle)
        {
            // Vocab file has no title
            m_logger.warning(logTag(), QStringLiteral("Vocab file '%1' has no explicit title. Using file name instead...").arg(filePath));
            result.data.title = QFileInfo(filePath).completeBaseName();
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
            m_logger.verbose(logTag(), QStringLiteral("Adding new entry '%1' in '%2'").arg(processedLine, result.data.title));
            result.data.entries.append(vocabFileEntry);
        }
        else
        {
            // Append pair to most recent group
            Group *mostRecentGroup = &result.data.groups.last();
            m_logger.verbose(logTag(), QStringLiteral("Adding new entry '%1' in '%2':'%3'").arg(processedLine, result.data.title, mostRecentGroup->title));
            mostRecentGroup->entries.append(vocabFileEntry);
        }

        entryCount++;
    }

    m_logger.debug(logTag(), QStringLiteral("Reading vocab file '%1' succeeded!").arg(filePath));

    if (!(entryCount > 0))
    {
        // Vocab contained no entries
        m_logger.error(logTag(), QStringLiteral("No valid entries found in vocab '%1'.").arg(result.data.title));
        m_logger.verbose(logTag(), QStringLiteral("Emitting failure signal..."));
        result.errorCode = ErrorCode::VocabFileNoEntries;
        return result;
    }

    m_logger.debug(logTag(), QStringLiteral("Added %1 entries in vocab '%2'").arg(QString::number(entryCount), result.data.title));

    if (result.failedEntryCount > 0)
    {
        m_logger.warning(logTag(), QStringLiteral("Skipped %1 faulty entries in vocab '%2'.").arg(QString::number(result.failedEntryCount), result.data.title));
        m_logger.verbose(logTag(), QStringLiteral("Emitting partial success signal..."));
        return result;
    }

    m_logger.verbose(logTag(), QStringLiteral("Reading vocab file '%1' done!").arg(filePath));
    m_logger.verbose(logTag(), QStringLiteral("Emitting success signal..."));
    return result;
}
