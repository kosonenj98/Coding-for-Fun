#ifndef VOCABGENERATOR_H
#define VOCABGENERATOR_H

#include "DataTypes/datatypes.h"

class VocabGenerator
{
public:
    VocabGenerator();

    const Vocab::Data generate(const VocabFile::Data& vocabFileData);

private:
    const Vocab::Entries generateVariants(VocabFile::Entry vocabFileEntry);
};

#endif // VOCABGENERATOR_H
