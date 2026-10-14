#ifndef VOCABGENERATION_H
#define VOCABGENERATION_H

#include "vocabfile.h"
#include "vocab.h"

#include <QSet>

namespace VocabGeneration
{
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

    struct ExtractBracketPartResult
    {
        Error::Code code = Error::Code::Success;
        BracketParts parts;
    };

    struct ExtractBracketPartsResult
    {
        Error::Code code = Error::Code::Success;
        QPair<BracketParts,BracketParts> parts;
    };

    struct HandleBracketsResult
    {
        Error::Code code = Error::Code::Success;
        QSet<VocabFile::Entry> entries;
    };

    struct GenerateVariantsResult
    {
        Error::Code code = Error::Code::Success;
        Vocab::Entries entries;
    };

    struct GenerateEntriesResult
    {
        Error::Code code = Error::Code::Success;
        int failedEntryCount = 0;
        Vocab::Entries entries;
    };

    struct GenerationResult
    {
        Error::Code code = Error::Code::Success;
        int failedGroupCount = 0;
        int partiallySucceededGroupCount = 0;
        int failedEntryCount = 0;
        Vocab::Data data;
    };
}

#endif // VOCABGENERATION_H
