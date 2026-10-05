#include "../../Logging/logger.h"
#include "../../Application/vocabfilehandler.h"

#include <QtTest>

class TestVocabFileHandler : public QObject
{
    Q_OBJECT

private slots:
    void init();

    // Positive tests
    void readsSimpleVocabulary();
    void readsVocabularyWithGroups();
    void readsVocabularyWithEntriesAndGroups();
    void readsVocabularyWithoutTitle();
    void readsTitleWithSpaces();
    void readsEntryWithSpaces();
    void readsComments();
    void readsEmptyLines();

    // Negative tests
    void rejectsInvalidEntry();
    void rejectsInvalidEntryInGroup();
    void failsWhenFileDoesNotExist();
    void failsWhenNoEntries();
    void failsWhenNoValidEntries();

private:
    std::unique_ptr<Logger> m_logger;
    std::unique_ptr<VocabFileHandler> m_handler;
};

// Initialize new logger and VocabFile for each test
void TestVocabFileHandler::init()
{
    m_logger = std::make_unique<Logger>();
    m_handler = std::make_unique<VocabFileHandler>(*m_logger);
}

// Reads simple vocab file with title and entries
void TestVocabFileHandler::readsSimpleVocabulary()
{
    // Create temporary vocab file and write its contents
    QTemporaryFile file;
    QVERIFY(file.open());

    const QString vocabTitle = QStringLiteral("vocabTitle");
    const QString k1 = QStringLiteral("k1");
    const QString k2 = QStringLiteral("k2");
    const QString v1 = QStringLiteral("v1");
    const QString v2 = QStringLiteral("v2");
    const QStringList lines = {
        vocabTitle,
        QString(),
        QStringLiteral("%1=%2").arg(k1, v1),
        QStringLiteral("%1=%2").arg(k2, v2),
    };
    const QByteArray content = lines.join('\n').toUtf8();

    QVERIFY(file.write(content) != -1);
    file.close();

    // Spy on signals emitted by VocabFileHandler
    QSignalSpy successSpy(m_handler.get(), &VocabFileHandler::readSucceeded);
    QSignalSpy partialSpy(m_handler.get(), &VocabFileHandler::readSucceededPartially);
    QSignalSpy failedSpy(m_handler.get(), &VocabFileHandler::readFailed);

    // Read test vocab file
    m_handler->read(file.fileName());

    // Read should succeed
    QCOMPARE(successSpy.count(), 1);
    QCOMPARE(partialSpy.count(), 0);
    QCOMPARE(failedSpy.count(), 0);

    // Parse vocab file data from signal
    const QList<QVariant> arguments = successSpy.takeFirst();
    const VocabFileData data = qvariant_cast<VocabFileData>(arguments.at(0));

    // Validate vocab file data
    QCOMPARE(data.title, vocabTitle);
    QCOMPARE(data.entries.size(), 2);
    QCOMPARE(data.groups.size(), 0);
    QCOMPARE(data.entries.at(0).first, k1);
    QCOMPARE(data.entries.at(0).second, v1);
    QCOMPARE(data.entries.at(1).first, k2);
    QCOMPARE(data.entries.at(1).second, v2);
}

