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

void TestVocabGenerator::generatesVariantsWithCurlyBraces()
{
    VocabFile::Data vocabFileData;
    vocabFileData.info = {"LOL", "title"};
    vocabFileData.entries = {{"{k1} a","v1"},{"k2","{v2} a"},{"{k3} a","{v3} b"}};
    VocabFile::Group group;
    group.title = "group";
    group.entries = {{"{gk1} a","gv1"},{"gk2","{gv2} a"}, {"{gk3} a","{gv3} b"}};
    vocabFileData.groups.append(group);

    GenerateVocabDataResult result = m_vocabGenerator->generate(vocabFileData);

    QCOMPARE(result.code, ErrorCode::Success);
    QCOMPARE(result.failedGroupCount, 0);
    QCOMPARE(result.partiallySucceededGroupCount, 0);
    QCOMPARE(result.failedEntryCount, 0);

    Vocab::Data data = result.data;
    QCOMPARE(data.title, "title");
    QCOMPARE(data.entries.size(), 4);
    QCOMPARE(data.entries["k1 a"], {"v1"});
    QCOMPARE(data.entries["a"], QSet<QString>({"v1", "v3 b", "b"}));
    QCOMPARE(data.entries["k2"], QSet<QString>({"a", "v2 a"}));
    QCOMPARE(data.entries["k3 a"], QSet<QString>({"v3 b", "b"}));

    QCOMPARE(data.groupEntries.size(), 1);
    QCOMPARE(data.groupEntries["group"].size(), 4);
    QCOMPARE(data.groupEntries["group"]["gk1 a"], {"gv1"});
    QCOMPARE(data.groupEntries["group"]["a"], QSet<QString>({"gv1", "gv3 b", "b"}));
    QCOMPARE(data.groupEntries["group"]["gk2"], QSet<QString>({"gv2 a", "a"}));
    QCOMPARE(data.groupEntries["group"]["gk3 a"], QSet<QString>({"gv3 b", "b"}));
}

void TestVocabGenerator::generatesVariantsWithParenthesis()
{
    VocabFile::Data vocabFileData;
    vocabFileData.info = {"LOL", "title"};
    vocabFileData.entries = {{"(k1) a","v1"},{"k2","(v2) a"},{"(k3) a","(v3) b"}};
    VocabFile::Group group;
    group.title = "group";
    group.entries = {{"(gk1) a","gv1"},{"gk2","(gv2) a"}, {"(gk3) a","(gv3) b"}};
    vocabFileData.groups.append(group);

    GenerateVocabDataResult result = m_vocabGenerator->generate(vocabFileData);

    QCOMPARE(result.code, ErrorCode::Success);
    QCOMPARE(result.failedGroupCount, 0);
    QCOMPARE(result.partiallySucceededGroupCount, 0);
    QCOMPARE(result.failedEntryCount, 0);

    Vocab::Data data = result.data;
    QCOMPARE(data.title, "title");
    QCOMPARE(data.entries.size(), 4);
    QCOMPARE(data.entries["k1 a"], {"v1"});
    QCOMPARE(data.entries["a"], QSet<QString>({"v1", "b"}));
    QCOMPARE(data.entries["k2"], QSet<QString>({"a", "v2 a"}));
    QCOMPARE(data.entries["k3 a"], QSet<QString>({"v3 b"}));

    QCOMPARE(data.groupEntries.size(), 1);
    QCOMPARE(data.groupEntries["group"].size(), 4);
    QCOMPARE(data.groupEntries["group"]["gk1 a"], {"gv1"});
    QCOMPARE(data.groupEntries["group"]["a"], QSet<QString>({"gv1", "b"}));
    QCOMPARE(data.groupEntries["group"]["gk2"], QSet<QString>({"gv2 a", "a"}));
    QCOMPARE(data.groupEntries["group"]["gk3 a"], QSet<QString>({"gv3 b"}));
}

