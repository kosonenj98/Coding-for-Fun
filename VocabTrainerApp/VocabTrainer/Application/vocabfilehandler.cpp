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

void VocabFileHandler::read(const QString &filePath)
{
    m_logger.verbose(logTag(), QStringLiteral("Reading vocab file '%1'...").arg(filePath));

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        m_logger.error(logTag(), QStringLiteral("Failed to open vocab file '%1': '%2' (%3)").arg(filePath, file.errorString(), QString::number(file.error())));
        m_logger.verbose(logTag(), QStringLiteral("Emitting failure signal..."));
        emit readFailed(ErrorCode::FileOpenFailed);
        return;
    }

    // Parse vocab file data
    VocabFileData data;
    const QStringList lines = QString::fromUtf8(file.readAll()).split('\n');
    ParseState state = ParseState::ExpectVocabularyTitle;
    int entryCount = 0;
    int failedEntryCount = 0;
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
                data.title = processedLine;
                state = ParseState::ReadingEntries;
                continue;
            }

            // Line is group title
            m_logger.verbose(logTag(), QStringLiteral("Set group title: '%1'").arg(processedLine));
            VocabFileGroup group;
            group.title = processedLine;
            data.groups.append(group);
            continue;
        }

        if (state == ParseState::ExpectVocabularyTitle)
        {
            // Vocab file has no title
            m_logger.warning(logTag(), QStringLiteral("Vocab file '%1' has no explicit title. Using file name instead...").arg(filePath));
            data.title = QFileInfo(filePath).completeBaseName();
            state = ParseState::ReadingEntries;
        }

        const QStringList parts = processedLine.split("=");
        if (parts.size() != 2)
        {
            // Invalid syntax
            m_logger.warning(logTag(), QStringLiteral("Cannot parse entry: '%1'. Skipping...").arg(processedLine));
            failedEntryCount++;
            continue;
        }

        const QString &key = parts.first().trimmed();
        const QString &value = parts.last().trimmed();
        if (key.isEmpty() || value.isEmpty())
        {
            // Invalid syntax
            m_logger.warning(logTag(), QStringLiteral("Empty values in entry: '%1'. Skipping...").arg(processedLine));
            failedEntryCount++;
            continue;
        }

        const VocabFileEntry vocabFileEntry = {key, value};
        if (data.groups.isEmpty())
        {
            // Vocab has not yet any groups. Append pair to entries
            m_logger.verbose(logTag(), QStringLiteral("Adding new entry '%1' in '%2'").arg(processedLine, data.title));
            data.entries.append(vocabFileEntry);
        }
        else
        {
            // Append pair to most recent group
            VocabFileGroup *mostRecentGroup = &data.groups.last();
            m_logger.verbose(logTag(), QStringLiteral("Adding new entry '%1' in '%2':'%3'").arg(processedLine, data.title, mostRecentGroup->title));
            mostRecentGroup->entries.append(vocabFileEntry);
        }

        entryCount++;
    }

    m_logger.debug(logTag(), QStringLiteral("Reading vocab file '%1' succeeded!").arg(filePath));

    if (!(entryCount > 0))
    {
        // Vocab contained no entries
        m_logger.error(logTag(), QStringLiteral("No valid entries found in vocab '%1'.").arg(data.title));
        m_logger.verbose(logTag(), QStringLiteral("Emitting failure signal..."));
        emit readFailed(ErrorCode::VocabFileNoEntries);
        return;
    }

    m_logger.debug(logTag(), QStringLiteral("Added %1 entries in vocab '%2'").arg(QString::number(entryCount), data.title));

    if (failedEntryCount > 0)
    {
        m_logger.warning(logTag(), QStringLiteral("Skipped %1 faulty entries in vocab '%2'.").arg(QString::number(failedEntryCount), data.title));
        m_logger.verbose(logTag(), QStringLiteral("Emitting partial success signal..."));
        emit readSucceededPartially(data, failedEntryCount);
        return;
    }

    m_logger.verbose(logTag(), QStringLiteral("Reading vocab file '%1' done!").arg(filePath));
    m_logger.verbose(logTag(), QStringLiteral("Emitting success signal..."));
    emit readSucceeded(data);
}