// Reads vocab file with title and two groups
void TestVocabFileHandler::readsVocabularyWithGroups()
{
    // Create temporary vocab file and write its contents
    QTemporaryFile file;
    QVERIFY(file.open());

    const QString vocabTitle = QStringLiteral("vocabTitle");
    const QString g1Title = QStringLiteral("g1Title");
    const QString g1k1 = QStringLiteral("g1k1");
    const QString g1k2 = QStringLiteral("g1k2");
    const QString g1v1 = QStringLiteral("g1v1");
    const QString g1v2 = QStringLiteral("g1v2");
    const QString g2Title = QStringLiteral("g2Title");
    const QString g2k1 = QStringLiteral("g2k1");
    const QString g2k2 = QStringLiteral("g2k2");
    const QString g2v1 = QStringLiteral("g2v1");
    const QString g2v2 = QStringLiteral("g2v2");

    const QStringList lines = {
        vocabTitle,
        QString(),
        g1Title,
        QStringLiteral("%1=%2").arg(g1k1, g1v1),
        QStringLiteral("%1=%2").arg(g1k2, g1v2),
        QString(),
        g2Title,
        QStringLiteral("%1=%2").arg(g2k1, g2v1),
        QStringLiteral("%1=%2").arg(g2k2, g2v2)
    };

    const QByteArray content = lines.join('\n').toUtf8();

    QVERIFY(file.write(content) != -1);
    file.close();

    // Spy on signals emitted by VocabFileHandler
    QSignalSpy successSpy(m_handler.get(), &VocabFileHandler::readSucceeded);
    QSignalSpy partialSpy(m_handler.get(), &VocabFileHandler::readSucceededPartially);
    QSignalSpy failedSpy(m_handler.get(), &VocabFileHandler::readFailed);

    // Read test vocab file
    m_handler->read(file.fileName());

    // Read should succeed
    QCOMPARE(successSpy.count(), 1);
    QCOMPARE(partialSpy.count(), 0);
    QCOMPARE(failedSpy.count(), 0);

    // Parse vocab file data from signal
    const QList<QVariant> arguments = successSpy.takeFirst();
    const VocabFileData data = qvariant_cast<VocabFileData>(arguments.at(0));

    // Validate vocab file data
    QCOMPARE(data.title, vocabTitle);
    QCOMPARE(data.entries.size(), 0);
    QCOMPARE(data.groups.size(), 2);

    // Group 1
    const auto &g1 = data.groups.at(0);
    QCOMPARE(g1.title, g1Title);
    QCOMPARE(g1.entries.size(), 2);
    QCOMPARE(g1.entries.at(0).first, g1k1);
    QCOMPARE(g1.entries.at(0).second, g1v1);
    QCOMPARE(g1.entries.at(1).first, g1k2);
    QCOMPARE(g1.entries.at(1).second, g1v2);

    // Group 2
    const auto &g2 = data.groups.at(1);
    QCOMPARE(g2.title, g2Title);
    QCOMPARE(g2.entries.size(), 2);
    QCOMPARE(g2.entries.at(0).first, g2k1);
    QCOMPARE(g2.entries.at(0).second, g2v1);
    QCOMPARE(g2.entries.at(1).first, g2k2);
    QCOMPARE(g2.entries.at(1).second, g2v2);
}

// Reads vocab file with title, entries and two groups
void TestVocabFileHandler::readsVocabularyWithEntriesAndGroups()
{
    // Create temporary vocab file and write its contents
    QTemporaryFile file;
    QVERIFY(file.open());

    const QString vocabTitle = QStringLiteral("vocabTitle");
    const QString k1 = QStringLiteral("k1");
    const QString k2 = QStringLiteral("k2");
    const QString v1 = QStringLiteral("v1");
    const QString v2 = QStringLiteral("v2");
    const QString g1Title = QStringLiteral("g1Title");
    const QString g1k1 = QStringLiteral("g1k1");
    const QString g1k2 = QStringLiteral("g1k2");
    const QString g1v1 = QStringLiteral("g1v1");
    const QString g1v2 = QStringLiteral("g1v2");
    const QString g2Title = QStringLiteral("g2Title");
    const QString g2k1 = QStringLiteral("g2k1");
    const QString g2k2 = QStringLiteral("g2k2");
    const QString g2v1 = QStringLiteral("g2v1");
    const QString g2v2 = QStringLiteral("g2v2");

    const QStringList lines = {
        vocabTitle,
        QString(),
        QStringLiteral("%1=%2").arg(k1, v1),
        QStringLiteral("%1=%2").arg(k2, v2),
        QString(),
        g1Title,
        QStringLiteral("%1=%2").arg(g1k1, g1v1),
        QStringLiteral("%1=%2").arg(g1k2, g1v2),
        QString(),
        g2Title,
        QStringLiteral("%1=%2").arg(g2k1, g2v1),
        QStringLiteral("%1=%2").arg(g2k2, g2v2)
    };

    const QByteArray content = lines.join('\n').toUtf8();

    QVERIFY(file.write(content) != -1);
    file.close();

    // Spy on signals emitted by VocabFileHandler
    QSignalSpy successSpy(m_handler.get(), &VocabFileHandler::readSucceeded);
    QSignalSpy partialSpy(m_handler.get(), &VocabFileHandler::readSucceededPartially);
    QSignalSpy failedSpy(m_handler.get(), &VocabFileHandler::readFailed);

    // Read test vocab file
    m_handler->read(file.fileName());

    // Read should succeed
    QCOMPARE(successSpy.count(), 1);
    QCOMPARE(partialSpy.count(), 0);
    QCOMPARE(failedSpy.count(), 0);

    // Parse vocab file data from signal
    const QList<QVariant> arguments = successSpy.takeFirst();
    const VocabFileData data = qvariant_cast<VocabFileData>(arguments.at(0));

    // Validate vocab file data
    QCOMPARE(data.title, vocabTitle);
    QCOMPARE(data.entries.size(), 2);
    QCOMPARE(data.groups.size(), 2);

    // Entries
    QCOMPARE(data.entries.at(0).first, k1);
    QCOMPARE(data.entries.at(0).second, v1);
    QCOMPARE(data.entries.at(1).first, k2);
    QCOMPARE(data.entries.at(1).second, v2);

    // Group 1
    const auto &g1 = data.groups.at(0);
    QCOMPARE(g1.title, g1Title);
    QCOMPARE(g1.entries.size(), 2);
    QCOMPARE(g1.entries.at(0).first, g1k1);
    QCOMPARE(g1.entries.at(0).second, g1v1);
    QCOMPARE(g1.entries.at(1).first, g1k2);
    QCOMPARE(g1.entries.at(1).second, g1v2);

    // Group 2
    const auto &g2 = data.groups.at(1);
    QCOMPARE(g2.title, g2Title);
    QCOMPARE(g2.entries.size(), 2);
    QCOMPARE(g2.entries.at(0).first, g2k1);
    QCOMPARE(g2.entries.at(0).second, g2v1);
    QCOMPARE(g2.entries.at(1).first, g2k2);
    QCOMPARE(g2.entries.at(1).second, g2v2);
}

