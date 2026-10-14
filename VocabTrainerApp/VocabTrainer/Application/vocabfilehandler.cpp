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

void VocabFileHandler::handleReadRequest(const VocabFile::ReadRequest &request)
{
    m_logger.verbose(logTag(), QStringLiteral("Handling read request..."));
    VocabFile::ReadResult result;

    if (request.filePaths.isEmpty())
    {
        m_logger.error(logTag(), QStringLiteral("No filepaths given!"));
        result.code = Error::Code::NoVocabFilesGiven;
        emit readFinished(result);
        return;
    }

    for (const auto &filePath : request.filePaths)
    {
        VocabFile::ReadFileResult readFileResult = readFile(filePath, request.direction);
        if (readFileResult.code != Error::Code::Success)
        {
            m_logger.error(logTag(), QStringLiteral("Failed to read vocab file '%1'! ErrorCode %2").arg(filePath, Error::codeToQString(readFileResult.code)));
            result.failedVocabCount++;
            continue;
        }

        if (readFileResult.failedEntryCount > 0)
        {
            m_logger.warning(logTag(), QStringLiteral("Skipped %1 faulty entries in vocab file '%2'").arg(QString::number(readFileResult.failedEntryCount), filePath));
            result.partiallySucceededVocabCount++;
        }
        result.failedEntryCount += readFileResult.failedEntryCount;

        result.datas.append(readFileResult.data);
    }

    int successCount = request.filePaths.count() - result.failedVocabCount;
    if (!(successCount > 0))
    {
        m_logger.error(logTag(), QStringLiteral("Failed to load any requested vocab files!"));
        result.code = Error::Code::NoSucceededVocabs;
        emit readFinished(result);
        return;
    }

    m_logger.verbose(logTag(), QStringLiteral("Handling read request done!"));

    emit readFinished(result);
}

const VocabFile::ReadFileResult VocabFileHandler::readFile(const QString &filePath, const Training::Direction direction)
{
    m_logger.verbose(logTag(), QStringLiteral("Reading vocab file '%1'...").arg(filePath));
    VocabFile::ReadFileResult result;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        m_logger.error(logTag(), QStringLiteral("Failed to open vocab file '%1': '%2' (%3)").arg(filePath, file.errorString(), QString::number(file.error())));
        result.code = Error::Code::FileOpenFailed;
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
            VocabFile::Group group;
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

        VocabFile::Entry vocabFileEntry;
        switch (direction)
        {
        case Training::Direction::KeyToValue:
            vocabFileEntry = {key, value};
            break;

        case Training::Direction::ValueToKey:
            vocabFileEntry = {value, key};
            break;

        default:
            m_logger.warning(logTag(), QStringLiteral("Unsupported training direction. Using forward..."));
            vocabFileEntry = {key, value};
            break;
        }

        if (result.data.groups.isEmpty())
        {
            // Vocab has not yet any groups. Append pair to entries
            m_logger.verbose(logTag(), QStringLiteral("Adding new entry '%1' in '%2'").arg(processedLine, result.data.title));
            result.data.entries.append(vocabFileEntry);
        }
        else
        {
            // Append pair to most recent group
            VocabFile::Group *mostRecentGroup = &result.data.groups.last();
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
        result.code = Error::Code::VocabFileNoEntries;
        return result;
    }

    m_logger.debug(logTag(), QStringLiteral("Added %1 entries in vocab '%2'").arg(QString::number(entryCount), result.data.title));

    if (result.failedEntryCount > 0)
    {
        m_logger.warning(logTag(), QStringLiteral("Skipped %1 faulty entries in vocab '%2'.").arg(QString::number(result.failedEntryCount), result.data.title));
    }

    m_logger.verbose(logTag(), QStringLiteral("Reading vocab file '%1' done!").arg(filePath));
    return result;
}
