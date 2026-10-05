#ifndef VOCABFILEDATATYPES_H
#define VOCABFILEDATATYPES_H

#include <QList>
#include <QString>

using VocabFileEntry = QPair<QString,QString>;

struct VocabFileGroup
{
    QString title;
    QList<VocabFileEntry> entries;
};

struct VocabFileData
{
    QString title;
    QList<VocabFileEntry> entries;
    QList<VocabFileGroup> groups;
};

#endif // VOCABFILEDATATYPES_H