// Reads vocab file without title
void TestVocabFileHandler::readsVocabularyWithoutTitle()
{
    const QString k1 = QStringLiteral("k1");
    const QString k2 = QStringLiteral("k2");
    const QString v1 = QStringLiteral("v1");
    const QString v2 = QStringLiteral("v2");
    const QStringList lines = {
        QStringLiteral("%1=%2").arg(k1, v1),
        QStringLiteral("%1=%2").arg(k2, v2),
    };
    const QByteArray content = lines.join('\n').toUtf8();

    // Create temporary vocab files and write contents
    QTemporaryFile file1;
    QVERIFY(file1.open());
    QVERIFY(file1.write(content) != -1);
    file1.close();

    QTemporaryFile file2;
    QVERIFY(file2.open());
    const QString originalPath = file2.fileName();
    const QString newPath = originalPath + QStringLiteral(".extra.txt");
    file2.close();
    QVERIFY(file2.rename(newPath));
    QVERIFY(file2.open());
    QVERIFY(file2.write(content) != -1);
    file2.close();

    // Spy on signals emitted by VocabFileHandler
    QSignalSpy successSpy(m_handler.get(), &VocabFileHandler::readSucceeded);
    QSignalSpy partialSpy(m_handler.get(), &VocabFileHandler::readSucceededPartially);
    QSignalSpy failedSpy(m_handler.get(), &VocabFileHandler::readFailed);

    // Read test vocab file
    m_handler->read(file1.fileName());
    m_handler->read(file2.fileName());

    // Read should succeed
    QCOMPARE(successSpy.count(), 2);
    QCOMPARE(partialSpy.count(), 0);
    QCOMPARE(failedSpy.count(), 0);

    // Parse vocab file data from signal
    QStringList fileNames = {QFileInfo(file1).completeBaseName(), QFileInfo(file2).completeBaseName()};
    for (int i=0; i < successSpy.count(); i++)
    {
        const QList<QVariant> arguments = successSpy.at(i);
        const VocabFileData data = qvariant_cast<VocabFileData>(arguments.at(0));

        // Validate vocab file data
        QCOMPARE(data.title, fileNames.at(i));
        QCOMPARE(data.entries.size(), 2);
        QCOMPARE(data.groups.size(), 0);
        QCOMPARE(data.entries.at(0).first, k1);
        QCOMPARE(data.entries.at(0).second, v1);
        QCOMPARE(data.entries.at(1).first, k2);
        QCOMPARE(data.entries.at(1).second, v2);
    }
}

