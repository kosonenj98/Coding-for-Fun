#ifndef VOCABDATATYPES_H
#define VOCABDATATYPES_H

#include <QMultiMap>
#include <QString>

namespace Vocab
{
    using Entries = QMultiMap<QString,QString>;

    using GroupEntries = QMultiMap<QString,Entries>;

    struct Data
    {
        QString title;
        Entries entries;
        GroupEntries groupEntries;
    };
}

#endif // VOCABDATATYPES_H
