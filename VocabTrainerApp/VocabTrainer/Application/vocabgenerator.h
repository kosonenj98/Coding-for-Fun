#ifndef VOCABGENERATOR_H
#define VOCABGENERATOR_H

#include "Logging/logger.h"
#include "DataTypes/vocabgeneration.h"

class VocabGenerator
{
public:
    VocabGenerator(Logger &logger);

    const VocabGeneration::GenerationResult generate(const VocabFile::Data& vocabFileData);

private:
    const VocabGeneration::GenerateEntriesResult handleVocabFileEntries(const QList<VocabFile::Entry> &entries);
    const VocabGeneration::GenerateVariantsResult generateVariants(const VocabFile::Entry &vocabFileEntry);

    QPair<QSet<VocabFile::Entry>, QSet<VocabFile::Entry>> extractEntriesWithVariants(const QSet<VocabFile::Entry> &entries);
    const VocabGeneration::ExtractBracketPartsResult extractInnermostBracketParts(const VocabFile::Entry &entry, const VocabGeneration::BracketType &type);
    const VocabGeneration::ExtractBracketPartResult extractInnermostBracketPart(const QString &text, const VocabGeneration::BracketType &type);
    const QString toString(VocabGeneration::BracketParts parts, bool inside = true);

    const VocabGeneration::HandleBracketsResult handleCurlyBraces(const QPair<VocabGeneration::BracketParts, VocabGeneration::BracketParts> &parts);
    const VocabGeneration::HandleBracketsResult handleParenthesis(const QPair<VocabGeneration::BracketParts, VocabGeneration::BracketParts> &parts);
    const VocabGeneration::HandleBracketsResult handleSquareBrackets(const QPair<VocabGeneration::BracketParts, VocabGeneration::BracketParts> &parts);

    void uniteEntries(Vocab::Entries &entries, const Vocab::Entries &newEntries);
    void uniteGroups(Vocab::GroupEntries &groupEntries, const Vocab::GroupEntries &newGroupEntries);

    Logger &m_logger;
};

#endif // VOCABGENERATOR_H