// Reads vocab and group titles with spaces
void TestVocabFileHandler::readsTitleWithSpaces()
{
    // Create temporary vocab file and write its contents
    QTemporaryFile file;
    QVERIFY(file.open());

    const QString vocabTitle = QStringLiteral("vocab Title");
    const QString g1Title = QStringLiteral("g1 Title");
    const QString g1k1 = QStringLiteral("g1k1");
    const QString g1k2 = QStringLiteral("g1k2");
    const QString g1v1 = QStringLiteral("g1v1");
    const QString g1v2 = QStringLiteral("g1v2");
    const QString g2Title = QStringLiteral("g1    Title");
    const QString g2k1 = QStringLiteral("g2k1");
    const QString g2k2 = QStringLiteral("g2k2");
    const QString g2v1 = QStringLiteral("g2v1");
    const QString g2v2 = QStringLiteral("g2v2");
    const QStringList lines = {
        QStringLiteral(" %1  ").arg(vocabTitle),
        QString(),
        QStringLiteral("   %1 ").arg(g1Title),
        QStringLiteral("%1=%2").arg(g1k1, g1v1),
        QStringLiteral("%1=%2").arg(g1k2, g1v2),
        QString(),
        QStringLiteral(" %1    ").arg(g2Title),
        QStringLiteral("%1=%2").arg(g2k1, g2v1),
        QStringLiteral("%1=%2").arg(g2k2, g2v2),
    };
    const QByteArray content = lines.join('\n').toUtf8();

    QVERIFY(file.write(content) != -1);
    file.close();

    // Spy on signals emitted by VocabFileHandler
    QSignalSpy successSpy(m_handler.get(), &VocabFileHandler::readSucceeded);
    QSignalSpy partialSpy(m_handler.get(), &VocabFileHandler::readSucceededPartially);
    QSignalSpy failedSpy(m_handler.get(), &VocabFileHandler::readFailed);

    // Read test vocab file
    m_handler->read(file.fileName());

    // Read should succeed
    QCOMPARE(successSpy.count(), 1);
    QCOMPARE(partialSpy.count(), 0);
    QCOMPARE(failedSpy.count(), 0);

    // Parse vocab file data from signal
    const QList<QVariant> arguments = successSpy.takeFirst();
    const VocabFileData data = qvariant_cast<VocabFileData>(arguments.at(0));

    // Validate vocab file data
    QCOMPARE(data.title, vocabTitle);
    QCOMPARE(data.entries.size(), 0);
    QCOMPARE(data.groups.size(), 2);

    // Group 1
    const auto &g1 = data.groups.at(0);
    QCOMPARE(g1.title, g1Title);
    QCOMPARE(g1.entries.size(), 2);
    QCOMPARE(g1.entries.at(0).first, g1k1);
    QCOMPARE(g1.entries.at(0).second, g1v1);
    QCOMPARE(g1.entries.at(1).first, g1k2);
    QCOMPARE(g1.entries.at(1).second, g1v2);

    // Group 2
    const auto &g2 = data.groups.at(1);
    QCOMPARE(g2.title, g2Title);
    QCOMPARE(g2.entries.size(), 2);
    QCOMPARE(g2.entries.at(0).first, g2k1);
    QCOMPARE(g2.entries.at(0).second, g2v1);
    QCOMPARE(g2.entries.at(1).first, g2k2);
    QCOMPARE(g2.entries.at(1).second, g2v2);
}