void TestVocabGenerator::generatesVariantsWithSquareBrackets()
{
    VocabFile::Data vocabFileData;
    vocabFileData.info = {"LOL", "title"};
    vocabFileData.entries = {{"[k1o1/k1o2] a","v1"},{"k2","[v2o1/v2o2] a"},{"[k3o1/k3o2] a","[v3o1/v3o2] b"}};
    VocabFile::Group group;
    group.title = "group";
    group.entries = {{"[gk1o1/gk1o2] a","gv1"},{"gk2","[gv2o1/gv2o2] a"}, {"[gk3o1/gk3o2] a","[gv3o1/gv3o2] b"}};
    vocabFileData.groups.append(group);

    GenerateVocabDataResult result = m_vocabGenerator->generate(vocabFileData);

    QCOMPARE(result.code, ErrorCode::Success);
    QCOMPARE(result.failedGroupCount, 0);
    QCOMPARE(result.partiallySucceededGroupCount, 0);
    QCOMPARE(result.failedEntryCount, 0);

    Vocab::Data data = result.data;
    QCOMPARE(data.title, "title");
    QCOMPARE(data.entries.size(), 5);
    QCOMPARE(data.entries["k1o1 a"], {"v1"});
    QCOMPARE(data.entries["k1o2 a"], {"v1"});
    QCOMPARE(data.entries["k2"], QSet<QString>({"v2o1 a", "v2o2 a"}));
    QCOMPARE(data.entries["k3o1 a"], QSet<QString>({"v3o1 b"}));
    QCOMPARE(data.entries["k3o2 a"], QSet<QString>({"v3o2 b"}));

    QCOMPARE(data.groupEntries.size(), 1);
    QCOMPARE(data.groupEntries["group"].size(), 5);
    QCOMPARE(data.groupEntries["group"]["gk1o1 a"], {"gv1"});
    QCOMPARE(data.groupEntries["group"]["gk1o2 a"], {"gv1"});
    QCOMPARE(data.groupEntries["group"]["gk2"], QSet<QString>({"gv2o1 a", "gv2o2 a"}));
    QCOMPARE(data.groupEntries["group"]["gk3o1 a"], QSet<QString>({"gv3o1 b"}));
    QCOMPARE(data.groupEntries["group"]["gk3o2 a"], QSet<QString>({"gv3o2 b"}));
}

void TestVocabGenerator::generatesVariantsWithNestedBrackets()
{
    VocabFile::Data vocabFileData;
    vocabFileData.info = {"LOL", "title"};
    vocabFileData.entries = {{"((a1) b1) c1","d1"}, {"{{a2} b2} c2","d2"}, {"[[a3o1/a3o2]_O1 b3O1/a3O2 b3O2] c3","d3"}};

    GenerateVocabDataResult result = m_vocabGenerator->generate(vocabFileData);

    QCOMPARE(result.code, ErrorCode::Success);
    QCOMPARE(result.failedGroupCount, 0);
    QCOMPARE(result.partiallySucceededGroupCount, 0);
    QCOMPARE(result.failedEntryCount, 0);

    Vocab::Data data = result.data;
    QCOMPARE(data.title, "title");
    QCOMPARE(data.entries.size(), 9);
    QCOMPARE(data.entries["a1 b1 c1"], {"d1"});
    QCOMPARE(data.entries["b1 c1"], {"d1"});
    QCOMPARE(data.entries["c1"], {"d1"});
    QCOMPARE(data.entries["a2 b2 c2"], {"d2"});
    QCOMPARE(data.entries["b2 c2"], {"d2"});
    QCOMPARE(data.entries["c2"], {"d2"});
    QCOMPARE(data.entries["a3o1O1 b3O1 c3"], {"d3"});
    QCOMPARE(data.entries["a3o2O1 b3O1 c3"], {"d3"});
    QCOMPARE(data.entries["a3O2 b3O2 c3"], {"d3"});
}

void TestVocabGenerator::supportsVariantConcatenationWithoutSpaces()
{
    VocabFile::Data vocabFileData;
    vocabFileData.info = {"LOL", "title"};
    vocabFileData.entries = {{"a (k1)_b","v1"},{"k2","a (v2)_b"},{"a_(k3)_b","a_(v3)_b"}};

    GenerateVocabDataResult result = m_vocabGenerator->generate(vocabFileData);

    QCOMPARE(result.code, ErrorCode::Success);
    QCOMPARE(result.failedGroupCount, 0);
    QCOMPARE(result.partiallySucceededGroupCount, 0);
    QCOMPARE(result.failedEntryCount, 0);

    Vocab::Data data = result.data;
    QCOMPARE(data.title, "title");
    QCOMPARE(data.entries.size(), 5);
    QCOMPARE(data.entries["a k1b"], {"v1"});
    QCOMPARE(data.entries["a b"], {"v1"});
    QCOMPARE(data.entries["k2"], QSet<QString>({"a b", "a v2b"}));
    QCOMPARE(data.entries["ab"], QSet<QString>({"ab"}));
    QCOMPARE(data.entries["ak3b"], QSet<QString>({"av3b"}));
}
