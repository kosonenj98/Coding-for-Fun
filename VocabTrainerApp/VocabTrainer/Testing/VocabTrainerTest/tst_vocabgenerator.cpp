#include "tst_vocabgenerator.h"
#include "DataTypes/datatypes.h"
#include "DataTypes/results.h"

#include <QtTest>

void TestVocabGenerator::init()
{
    m_logger = std::make_unique<Logger>();
    m_vocabGenerator = std::make_unique<VocabGenerator>(*m_logger);
}

void TestVocabGenerator::generatesSimpleData()
{
    VocabFile::Data vocabFileData;
    vocabFileData.info = {"LOL", "title"};
    vocabFileData.entries = {{"k1","v1"},{"k2","v2"}};
    VocabFile::Group group;
    group.title = "group";
    group.entries = {{"gk1","gv1"},{"gk2","gv2"}};
    vocabFileData.groups.append(group);

    GenerateVocabDataResult result = m_vocabGenerator->generate(vocabFileData);

    QCOMPARE(result.code, ErrorCode::Success);
    QCOMPARE(result.failedGroupCount, 0);
    QCOMPARE(result.partiallySucceededGroupCount, 0);
    QCOMPARE(result.failedEntryCount, 0);

    Vocab::Data data = result.data;
    QCOMPARE(data.title, "title");
    QCOMPARE(data.entries.size(), 2);
    QCOMPARE(data.entries["k1"], {"v1"});
    QCOMPARE(data.entries["k2"], {"v2"});
    QCOMPARE(data.groupEntries.size(), 1);
    QCOMPARE(data.groupEntries["group"].size(), 2);
    QCOMPARE(data.groupEntries["group"]["gk1"], {"gv1"});
    QCOMPARE(data.groupEntries["group"]["gk2"], {"gv2"});
}