// Reads vocab file entries with spaces
void TestVocabFileHandler::readsEntryWithSpaces()
{
    // Create temporary vocab file and write its contents
    QTemporaryFile file;
    QVERIFY(file.open());

    const QString vocabTitle = QStringLiteral("vocabTitle");
    const QString k1 = QStringLiteral("k 1");
    const QString k2 = QStringLiteral("k  2");
    const QString k3 = QStringLiteral("k   3");
    const QString k4 = QStringLiteral("k    4");
    const QString k5 = QStringLiteral("k     5");
    const QString k6 = QStringLiteral("k      6");
    const QString k7 = QStringLiteral("k       7");
    const QString k8 = QStringLiteral("k        8");
    const QString k9 = QStringLiteral("k         9");
    const QString k10 = QStringLiteral("k         10");
    const QString k11 = QStringLiteral("k          11");
    const QString k12 = QStringLiteral("k           12");
    const QString k13 = QStringLiteral("k            13");
    const QString k14 = QStringLiteral("k             14");
    const QString k15 = QStringLiteral("k              15");
    const QString k16 = QStringLiteral("k               16");

    const QString v1 = QStringLiteral("v                1");
    const QString v2 = QStringLiteral("v               2");
    const QString v3 = QStringLiteral("v              3");
    const QString v4 = QStringLiteral("v             4");
    const QString v5 = QStringLiteral("v            5");
    const QString v6 = QStringLiteral("v           6");
    const QString v7 = QStringLiteral("v          7");
    const QString v8 = QStringLiteral("v         8");
    const QString v9 = QStringLiteral("v        9");
    const QString v10 = QStringLiteral("v      10");
    const QString v11 = QStringLiteral("v     11");
    const QString v12 = QStringLiteral("v    12");
    const QString v13 = QStringLiteral("v   13");
    const QString v14 = QStringLiteral("v  14");
    const QString v15 = QStringLiteral("v 15");
    const QString v16 = QStringLiteral("v16");

    const QStringList lines = {
        QStringLiteral("%1").arg(vocabTitle),
        QString(),
        QStringLiteral("%1=%2").arg(k1, v1),
        QStringLiteral("%1=%2 ").arg(k2, v2),
        QStringLiteral("%1= %2").arg(k3, v3),
        QStringLiteral("%1= %2 ").arg(k4, v4),
        QStringLiteral("%1 =%2").arg(k5, v5),
        QStringLiteral("%1 =%2 ").arg(k6, v6),
        QStringLiteral("%1 = %2").arg(k7, v7),
        QStringLiteral("%1 = %2 ").arg(k8, v8),
        QStringLiteral(" %1=%2").arg(k9, v9),
        QStringLiteral(" %1=%2 ").arg(k10, v10),
        QStringLiteral(" %1= %2").arg(k11, v11),
        QStringLiteral(" %1= %2 ").arg(k12, v12),
        QStringLiteral(" %1 =%2").arg(k13, v13),
        QStringLiteral(" %1 =%2 ").arg(k14, v14),
        QStringLiteral(" %1 = %2").arg(k15, v15),
        QStringLiteral(" %1 = %2 ").arg(k16, v16)
    };
    const QByteArray content = lines.join('\n').toUtf8();

    QVERIFY(file.write(content) != -1);
    file.close();

    // Spy on signals emitted by VocabFileHandler
    QSignalSpy successSpy(m_handler.get(), &VocabFileHandler::readSucceeded);
    QSignalSpy partialSpy(m_handler.get(), &VocabFileHandler::readSucceededPartially);
    QSignalSpy failedSpy(m_handler.get(), &VocabFileHandler::readFailed);

    // Read test vocab file
    m_handler->read(file.fileName());

    // Read should succeed
    QCOMPARE(successSpy.count(), 1);
    QCOMPARE(partialSpy.count(), 0);
    QCOMPARE(failedSpy.count(), 0);

    // Parse vocab file data from signal
    const QList<QVariant> arguments = successSpy.takeFirst();
    const VocabFileData data = qvariant_cast<VocabFileData>(arguments.at(0));

    // Validate vocab file data
    QCOMPARE(data.title, vocabTitle);
    QCOMPARE(data.entries.size(), 16);
    QCOMPARE(data.groups.size(), 0);
    QCOMPARE(data.entries.at(0).first, k1);
    QCOMPARE(data.entries.at(0).second, v1);
    QCOMPARE(data.entries.at(1).first, k2);
    QCOMPARE(data.entries.at(1).second, v2);
    QCOMPARE(data.entries.at(2).first, k3);
    QCOMPARE(data.entries.at(2).second, v3);
    QCOMPARE(data.entries.at(3).first, k4);
    QCOMPARE(data.entries.at(3).second, v4);
    QCOMPARE(data.entries.at(4).first, k5);
    QCOMPARE(data.entries.at(4).second, v5);
    QCOMPARE(data.entries.at(5).first, k6);
    QCOMPARE(data.entries.at(5).second, v6);
    QCOMPARE(data.entries.at(6).first, k7);
    QCOMPARE(data.entries.at(6).second, v7);
    QCOMPARE(data.entries.at(7).first, k8);
    QCOMPARE(data.entries.at(7).second, v8);
    QCOMPARE(data.entries.at(8).first, k9);
    QCOMPARE(data.entries.at(8).second, v9);
    QCOMPARE(data.entries.at(9).first, k10);
    QCOMPARE(data.entries.at(9).second, v10);
    QCOMPARE(data.entries.at(10).first, k11);
    QCOMPARE(data.entries.at(10).second, v11);
    QCOMPARE(data.entries.at(11).first, k12);
    QCOMPARE(data.entries.at(11).second, v12);
    QCOMPARE(data.entries.at(12).first, k13);
    QCOMPARE(data.entries.at(12).second, v13);
    QCOMPARE(data.entries.at(13).first, k14);
    QCOMPARE(data.entries.at(13).second, v14);
    QCOMPARE(data.entries.at(14).first, k15);
    QCOMPARE(data.entries.at(14).second, v15);
    QCOMPARE(data.entries.at(15).first, k16);
    QCOMPARE(data.entries.at(15).second, v16);
}

