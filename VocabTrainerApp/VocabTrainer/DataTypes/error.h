#ifndef ERROR_H
#define ERROR_H

#include <QString>

namespace Error
{
    enum class Code
    {
        Success,
        FileOpenFailed,
        FileReadFailed,
        FileWriteFailed,
        InvalidJson,
        InvalidSettings,
        VocabFileNoEntries,
        NoSucceededVocabs,
        NoSucceededGroups,
        NoVocabFilesGiven,
        VocabNoEntries,
        UnsupportedDirection,
        UnsupportedBracket,
        MismatchingBrackets,
        MismatchingOptions,
        OutOfBoundsStart,
        OutOfBoundsEnd,
        Unknown
    };

    QString codeToQString(Code code);
}

#endif // ERROR_H
