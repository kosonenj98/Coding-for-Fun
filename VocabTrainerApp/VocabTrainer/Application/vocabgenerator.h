#ifndef VOCABGENERATOR_H
#define VOCABGENERATOR_H

#include "DataTypes/datatypes.h"
#include "DataTypes/results.h"
#include "Logging/logger.h"

enum class BracketType
{
    Parenthesis,
    Square,
    Curly
};

struct BracketParts
{
    QString before;
    QString inside;
    QString after;
};

struct ExtractBracketPartsResult
{
    ErrorCode code = ErrorCode::Success;
    QPair<BracketParts,BracketParts> parts;
};

struct ExtractBracketPartResult
{
    ErrorCode code = ErrorCode::Success;
    BracketParts parts;
};

struct HandleBracketsResult
{
    ErrorCode code = ErrorCode::Success;
    QList<VocabFile::Entry> entries;
};

class VocabGenerator
{
public:
    VocabGenerator(Logger &logger);

    const GenerateVocabDataResult generate(const VocabFile::Data& vocabFileData);

private:
    const GenerateVocabEntriesResult handleVocabFileEntries(const QList<VocabFile::Entry> &entries);
    const GenerateVocabEntryVariantsResult generateVariants(const VocabFile::Entry &vocabFileEntry);

    QPair<QList<VocabFile::Entry>,QList<VocabFile::Entry>> extractEntriesWithVariants(const QList<VocabFile::Entry> &entries);
    const ExtractBracketPartsResult extractBracketParts(const VocabFile::Entry &entry, const BracketType &type);
    const ExtractBracketPartResult extractBracketPart(const QString &text, const BracketType &type);
    const QString toString(const BracketParts &parts, bool inside = true);

    const HandleBracketsResult handleCurlyBraces(const QPair<BracketParts,BracketParts> &parts);
    const HandleBracketsResult handleParenthesis(const QPair<BracketParts,BracketParts> &parts);
    const HandleBracketsResult handleSquareBrackets(const QPair<BracketParts,BracketParts> &parts);

    void uniteEntries(Vocab::Entries &entries, const Vocab::Entries &newEntries);
    void uniteGroups(Vocab::GroupEntries &groupEntries, const Vocab::GroupEntries &newGroupEntries);

    Logger &m_logger;
};

#endif // VOCABGENERATOR_H