// Reads vocab file with comments
void TestVocabFileHandler::readsComments()
{
    // Create temporary vocab file and write its contents
    QTemporaryFile file;
    QVERIFY(file.open());

    const QString vocabTitle = QStringLiteral("vocabTitle");
    const QString vocabTitle2 = QStringLiteral("vocabTitle2");
    const QString k1 = QStringLiteral("k1");
    const QString k2 = QStringLiteral("k2");
    const QString k3 = QStringLiteral("k3");
    const QString v1 = QStringLiteral("v1");
    const QString v2 = QStringLiteral("v2");
    const QString v3 = QStringLiteral("v3");
    const QString gTitle = QStringLiteral("gTitle");
    const QString comment1 = QStringLiteral("comment1");
    const QString comment2 = QStringLiteral("comment2");

    const QStringList lines = {
        QStringLiteral("# %1").arg(comment1),
        vocabTitle,
        QStringLiteral("# %1").arg(vocabTitle2),
        QString(),
        QStringLiteral("#"),
        QStringLiteral("###########"),
        QStringLiteral("%1=%2").arg(k1, v1),
        QStringLiteral("#%1").arg(gTitle),
        QStringLiteral("%1=%2 # %3").arg(k2, v2, comment2),
        QStringLiteral("#%1=%2").arg(k3, v3)
    };
    const QByteArray content = lines.join('\n').toUtf8();

    QVERIFY(file.write(content) != -1);
    file.close();

    // Spy on signals emitted by VocabFileHandler
    QSignalSpy successSpy(m_handler.get(), &VocabFileHandler::readSucceeded);
    QSignalSpy partialSpy(m_handler.get(), &VocabFileHandler::readSucceededPartially);
    QSignalSpy failedSpy(m_handler.get(), &VocabFileHandler::readFailed);

    // Read test vocab file
    m_handler->read(file.fileName());

    // Read should succeed
    QCOMPARE(successSpy.count(), 1);
    QCOMPARE(partialSpy.count(), 0);
    QCOMPARE(failedSpy.count(), 0);

    // Parse vocab file data from signal
    const QList<QVariant> arguments = successSpy.takeFirst();
    const VocabFileData data = qvariant_cast<VocabFileData>(arguments.at(0));

    // Validate vocab file data
    QCOMPARE(data.title, vocabTitle);
    QCOMPARE(data.entries.size(), 2);
    QCOMPARE(data.groups.size(), 0);
    QCOMPARE(data.entries.at(0).first, k1);
    QCOMPARE(data.entries.at(0).second, v1);
    QCOMPARE(data.entries.at(1).first, k2);
    QCOMPARE(data.entries.at(1).second, v2);
}

// Reads vocab file with empty lines
void TestVocabFileHandler::readsEmptyLines()
{
    // Create temporary vocab file and write its contents
    QTemporaryFile file;
    QVERIFY(file.open());

    const QString vocabTitle = QStringLiteral("vocabTitle");
    const QString k1 = QStringLiteral("k1");
    const QString k2 = QStringLiteral("k2");
    const QString v1 = QStringLiteral("v1");
    const QString v2 = QStringLiteral("v2");
    const QString gTitle = QStringLiteral("gTitle");
    const QString gk1 = QStringLiteral("gk1");
    const QString gk2 = QStringLiteral("gk2");
    const QString gv1 = QStringLiteral("gv1");
    const QString gv2 = QStringLiteral("gv2");
    const QStringList lines = {
        QString(),
        QString(),
        vocabTitle,
        QString(),
        QString(),
        QString(),
        QStringLiteral("%1=%2").arg(k1, v1),
        QString(),
        QString(),
        QString(),
        QString(),
        QString(),
        QString(),
        QStringLiteral("%1=%2").arg(k2, v2),
        QString(),
        QString(),
        gTitle,
        QString(),
        QStringLiteral("%1=%2").arg(gk1, gv1),
        QString(),
        QStringLiteral("%1=%2").arg(gk2, gv2),
        QString(),
    };
    const QByteArray content = lines.join('\n').toUtf8();

    QVERIFY(file.write(content) != -1);
    file.close();

    // Spy on signals emitted by VocabFileHandler
    QSignalSpy successSpy(m_handler.get(), &VocabFileHandler::readSucceeded);
    QSignalSpy partialSpy(m_handler.get(), &VocabFileHandler::readSucceededPartially);
    QSignalSpy failedSpy(m_handler.get(), &VocabFileHandler::readFailed);

    // Read test vocab file
    m_handler->read(file.fileName());

    // Read should succeed
    QCOMPARE(successSpy.count(), 1);
    QCOMPARE(partialSpy.count(), 0);
    QCOMPARE(failedSpy.count(), 0);

    // Parse vocab file data from signal
    const QList<QVariant> arguments = successSpy.takeFirst();
    const VocabFileData data = qvariant_cast<VocabFileData>(arguments.at(0));

    // Validate vocab file data
    QCOMPARE(data.title, vocabTitle);
    QCOMPARE(data.entries.size(), 2);
    QCOMPARE(data.groups.size(), 1);
    QCOMPARE(data.entries.at(0).first, k1);
    QCOMPARE(data.entries.at(0).second, v1);
    QCOMPARE(data.entries.at(1).first, k2);
    QCOMPARE(data.entries.at(1).second, v2);

    // Group
    const auto &g = data.groups.at(0);
    QCOMPARE(g.title, gTitle);
    QCOMPARE(g.entries.size(), 2);
    QCOMPARE(g.entries.at(0).first, gk1);
    QCOMPARE(g.entries.at(0).second, gv1);
    QCOMPARE(g.entries.at(1).first, gk2);
    QCOMPARE(g.entries.at(1).second, gv2);
}

// Rejects invalid entries in vocab file
void TestVocabFileHandler::rejectsInvalidEntry()
{
    // Create temporary vocab file and write its contents
    QTemporaryFile file;
    QVERIFY(file.open());

    const QString vocabTitle = QStringLiteral("vocabTitle");
    const QString k1 = QStringLiteral("k1");
    const QString k3 = QStringLiteral("k3");
    const QString k4 = QStringLiteral("k4");
    const QString v1 = QStringLiteral("v1");
    const QString v2 = QStringLiteral("v2");
    const QString v4 = QStringLiteral("v4");
    const QString v5 = QStringLiteral("v5");

    const QStringList lines = {
        vocabTitle,
        QString(),
        QStringLiteral("%1=%2").arg(k1, v1),
        QStringLiteral("=%1").arg(v2),
        QStringLiteral("%1=").arg(k3),
        QStringLiteral("="),
        QStringLiteral("%1=%2=%3").arg(k4, v4, v5)
    };
    const QByteArray content = lines.join('\n').toUtf8();

    QVERIFY(file.write(content) != -1);
    file.close();

    // Spy on signals emitted by VocabFileHandler
    QSignalSpy successSpy(m_handler.get(), &VocabFileHandler::readSucceeded);
    QSignalSpy partialSpy(m_handler.get(), &VocabFileHandler::readSucceededPartially);
    QSignalSpy failedSpy(m_handler.get(), &VocabFileHandler::readFailed);

    // Read test vocab file
    m_handler->read(file.fileName());

    // Read should succeed partially
    QCOMPARE(successSpy.count(), 0);
    QCOMPARE(partialSpy.count(), 1);
    QCOMPARE(failedSpy.count(), 0);

    // Parse vocab file data from signal
    const QList<QVariant> arguments = partialSpy.takeFirst();
    const VocabFileData data = qvariant_cast<VocabFileData>(arguments.at(0));
    int failedEntryCount = qvariant_cast<int>(arguments.at(1));

    // Validate vocab file data and failed entry count
    QCOMPARE(failedEntryCount, 4);
    QCOMPARE(data.title, vocabTitle);
    QCOMPARE(data.entries.size(), 1);
    QCOMPARE(data.groups.size(), 0);
    QCOMPARE(data.entries.at(0).first, k1);
    QCOMPARE(data.entries.at(0).second, v1);
}

// Rejects invalid entries in vocab file group
void TestVocabFileHandler::rejectsInvalidEntryInGroup()
{
    // Create temporary vocab file and write its contents
    QTemporaryFile file;
    QVERIFY(file.open());

    const QString vocabTitle = QStringLiteral("vocabTitle");
    const QString gTitle = QStringLiteral("gTitle");
    const QString gk1 = QStringLiteral("gk1");
    const QString gk3 = QStringLiteral("gk3");
    const QString gk4 = QStringLiteral("gk4");
    const QString gv1 = QStringLiteral("gv1");
    const QString gv2 = QStringLiteral("gv2");
    const QString gv4 = QStringLiteral("gv4");
    const QString gv5 = QStringLiteral("gv5");


    const QStringList lines = {
        vocabTitle,
        QString(),
        gTitle,
        QStringLiteral("%1=%2").arg(gk1, gv1),
        QStringLiteral("=%1").arg(gv2),
        QStringLiteral("%1=").arg(gk3),
        QStringLiteral("="),
        QStringLiteral("%1=%2=%3").arg(gk4, gv4, gv5)
    };
    const QByteArray content = lines.join('\n').toUtf8();

    QVERIFY(file.write(content) != -1);
    file.close();

    // Spy on signals emitted by VocabFileHandler
    QSignalSpy successSpy(m_handler.get(), &VocabFileHandler::readSucceeded);
    QSignalSpy partialSpy(m_handler.get(), &VocabFileHandler::readSucceededPartially);
    QSignalSpy failedSpy(m_handler.get(), &VocabFileHandler::readFailed);

    // Read test vocab file
    m_handler->read(file.fileName());

    // Read should succeed partially
    QCOMPARE(successSpy.count(), 0);
    QCOMPARE(partialSpy.count(), 1);
    QCOMPARE(failedSpy.count(), 0);

    // Parse vocab file data from signal
    const QList<QVariant> arguments = partialSpy.takeFirst();
    const VocabFileData data = qvariant_cast<VocabFileData>(arguments.at(0));
    int failedEntryCount = qvariant_cast<int>(arguments.at(1));

    // Validate vocab file data and failed entry count
    QCOMPARE(failedEntryCount, 4);
    QCOMPARE(data.title, vocabTitle);
    QCOMPARE(data.entries.size(), 0);
    QCOMPARE(data.groups.size(), 1);

    // Group
    const auto &g = data.groups.at(0);
    QCOMPARE(g.title, gTitle);
    QCOMPARE(g.entries.size(), 1);
    QCOMPARE(g.entries.at(0).first, gk1);
    QCOMPARE(g.entries.at(0).second, gv1);
}

void TestVocabFileHandler::failsWhenFileDoesNotExist()
{
    const QString faultyFileName = QStringLiteral("nofile.log");
    // Spy on signals emitted by VocabFileHandler
    QSignalSpy successSpy(m_handler.get(), &VocabFileHandler::readSucceeded);
    QSignalSpy partialSpy(m_handler.get(), &VocabFileHandler::readSucceededPartially);
    QSignalSpy failedSpy(m_handler.get(), &VocabFileHandler::readFailed);

    // Read non-existent vocab file
    m_handler->read(faultyFileName);

    // Read should fail
    QCOMPARE(successSpy.count(), 0);
    QCOMPARE(partialSpy.count(), 0);
    QCOMPARE(failedSpy.count(), 1);

    // Parse error code from signal
    const QList<QVariant> arguments = failedSpy.takeFirst();
    const ErrorCode code = qvariant_cast<ErrorCode>(arguments.at(0));

    // Validate error code
    QCOMPARE_NE(code, ErrorCode::Success);
    QCOMPARE(code, ErrorCode::FileOpenFailed);
}

void TestVocabFileHandler::failsWhenNoEntries()
{
    // Create temporary vocab file and write its contents
    QTemporaryFile file;
    QVERIFY(file.open());

    const QString vocabTitle = QStringLiteral("vocabTitle");
    const QString gTitle = QStringLiteral("gTitle");

    const QStringList lines = {
        vocabTitle,
        QString(),
        gTitle
    };
    const QByteArray content = lines.join('\n').toUtf8();

    QVERIFY(file.write(content) != -1);
    file.close();

    // Spy on signals emitted by VocabFileHandler
    QSignalSpy successSpy(m_handler.get(), &VocabFileHandler::readSucceeded);
    QSignalSpy partialSpy(m_handler.get(), &VocabFileHandler::readSucceededPartially);
    QSignalSpy failedSpy(m_handler.get(), &VocabFileHandler::readFailed);

    // Read test vocab file
    m_handler->read(file.fileName());

    // Read should fail
    QCOMPARE(successSpy.count(), 0);
    QCOMPARE(partialSpy.count(), 0);
    QCOMPARE(failedSpy.count(), 1);

    // Parse error code from signal
    const QList<QVariant> arguments = failedSpy.takeFirst();
    const ErrorCode code = qvariant_cast<ErrorCode>(arguments.at(0));

    // Validate error code
    QCOMPARE_NE(code, ErrorCode::Success);
    QCOMPARE(code, ErrorCode::VocabFileNoEntries);
}

void TestVocabFileHandler::failsWhenNoValidEntries()
{
    // Create temporary vocab file and write its contents
    QTemporaryFile file;
    QVERIFY(file.open());

    const QString vocabTitle = QStringLiteral("vocabTitle");
    const QString gTitle = QStringLiteral("gTitle");
    const QString gk1 = QStringLiteral("gk1");
    const QString gk3 = QStringLiteral("gk3");
    const QString gk4 = QStringLiteral("gk4");
    const QString gv1 = QStringLiteral("gv1");
    const QString gv2 = QStringLiteral("gv2");
    const QString gv4 = QStringLiteral("gv4");
    const QString gv5 = QStringLiteral("gv5");


    const QStringList lines = {
        vocabTitle,
        QString(),
        gTitle,
        QStringLiteral("%1==%2").arg(gk1, gv1),
        QStringLiteral("=%1").arg(gv2),
        QStringLiteral("%1=").arg(gk3),
        QStringLiteral("="),
        QStringLiteral("%1=%2=%3").arg(gk4, gv4, gv5)
    };
    const QByteArray content = lines.join('\n').toUtf8();

    QVERIFY(file.write(content) != -1);
    file.close();

    // Spy on signals emitted by VocabFileHandler
    QSignalSpy successSpy(m_handler.get(), &VocabFileHandler::readSucceeded);
    QSignalSpy partialSpy(m_handler.get(), &VocabFileHandler::readSucceededPartially);
    QSignalSpy failedSpy(m_handler.get(), &VocabFileHandler::readFailed);

    // Read test vocab file
    m_handler->read(file.fileName());

    // Read should fail
    QCOMPARE(successSpy.count(), 0);
    QCOMPARE(partialSpy.count(), 0);
    QCOMPARE(failedSpy.count(), 1);

    // Parse error code from signal
    const QList<QVariant> arguments = failedSpy.takeFirst();
    const ErrorCode code = qvariant_cast<ErrorCode>(arguments.at(0));

    // Validate error code
    QCOMPARE_NE(code, ErrorCode::Success);
    QCOMPARE(code, ErrorCode::VocabFileNoEntries);
}

QTEST_APPLESS_MAIN(TestVocabFileHandler)

#include "tst_vocabfilehandler.moc"
