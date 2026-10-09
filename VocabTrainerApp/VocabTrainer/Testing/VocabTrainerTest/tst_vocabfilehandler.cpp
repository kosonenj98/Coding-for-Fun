#include "Logging/logger.h"
#include "Application/vocabfilehandler.h"

#include <QtTest>

class TestVocabFileHandler : public QObject
{
    Q_OBJECT

private slots:
    void init();

    // Positive tests for single read
    void readsSimpleVocab();
    void readsVocabWithGroups();
    void readsVocabWithEntriesAndGroups();
    void readsVocabWithoutTitle();
    void readsTitleWithSpaces();
    void readsEntryWithSpaces();
    void readsComments();
    void readsEmptyLines();

    // Negative tests for single read
    void rejectsInvalidEntry();
    void rejectsInvalidEntryInGroup();
    void failsWhenFileDoesNotExist();
    void failsWhenNoEntries();
    void failsWhenNoValidEntries();

    // Positive tests for single info read
    void readsInfo();
    void readsInfoWithoutTitle();
    void readsInfoWithLeadingCommentsAndEmptyLines();

    // Negative tests for single info read
    void failsInfoWhenFileDoesNotExist();

    // Positive tests for multiple read
    void readsValidVocabs();
    void readsFaultyVocabs();
    void readsVocabsWithInvalidEntries();

    // Negative tests for multiple read
    void failsReadVocabsWhenNoVocabsCanBeRead();
    void failsReadWhenNoVocabFilesAreGiven();

    // Positive tests for multiple info read
    void readsVocabInfos();
    void readsMultipleFaultyVocabInfos();

    // Negative tests for multiple info read
    void failsReadInfosWhenNoVocabsCanBeRead();
    void failsReadInfosWhenNoVocabFilesAreGiven();

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
void TestVocabFileHandler::readsSimpleVocab()
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
    QSignalSpy readInfoFinishedSpy(m_handler.get(), &VocabFileHandler::readInfoFinished);
    QSignalSpy readInfosFinishedSpy(m_handler.get(), &VocabFileHandler::readInfosFinished);
    QSignalSpy readDataFinishedSpy(m_handler.get(), &VocabFileHandler::readDataFinished);
    QSignalSpy readDatasFinishedSpy(m_handler.get(), &VocabFileHandler::readDatasFinished);

    // Read test vocab file
    ReadVocabFileRequest request;
    request.filePath = file.fileName();
    request.mode = VocabFile::ReadMode::Data;
    m_handler->read(request);

    // Check signals
    QCOMPARE(readInfoFinishedSpy.count(), 0);
    QCOMPARE(readInfosFinishedSpy.count(), 0);
    QCOMPARE(readDataFinishedSpy.count(), 1);
    QCOMPARE(readDatasFinishedSpy.count(), 0);

    // Parse vocab file read result from signal
    const QList<QVariant> arguments = readDataFinishedSpy.takeFirst();
    const ReadVocabFileDataResult result = qvariant_cast<ReadVocabFileDataResult>(arguments.at(0));

    // Validate result
    QCOMPARE(result.code, ErrorCode::Success);
    QCOMPARE(result.failedEntryCount, 0);

    // Validate vocab file data
    VocabFile::Data data = result.data;
    QCOMPARE(data.info.filePath, request.filePath);
    QCOMPARE(data.info.vocabTitle, vocabTitle);
    QCOMPARE(data.entries.size(), 2);
    QCOMPARE(data.groups.size(), 0);
    QCOMPARE(data.entries.at(0).first, k1);
    QCOMPARE(data.entries.at(0).second, v1);
    QCOMPARE(data.entries.at(1).first, k2);
    QCOMPARE(data.entries.at(1).second, v2);
}

// Reads vocab file with title and two groups
void TestVocabFileHandler::readsVocabWithGroups()
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
    QSignalSpy readInfoFinishedSpy(m_handler.get(), &VocabFileHandler::readInfoFinished);
    QSignalSpy readInfosFinishedSpy(m_handler.get(), &VocabFileHandler::readInfosFinished);
    QSignalSpy readDataFinishedSpy(m_handler.get(), &VocabFileHandler::readDataFinished);
    QSignalSpy readDatasFinishedSpy(m_handler.get(), &VocabFileHandler::readDatasFinished);

    // Read test vocab file
    ReadVocabFileRequest request;
    request.filePath = file.fileName();
    request.mode = VocabFile::ReadMode::Data;
    m_handler->read(request);

    // Check signals
    QCOMPARE(readInfoFinishedSpy.count(), 0);
    QCOMPARE(readInfosFinishedSpy.count(), 0);
    QCOMPARE(readDataFinishedSpy.count(), 1);
    QCOMPARE(readDatasFinishedSpy.count(), 0);

    // Parse vocab file read result from signal
    const QList<QVariant> arguments = readDataFinishedSpy.takeFirst();
    const ReadVocabFileDataResult result = qvariant_cast<ReadVocabFileDataResult>(arguments.at(0));

    // Validate result
    QCOMPARE(result.code, ErrorCode::Success);
    QCOMPARE(result.failedEntryCount, 0);

    // Validate vocab file data
    VocabFile::Data data = result.data;
    QCOMPARE(data.info.filePath, request.filePath);
    QCOMPARE(data.info.vocabTitle, vocabTitle);
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
void TestVocabFileHandler::readsVocabWithEntriesAndGroups()
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
    QSignalSpy readInfoFinishedSpy(m_handler.get(), &VocabFileHandler::readInfoFinished);
    QSignalSpy readInfosFinishedSpy(m_handler.get(), &VocabFileHandler::readInfosFinished);
    QSignalSpy readDataFinishedSpy(m_handler.get(), &VocabFileHandler::readDataFinished);
    QSignalSpy readDatasFinishedSpy(m_handler.get(), &VocabFileHandler::readDatasFinished);

    // Read test vocab file
    ReadVocabFileRequest request;
    request.filePath = file.fileName();
    request.mode = VocabFile::ReadMode::Data;
    m_handler->read(request);

    // Check signals
    QCOMPARE(readInfoFinishedSpy.count(), 0);
    QCOMPARE(readInfosFinishedSpy.count(), 0);
    QCOMPARE(readDataFinishedSpy.count(), 1);
    QCOMPARE(readDatasFinishedSpy.count(), 0);

    // Parse vocab file read result from signal
    const QList<QVariant> arguments = readDataFinishedSpy.takeFirst();
    const ReadVocabFileDataResult result = qvariant_cast<ReadVocabFileDataResult>(arguments.at(0));

    // Validate result
    QCOMPARE(result.code, ErrorCode::Success);
    QCOMPARE(result.failedEntryCount, 0);

    // Validate vocab file data
    VocabFile::Data data = result.data;
    QCOMPARE(data.info.filePath, request.filePath);
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
void TestVocabFileHandler::readsVocabWithoutTitle()
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
    QSignalSpy readInfoFinishedSpy(m_handler.get(), &VocabFileHandler::readInfoFinished);
    QSignalSpy readInfosFinishedSpy(m_handler.get(), &VocabFileHandler::readInfosFinished);
    QSignalSpy readDataFinishedSpy(m_handler.get(), &VocabFileHandler::readDataFinished);
    QSignalSpy readDatasFinishedSpy(m_handler.get(), &VocabFileHandler::readDatasFinished);

    // Read test vocab files
    ReadVocabFileRequest request1;
    request1.filePath = file1.fileName();
    request1.mode = VocabFile::ReadMode::Data;
    ReadVocabFileRequest request2;
    request2.filePath = file2.fileName();
    request2.mode = VocabFile::ReadMode::Data;
    m_handler->read(request1);
    m_handler->read(request2);

    // Check signals
    QCOMPARE(readInfoFinishedSpy.count(), 0);
    QCOMPARE(readInfosFinishedSpy.count(), 0);
    QCOMPARE(readDataFinishedSpy.count(), 2);
    QCOMPARE(readDatasFinishedSpy.count(), 0);

    QList<VocabFile::Info> fileInfos = {{file1.fileName(), QFileInfo(file1).completeBaseName()},
                                        {file2.fileName(), QFileInfo(file2).completeBaseName()}};

    for (int i=0; i < readDataFinishedSpy.count(); i++)
    {
        // Parse vocab file read result from signal
        const QList<QVariant> arguments = readDataFinishedSpy.takeFirst();
        const ReadVocabFileDataResult result = qvariant_cast<ReadVocabFileDataResult>(arguments.at(0));

        // Validate result
        QCOMPARE(result.code, ErrorCode::Success);
        QCOMPARE(result.failedEntryCount, 0);

        // Validate vocab file data
        VocabFile::Data data = result.data;
        QCOMPARE(data.info.filePath, fileInfos.at(i).filePath);
        QCOMPARE(data.info.vocabTitle, fileInfos.at(i).vocabTitle);
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
    QSignalSpy readInfoFinishedSpy(m_handler.get(), &VocabFileHandler::readInfoFinished);
    QSignalSpy readInfosFinishedSpy(m_handler.get(), &VocabFileHandler::readInfosFinished);
    QSignalSpy readDataFinishedSpy(m_handler.get(), &VocabFileHandler::readDataFinished);
    QSignalSpy readDatasFinishedSpy(m_handler.get(), &VocabFileHandler::readDatasFinished);

    // Read test vocab file
    ReadVocabFileRequest request;
    request.filePath = file.fileName();
    request.mode = VocabFile::ReadMode::Data;
    m_handler->read(request);

    // Check signals
    QCOMPARE(readInfoFinishedSpy.count(), 0);
    QCOMPARE(readInfosFinishedSpy.count(), 0);
    QCOMPARE(readDataFinishedSpy.count(), 1);
    QCOMPARE(readDatasFinishedSpy.count(), 0);

    // Parse vocab file read result from signal
    const QList<QVariant> arguments = readDataFinishedSpy.takeFirst();
    const ReadVocabFileDataResult result = qvariant_cast<ReadVocabFileDataResult>(arguments.at(0));

    // Validate result
    QCOMPARE(result.code, ErrorCode::Success);
    QCOMPARE(result.failedEntryCount, 0);

    // Validate vocab file data
    VocabFile::Data data = result.data;
    QCOMPARE(data.info.filePath, request.filePath);
    QCOMPARE(data.info.vocabTitle, vocabTitle);
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
    QSignalSpy readInfoFinishedSpy(m_handler.get(), &VocabFileHandler::readInfoFinished);
    QSignalSpy readInfosFinishedSpy(m_handler.get(), &VocabFileHandler::readInfosFinished);
    QSignalSpy readDataFinishedSpy(m_handler.get(), &VocabFileHandler::readDataFinished);
    QSignalSpy readDatasFinishedSpy(m_handler.get(), &VocabFileHandler::readDatasFinished);

    // Read test vocab file
    ReadVocabFileRequest request;
    request.filePath = file.fileName();
    request.mode = VocabFile::ReadMode::Data;
    m_handler->read(request);

    // Check signals
    QCOMPARE(readInfoFinishedSpy.count(), 0);
    QCOMPARE(readInfosFinishedSpy.count(), 0);
    QCOMPARE(readDataFinishedSpy.count(), 1);
    QCOMPARE(readDatasFinishedSpy.count(), 0);

    // Parse vocab file read result from signal
    const QList<QVariant> arguments = readDataFinishedSpy.takeFirst();
    const ReadVocabFileDataResult result = qvariant_cast<ReadVocabFileDataResult>(arguments.at(0));

    // Validate result
    QCOMPARE(result.code, ErrorCode::Success);
    QCOMPARE(result.failedEntryCount, 0);

    // Validate vocab file data
    VocabFile::Data data = result.data;
    QCOMPARE(data.info.filePath, request.filePath);
    QCOMPARE(data.info.vocabTitle, vocabTitle);
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
    QSignalSpy readInfoFinishedSpy(m_handler.get(), &VocabFileHandler::readInfoFinished);
    QSignalSpy readInfosFinishedSpy(m_handler.get(), &VocabFileHandler::readInfosFinished);
    QSignalSpy readDataFinishedSpy(m_handler.get(), &VocabFileHandler::readDataFinished);
    QSignalSpy readDatasFinishedSpy(m_handler.get(), &VocabFileHandler::readDatasFinished);

    // Read test vocab file
    ReadVocabFileRequest request;
    request.filePath = file.fileName();
    request.mode = VocabFile::ReadMode::Data;
    m_handler->read(request);

    // Check signals
    QCOMPARE(readInfoFinishedSpy.count(), 0);
    QCOMPARE(readInfosFinishedSpy.count(), 0);
    QCOMPARE(readDataFinishedSpy.count(), 1);
    QCOMPARE(readDatasFinishedSpy.count(), 0);

    // Parse vocab file read result from signal
    const QList<QVariant> arguments = readDataFinishedSpy.takeFirst();
    const ReadVocabFileDataResult result = qvariant_cast<ReadVocabFileDataResult>(arguments.at(0));

    // Validate result
    QCOMPARE(result.code, ErrorCode::Success);
    QCOMPARE(result.failedEntryCount, 0);

    // Validate vocab file data
    VocabFile::Data data = result.data;
    QCOMPARE(data.info.filePath, request.filePath);
    QCOMPARE(data.info.vocabTitle, vocabTitle);
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
    QSignalSpy readInfoFinishedSpy(m_handler.get(), &VocabFileHandler::readInfoFinished);
    QSignalSpy readInfosFinishedSpy(m_handler.get(), &VocabFileHandler::readInfosFinished);
    QSignalSpy readDataFinishedSpy(m_handler.get(), &VocabFileHandler::readDataFinished);
    QSignalSpy readDatasFinishedSpy(m_handler.get(), &VocabFileHandler::readDatasFinished);

    // Read test vocab file
    ReadVocabFileRequest request;
    request.filePath = file.fileName();
    request.mode = VocabFile::ReadMode::Data;
    m_handler->read(request);

    // Check signals
    QCOMPARE(readInfoFinishedSpy.count(), 0);
    QCOMPARE(readInfosFinishedSpy.count(), 0);
    QCOMPARE(readDataFinishedSpy.count(), 1);
    QCOMPARE(readDatasFinishedSpy.count(), 0);

    // Parse vocab file read result from signal
    const QList<QVariant> arguments = readDataFinishedSpy.takeFirst();
    const ReadVocabFileDataResult result = qvariant_cast<ReadVocabFileDataResult>(arguments.at(0));

    // Validate result
    QCOMPARE(result.code, ErrorCode::Success);
    QCOMPARE(result.failedEntryCount, 0);

    // Validate vocab file data
    VocabFile::Data data = result.data;
    QCOMPARE(data.info.filePath, request.filePath);
    QCOMPARE(data.info.vocabTitle, vocabTitle);
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
    QSignalSpy readInfoFinishedSpy(m_handler.get(), &VocabFileHandler::readInfoFinished);
    QSignalSpy readInfosFinishedSpy(m_handler.get(), &VocabFileHandler::readInfosFinished);
    QSignalSpy readDataFinishedSpy(m_handler.get(), &VocabFileHandler::readDataFinished);
    QSignalSpy readDatasFinishedSpy(m_handler.get(), &VocabFileHandler::readDatasFinished);

    // Read test vocab file
    ReadVocabFileRequest request;
    request.filePath = file.fileName();
    request.mode = VocabFile::ReadMode::Data;
    m_handler->read(request);

    // Check signals
    QCOMPARE(readInfoFinishedSpy.count(), 0);
    QCOMPARE(readInfosFinishedSpy.count(), 0);
    QCOMPARE(readDataFinishedSpy.count(), 1);
    QCOMPARE(readDatasFinishedSpy.count(), 0);

    // Parse vocab file read result from signal
    const QList<QVariant> arguments = readDataFinishedSpy.takeFirst();
    const ReadVocabFileDataResult result = qvariant_cast<ReadVocabFileDataResult>(arguments.at(0));

    // Validate result
    QCOMPARE(result.code, ErrorCode::Success);
    QCOMPARE(result.failedEntryCount, 4);

    // Validate vocab file data
    VocabFile::Data data = result.data;
    QCOMPARE(data.info.filePath, request.filePath);
    QCOMPARE(data.info.vocabTitle, vocabTitle);
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
    QSignalSpy readInfoFinishedSpy(m_handler.get(), &VocabFileHandler::readInfoFinished);
    QSignalSpy readInfosFinishedSpy(m_handler.get(), &VocabFileHandler::readInfosFinished);
    QSignalSpy readDataFinishedSpy(m_handler.get(), &VocabFileHandler::readDataFinished);
    QSignalSpy readDatasFinishedSpy(m_handler.get(), &VocabFileHandler::readDatasFinished);

    // Read test vocab file
    ReadVocabFileRequest request;
    request.filePath = file.fileName();
    request.mode = VocabFile::ReadMode::Data;
    m_handler->read(request);

    // Check signals
    QCOMPARE(readInfoFinishedSpy.count(), 0);
    QCOMPARE(readInfosFinishedSpy.count(), 0);
    QCOMPARE(readDataFinishedSpy.count(), 1);
    QCOMPARE(readDatasFinishedSpy.count(), 0);

    // Parse vocab file read result from signal
    const QList<QVariant> arguments = readDataFinishedSpy.takeFirst();
    const ReadVocabFileDataResult result = qvariant_cast<ReadVocabFileDataResult>(arguments.at(0));

    // Validate result
    QCOMPARE(result.code, ErrorCode::Success);
    QCOMPARE(result.failedEntryCount, 4);

    // Validate vocab file data
    VocabFile::Data data = result.data;
    QCOMPARE(data.info.filePath, request.filePath);
    QCOMPARE(data.info.vocabTitle, vocabTitle);
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
    QSignalSpy readInfoFinishedSpy(m_handler.get(), &VocabFileHandler::readInfoFinished);
    QSignalSpy readInfosFinishedSpy(m_handler.get(), &VocabFileHandler::readInfosFinished);
    QSignalSpy readDataFinishedSpy(m_handler.get(), &VocabFileHandler::readDataFinished);
    QSignalSpy readDatasFinishedSpy(m_handler.get(), &VocabFileHandler::readDatasFinished);

    // Read test vocab file
    ReadVocabFileRequest request;
    request.filePath = faultyFileName;
    request.mode = VocabFile::ReadMode::Data;
    m_handler->read(request);

    // Check signals
    QCOMPARE(readInfoFinishedSpy.count(), 0);
    QCOMPARE(readInfosFinishedSpy.count(), 0);
    QCOMPARE(readDataFinishedSpy.count(), 1);
    QCOMPARE(readDatasFinishedSpy.count(), 0);

    // Parse vocab file read result from signal
    const QList<QVariant> arguments = readDataFinishedSpy.takeFirst();
    const ReadVocabFileDataResult result = qvariant_cast<ReadVocabFileDataResult>(arguments.at(0));

    // Validate result
    QCOMPARE_NE(result.code, ErrorCode::Success);
    QCOMPARE(result.code, ErrorCode::FileOpenFailed);
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
    QSignalSpy readInfoFinishedSpy(m_handler.get(), &VocabFileHandler::readInfoFinished);
    QSignalSpy readInfosFinishedSpy(m_handler.get(), &VocabFileHandler::readInfosFinished);
    QSignalSpy readDataFinishedSpy(m_handler.get(), &VocabFileHandler::readDataFinished);
    QSignalSpy readDatasFinishedSpy(m_handler.get(), &VocabFileHandler::readDatasFinished);

    // Read test vocab file
    ReadVocabFileRequest request;
    request.filePath = file.fileName();
    request.mode = VocabFile::ReadMode::Data;
    m_handler->read(request);

    // Check signals
    QCOMPARE(readInfoFinishedSpy.count(), 0);
    QCOMPARE(readInfosFinishedSpy.count(), 0);
    QCOMPARE(readDataFinishedSpy.count(), 1);
    QCOMPARE(readDatasFinishedSpy.count(), 0);

    // Parse vocab file read result from signal
    const QList<QVariant> arguments = readDataFinishedSpy.takeFirst();
    const ReadVocabFileDataResult result = qvariant_cast<ReadVocabFileDataResult>(arguments.at(0));

    // Validate result
    QCOMPARE_NE(result.code, ErrorCode::Success);
    QCOMPARE(result.code, ErrorCode::VocabFileNoEntries);
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
    QSignalSpy readInfoFinishedSpy(m_handler.get(), &VocabFileHandler::readInfoFinished);
    QSignalSpy readInfosFinishedSpy(m_handler.get(), &VocabFileHandler::readInfosFinished);
    QSignalSpy readDataFinishedSpy(m_handler.get(), &VocabFileHandler::readDataFinished);
    QSignalSpy readDatasFinishedSpy(m_handler.get(), &VocabFileHandler::readDatasFinished);

    // Read test vocab file
    ReadVocabFileRequest request;
    request.filePath = file.fileName();
    request.mode = VocabFile::ReadMode::Data;
    m_handler->read(request);

    // Check signals
    QCOMPARE(readInfoFinishedSpy.count(), 0);
    QCOMPARE(readInfosFinishedSpy.count(), 0);
    QCOMPARE(readDataFinishedSpy.count(), 1);
    QCOMPARE(readDatasFinishedSpy.count(), 0);

    // Parse vocab file read result from signal
    const QList<QVariant> arguments = readDataFinishedSpy.takeFirst();
    const ReadVocabFileDataResult result = qvariant_cast<ReadVocabFileDataResult>(arguments.at(0));

    // Validate result
    QCOMPARE_NE(result.code, ErrorCode::Success);
    QCOMPARE(result.code, ErrorCode::VocabFileNoEntries);
}

// Reads vocabulary info from file with title
void TestVocabFileHandler::readsInfo()
{
    QTemporaryFile file;
    QVERIFY(file.open());

    const QString vocabTitle = QStringLiteral("vocabTitle");
    const QString k1 = QStringLiteral("k1");
    const QString v1 = QStringLiteral("v1");

    const QStringList lines = {
        vocabTitle,
        QString(),
        QStringLiteral("%1=%2").arg(k1, v1)
    };

    const QByteArray content = lines.join('\n').toUtf8();

    QVERIFY(file.write(content) != -1);
    file.close();

    // Spy on signals emitted by VocabFileHandler
    QSignalSpy readInfoFinishedSpy(m_handler.get(), &VocabFileHandler::readInfoFinished);
    QSignalSpy readInfosFinishedSpy(m_handler.get(), &VocabFileHandler::readInfosFinished);
    QSignalSpy readDataFinishedSpy(m_handler.get(), &VocabFileHandler::readDataFinished);
    QSignalSpy readDatasFinishedSpy(m_handler.get(), &VocabFileHandler::readDatasFinished);

    // Read test vocab file
    ReadVocabFileRequest request;
    request.filePath = file.fileName();
    request.mode = VocabFile::ReadMode::Info;
    m_handler->read(request);

    // Check signals
    QCOMPARE(readInfoFinishedSpy.count(), 1);
    QCOMPARE(readInfosFinishedSpy.count(), 0);
    QCOMPARE(readDataFinishedSpy.count(), 0);
    QCOMPARE(readDatasFinishedSpy.count(), 0);

    // Parse vocab file read result from signal
    const QList<QVariant> arguments = readInfoFinishedSpy.takeFirst();
    const ReadVocabFileInfoResult result = qvariant_cast<ReadVocabFileInfoResult>(arguments.at(0));

    // Validate result
    QCOMPARE(result.code, ErrorCode::Success);

    // Validate vocab file info
    VocabFile::Info info = result.info;
    QCOMPARE(info.filePath, file.fileName());
    QCOMPARE(info.vocabTitle, vocabTitle);
}

// Reads vocab info without explicit title
void TestVocabFileHandler::readsInfoWithoutTitle()
{
    QTemporaryFile file;
    QVERIFY(file.open());

    const QString k1 = QStringLiteral("k1");
    const QString v1 = QStringLiteral("v1");
    const QStringList lines = {
        QStringLiteral("%1=%2").arg(k1, v1)
    };
    const QByteArray content = lines.join('\n').toUtf8();

    QVERIFY(file.write(content) != -1);
    file.close();

    const QString expectedTitle = QFileInfo(file.fileName()).completeBaseName();

    // Spy on signals emitted by VocabFileHandler
    QSignalSpy readInfoFinishedSpy(m_handler.get(), &VocabFileHandler::readInfoFinished);
    QSignalSpy readInfosFinishedSpy(m_handler.get(), &VocabFileHandler::readInfosFinished);
    QSignalSpy readDataFinishedSpy(m_handler.get(), &VocabFileHandler::readDataFinished);
    QSignalSpy readDatasFinishedSpy(m_handler.get(), &VocabFileHandler::readDatasFinished);

    // Read test vocab file
    ReadVocabFileRequest request;
    request.filePath = file.fileName();
    request.mode = VocabFile::ReadMode::Info;
    m_handler->read(request);

    // Check signals
    QCOMPARE(readInfoFinishedSpy.count(), 1);
    QCOMPARE(readInfosFinishedSpy.count(), 0);
    QCOMPARE(readDataFinishedSpy.count(), 0);
    QCOMPARE(readDatasFinishedSpy.count(), 0);

    // Parse vocab file read result from signal
    const QList<QVariant> arguments = readInfoFinishedSpy.takeFirst();
    const ReadVocabFileInfoResult result = qvariant_cast<ReadVocabFileInfoResult>(arguments.at(0));

    // Validate result
    QCOMPARE(result.code, ErrorCode::Success);

    // Validate vocab file info
    VocabFile::Info info = result.info;
    QCOMPARE(info.filePath, file.fileName());
    QCOMPARE(info.vocabTitle, expectedTitle);
}

// Reads vocab info when empty lines and comments are present
void TestVocabFileHandler::readsInfoWithLeadingCommentsAndEmptyLines()
{
    QTemporaryFile file;
    QVERIFY(file.open());

    const QString vocabTitle = QStringLiteral("vocabTitle");

    const QString k1 = QStringLiteral("k1");
    const QString v1 = QStringLiteral("v1");
    const QStringList lines = {
        QStringLiteral("# comment"),
        QString(),
        QStringLiteral("   "),
        QStringLiteral("### another comment"),
        vocabTitle,
        QStringLiteral("%1=%2").arg(k1, v1)
    };
    const QByteArray content = lines.join('\n').toUtf8();

    QVERIFY(file.write(content) != -1);
    file.close();

    // Spy on signals emitted by VocabFileHandler
    QSignalSpy readInfoFinishedSpy(m_handler.get(), &VocabFileHandler::readInfoFinished);
    QSignalSpy readInfosFinishedSpy(m_handler.get(), &VocabFileHandler::readInfosFinished);
    QSignalSpy readDataFinishedSpy(m_handler.get(), &VocabFileHandler::readDataFinished);
    QSignalSpy readDatasFinishedSpy(m_handler.get(), &VocabFileHandler::readDatasFinished);

    // Read test vocab file
    ReadVocabFileRequest request;
    request.filePath = file.fileName();
    request.mode = VocabFile::ReadMode::Info;
    m_handler->read(request);

    // Check signals
    QCOMPARE(readInfoFinishedSpy.count(), 1);
    QCOMPARE(readInfosFinishedSpy.count(), 0);
    QCOMPARE(readDataFinishedSpy.count(), 0);
    QCOMPARE(readDatasFinishedSpy.count(), 0);

    // Parse vocab file read result from signal
    const QList<QVariant> arguments = readInfoFinishedSpy.takeFirst();
    const ReadVocabFileInfoResult result = qvariant_cast<ReadVocabFileInfoResult>(arguments.at(0));

    // Validate result
    QCOMPARE(result.code, ErrorCode::Success);

    // Validate vocab file info
    VocabFile::Info info = result.info;
    QCOMPARE(info.vocabTitle, vocabTitle);
}

// Fails to read non-existent file
void TestVocabFileHandler::failsInfoWhenFileDoesNotExist()
{
    const QString filePath = QStringLiteral("nofile.vocab");

    // Spy on signals emitted by VocabFileHandler
    QSignalSpy readInfoFinishedSpy(m_handler.get(), &VocabFileHandler::readInfoFinished);
    QSignalSpy readInfosFinishedSpy(m_handler.get(), &VocabFileHandler::readInfosFinished);
    QSignalSpy readDataFinishedSpy(m_handler.get(), &VocabFileHandler::readDataFinished);
    QSignalSpy readDatasFinishedSpy(m_handler.get(), &VocabFileHandler::readDatasFinished);

    // Read test vocab file
    ReadVocabFileRequest request;
    request.filePath = filePath;
    request.mode = VocabFile::ReadMode::Info;
    m_handler->read(request);

    // Check signals
    QCOMPARE(readInfoFinishedSpy.count(), 1);
    QCOMPARE(readInfosFinishedSpy.count(), 0);
    QCOMPARE(readDataFinishedSpy.count(), 0);
    QCOMPARE(readDatasFinishedSpy.count(), 0);

    // Parse vocab file read result from signal
    const QList<QVariant> arguments = readInfoFinishedSpy.takeFirst();
    const ReadVocabFileInfoResult result = qvariant_cast<ReadVocabFileInfoResult>(arguments.at(0));

    // Validate result
    QCOMPARE_NE(result.code, ErrorCode::Success);
    QCOMPARE(result.code, ErrorCode::FileOpenFailed);
}

// Reads multiple valid vocab files
void TestVocabFileHandler::readsValidVocabs()
{
    QTemporaryFile file1;
    QVERIFY(file1.open());

    const QString f1vocabTitle = QStringLiteral("f1vocabTitle");
    const QString f1k1 = QStringLiteral("f1k1");
    const QString f1k2 = QStringLiteral("f1k2");
    const QString f1v1 = QStringLiteral("f1v1");
    const QString f1v2 = QStringLiteral("f1v2");
    const QStringList f1lines = {
        f1vocabTitle,
        QString(),
        QStringLiteral("%1=%2").arg(f1k1, f1v1),
        QStringLiteral("%1=%2").arg(f1k2, f1v2),
    };
    const QByteArray f1content = f1lines.join('\n').toUtf8();

    QVERIFY(file1.write(f1content) != -1);
    file1.close();

    QTemporaryFile file2;
    QVERIFY(file2.open());

    const QString f2vocabTitle = QStringLiteral("f2vocabTitle");
    const QString f2k1 = QStringLiteral("f2k1");
    const QString f2k2 = QStringLiteral("f2k2");
    const QString f2v1 = QStringLiteral("f2v1");
    const QString f2v2 = QStringLiteral("f2v2");
    const QStringList f2lines = {
        f2vocabTitle,
        QString(),
        QStringLiteral("%1=%2").arg(f2k1, f2v1),
        QStringLiteral("%1=%2").arg(f2k2, f2v2),
    };
    const QByteArray f2content = f2lines.join('\n').toUtf8();

    QVERIFY(file2.write(f2content) != -1);
    file2.close();

    // Spy on signals emitted by VocabFileHandler
    QSignalSpy readInfoFinishedSpy(m_handler.get(), &VocabFileHandler::readInfoFinished);
    QSignalSpy readInfosFinishedSpy(m_handler.get(), &VocabFileHandler::readInfosFinished);
    QSignalSpy readDataFinishedSpy(m_handler.get(), &VocabFileHandler::readDataFinished);
    QSignalSpy readDatasFinishedSpy(m_handler.get(), &VocabFileHandler::readDatasFinished);

    const QStringList filePaths = {
        file1.fileName(),
        file2.fileName()
    };

    // Read test vocab file
    ReadVocabFilesRequest request;
    request.filePaths = filePaths;
    request.mode = VocabFile::ReadMode::Data;
    m_handler->readMultiple(request);

    // Check signals
    QCOMPARE(readInfoFinishedSpy.count(), 0);
    QCOMPARE(readInfosFinishedSpy.count(), 0);
    QCOMPARE(readDataFinishedSpy.count(), 0);
    QCOMPARE(readDatasFinishedSpy.count(), 1);

    // Parse vocab file read result from signal
    const QList<QVariant> arguments = readDatasFinishedSpy.takeFirst();
    const ReadVocabFileDatasResult result = qvariant_cast<ReadVocabFileDatasResult>(arguments.at(0));

    // Validate result
    QCOMPARE(result.code, ErrorCode::Success);
    QCOMPARE(result.failedVocabCount, 0);
    QCOMPARE(result.partiallySucceededVocabCount, 0);
    QCOMPARE(result.failedEntryCount, 0);

    // Validate vocab file datas
    QList<VocabFile::Data> datas = result.datas;
    QCOMPARE(datas.size(), 2);

    VocabFile::Data data1 = datas.at(0);
    QCOMPARE(data1.info.filePath, file1.fileName());
    QCOMPARE(data1.info.vocabTitle, f1vocabTitle);
    QCOMPARE(data1.entries.size(), 2);
    QCOMPARE(data1.groups.size(), 0);
    QCOMPARE(data1.entries.at(0).first, f1k1);
    QCOMPARE(data1.entries.at(0).second, f1v1);
    QCOMPARE(data1.entries.at(1).first, f1k2);
    QCOMPARE(data1.entries.at(1).second, f1v2);

    VocabFile::Data data2 = datas.at(1);
    QCOMPARE(data2.info.filePath, file2.fileName());
    QCOMPARE(data2.info.vocabTitle, f2vocabTitle);
    QCOMPARE(data2.entries.size(), 2);
    QCOMPARE(data2.groups.size(), 0);
    QCOMPARE(data2.entries.at(0).first, f2k1);
    QCOMPARE(data2.entries.at(0).second, f2v1);
    QCOMPARE(data2.entries.at(1).first, f2k2);
    QCOMPARE(data2.entries.at(1).second, f2v2);
}

// Fails when no vocab is valid
void TestVocabFileHandler::readsFaultyVocabs()
{
    const QString f1FileName = QStringLiteral("nofile.log");

    // Create temporary vocab file and write its contents
    QTemporaryFile file2;
    QVERIFY(file2.open());

    const QString f2vocabTitle = QStringLiteral("f2vocabTitle");
    const QString f2gTitle = QStringLiteral("f2gTitle");

    const QStringList f2lines = {
        f2vocabTitle,
        QString(),
        f2gTitle
    };
    const QByteArray f2content = f2lines.join('\n').toUtf8();

    QVERIFY(file2.write(f2content) != -1);
    file2.close();

    // Create temporary vocab file and write its contents
    QTemporaryFile file3;
    QVERIFY(file3.open());

    const QString f3vocabTitle = QStringLiteral("f3vocabTitle");
    const QString f3gTitle = QStringLiteral("f3gTitle");
    const QString f3gk1 = QStringLiteral("f3gk1");
    const QString f3gk3 = QStringLiteral("f3gk3");
    const QString f3gk4 = QStringLiteral("f3gk4");
    const QString f3gv1 = QStringLiteral("f3gv1");
    const QString f3gv2 = QStringLiteral("f3gv2");
    const QString f3gv4 = QStringLiteral("f3gv4");
    const QString f3gv5 = QStringLiteral("f3gv5");


    const QStringList f3lines = {
        f3vocabTitle,
        QString(),
        f3gTitle,
        QStringLiteral("%1==%2").arg(f3gk1, f3gv1),
        QStringLiteral("=%1").arg(f3gv2),
        QStringLiteral("%1=").arg(f3gk3),
        QStringLiteral("="),
        QStringLiteral("%1=%2=%3").arg(f3gk4, f3gv4, f3gv5)
    };
    const QByteArray f3content = f3lines.join('\n').toUtf8();

    QVERIFY(file3.write(f3content) != -1);
    file3.close();

    // Spy on signals emitted by VocabFileHandler
    QSignalSpy readInfoFinishedSpy(m_handler.get(), &VocabFileHandler::readInfoFinished);
    QSignalSpy readInfosFinishedSpy(m_handler.get(), &VocabFileHandler::readInfosFinished);
    QSignalSpy readDataFinishedSpy(m_handler.get(), &VocabFileHandler::readDataFinished);
    QSignalSpy readDatasFinishedSpy(m_handler.get(), &VocabFileHandler::readDatasFinished);

    const QStringList filePaths = {
        f1FileName,
        file2.fileName(),
        file3.fileName()
    };

    // Read test vocab file
    ReadVocabFilesRequest request;
    request.filePaths = filePaths;
    request.mode = VocabFile::ReadMode::Data;
    m_handler->readMultiple(request);

    // Check signals
    QCOMPARE(readInfoFinishedSpy.count(), 0);
    QCOMPARE(readInfosFinishedSpy.count(), 0);
    QCOMPARE(readDataFinishedSpy.count(), 0);
    QCOMPARE(readDatasFinishedSpy.count(), 1);

    // Parse vocab file read result from signal
    const QList<QVariant> arguments = readDatasFinishedSpy.takeFirst();
    const ReadVocabFileDatasResult result = qvariant_cast<ReadVocabFileDatasResult>(arguments.at(0));

    // Validate result
    QCOMPARE_NE(result.code, ErrorCode::Success);
    QCOMPARE(result.code, ErrorCode::NoSucceededVocabs);
    QCOMPARE(result.failedVocabCount, filePaths.count());
    QCOMPARE(result.partiallySucceededVocabCount, 0);
    QCOMPARE(result.failedEntryCount, 0);
}

// Reads vocabs with invalid entries
void TestVocabFileHandler::readsVocabsWithInvalidEntries()
{
    // Create temporary vocab file and write its contents
    QTemporaryFile file1;
    QVERIFY(file1.open());

    const QString f1vocabTitle = QStringLiteral("f1vocabTitle");
    const QString f1k1 = QStringLiteral("f1k1");
    const QString f1k3 = QStringLiteral("f1k3");
    const QString f1k4 = QStringLiteral("f1k4");
    const QString f1v1 = QStringLiteral("f1v1");
    const QString f1v2 = QStringLiteral("f1v2");
    const QString f1v4 = QStringLiteral("f1v4");
    const QString f1v5 = QStringLiteral("f1v5");

    const QStringList f1lines = {
        f1vocabTitle,
        QString(),
        QStringLiteral("%1=%2").arg(f1k1, f1v1),
        QStringLiteral("=%1").arg(f1v2),
        QStringLiteral("%1=").arg(f1k3),
        QStringLiteral("="),
        QStringLiteral("%1=%2=%3").arg(f1k4, f1v4, f1v5)
    };
    const QByteArray f1content = f1lines.join('\n').toUtf8();

    QVERIFY(file1.write(f1content) != -1);
    file1.close();

    // Create temporary vocab file and write its contents
    QTemporaryFile file2;
    QVERIFY(file2.open());

    const QString f2vocabTitle = QStringLiteral("f2vocabTitle");
    const QString f2gTitle = QStringLiteral("f2gTitle");
    const QString f2gk1 = QStringLiteral("f2gk1");
    const QString f2gk3 = QStringLiteral("f2gk3");
    const QString f2gk4 = QStringLiteral("f2gk4");
    const QString f2gv1 = QStringLiteral("f2gv1");
    const QString f2gv2 = QStringLiteral("f2gv2");
    const QString f2gv4 = QStringLiteral("f2gv4");
    const QString f2gv5 = QStringLiteral("f2gv5");


    const QStringList f2lines = {
        f2vocabTitle,
        QString(),
        f2gTitle,
        QStringLiteral("%1=%2").arg(f2gk1, f2gv1),
        QStringLiteral("=%1").arg(f2gv2),
        QStringLiteral("%1=").arg(f2gk3),
        QStringLiteral("="),
        QStringLiteral("%1=%2=%3").arg(f2gk4, f2gv4, f2gv5)
    };
    const QByteArray f2content = f2lines.join('\n').toUtf8();

    QVERIFY(file2.write(f2content) != -1);
    file2.close();

    // Spy on signals emitted by VocabFileHandler
    QSignalSpy readInfoFinishedSpy(m_handler.get(), &VocabFileHandler::readInfoFinished);
    QSignalSpy readInfosFinishedSpy(m_handler.get(), &VocabFileHandler::readInfosFinished);
    QSignalSpy readDataFinishedSpy(m_handler.get(), &VocabFileHandler::readDataFinished);
    QSignalSpy readDatasFinishedSpy(m_handler.get(), &VocabFileHandler::readDatasFinished);

    const QStringList filePaths = {
        file1.fileName(),
        file2.fileName()
    };

    // Read test vocab file
    ReadVocabFilesRequest request;
    request.filePaths = filePaths;
    request.mode = VocabFile::ReadMode::Data;
    m_handler->readMultiple(request);

    // Check signals
    QCOMPARE(readInfoFinishedSpy.count(), 0);
    QCOMPARE(readInfosFinishedSpy.count(), 0);
    QCOMPARE(readDataFinishedSpy.count(), 0);
    QCOMPARE(readDatasFinishedSpy.count(), 1);

    // Parse vocab file read result from signal
    const QList<QVariant> arguments = readDatasFinishedSpy.takeFirst();
    const ReadVocabFileDatasResult result = qvariant_cast<ReadVocabFileDatasResult>(arguments.at(0));

    // Validate result
    QCOMPARE(result.code, ErrorCode::Success);
    QCOMPARE(result.failedVocabCount, 0);
    QCOMPARE(result.partiallySucceededVocabCount, 2);
    QCOMPARE(result.failedEntryCount, 8);

    // Validate vocab file datas
    QList<VocabFile::Data> datas = result.datas;
    QCOMPARE(datas.size(), 2);

    VocabFile::Data data1 = datas.at(0);
    QCOMPARE(data1.info.filePath, file1.fileName());
    QCOMPARE(data1.info.vocabTitle, f1vocabTitle);
    QCOMPARE(data1.entries.size(), 1);
    QCOMPARE(data1.groups.size(), 0);
    QCOMPARE(data1.entries.at(0).first, f1k1);
    QCOMPARE(data1.entries.at(0).second, f1v1);

    VocabFile::Data data2 = datas.at(1);
    QCOMPARE(data2.info.filePath, file2.fileName());
    QCOMPARE(data2.info.vocabTitle, f2vocabTitle);
    QCOMPARE(data2.entries.size(), 0);
    QCOMPARE(data2.groups.size(), 1);
    VocabFile::Group group = data2.groups.at(0);
    QCOMPARE(group.entries.size(), 1);
    QCOMPARE(group.entries.at(0).first, f2gk1);
    QCOMPARE(group.entries.at(0).second, f2gv1);
}

void TestVocabFileHandler::failsReadVocabsWhenNoVocabsCanBeRead()
{
    const QStringList filePaths = {
        QStringLiteral("missing1.vocab"),
        QStringLiteral("missing2.vocab")
    };

    // Spy on signals emitted by VocabFileHandler
    QSignalSpy readInfoFinishedSpy(m_handler.get(), &VocabFileHandler::readInfoFinished);
    QSignalSpy readInfosFinishedSpy(m_handler.get(), &VocabFileHandler::readInfosFinished);
    QSignalSpy readDataFinishedSpy(m_handler.get(), &VocabFileHandler::readDataFinished);
    QSignalSpy readDatasFinishedSpy(m_handler.get(), &VocabFileHandler::readDatasFinished);

    // Read test vocab file
    ReadVocabFilesRequest request;
    request.filePaths = filePaths;
    request.mode = VocabFile::ReadMode::Data;
    m_handler->readMultiple(request);

    // Check signals
    QCOMPARE(readInfoFinishedSpy.count(), 0);
    QCOMPARE(readInfosFinishedSpy.count(), 0);
    QCOMPARE(readDataFinishedSpy.count(), 0);
    QCOMPARE(readDatasFinishedSpy.count(), 1);

    // Parse vocab file read result from signal
    const QList<QVariant> arguments = readDatasFinishedSpy.takeFirst();
    const ReadVocabFileDatasResult result = qvariant_cast<ReadVocabFileDatasResult>(arguments.at(0));

    // Validate result
    QCOMPARE_NE(result.code, ErrorCode::Success);
    QCOMPARE(result.code, ErrorCode::NoSucceededVocabs);
    QCOMPARE(result.failedVocabCount, filePaths.count());
}

void TestVocabFileHandler::failsReadWhenNoVocabFilesAreGiven()
{
    const QStringList filePaths;

    // Spy on signals emitted by VocabFileHandler
    QSignalSpy readInfoFinishedSpy(m_handler.get(), &VocabFileHandler::readInfoFinished);
    QSignalSpy readInfosFinishedSpy(m_handler.get(), &VocabFileHandler::readInfosFinished);
    QSignalSpy readDataFinishedSpy(m_handler.get(), &VocabFileHandler::readDataFinished);
    QSignalSpy readDatasFinishedSpy(m_handler.get(), &VocabFileHandler::readDatasFinished);

    // Read test vocab file
    ReadVocabFilesRequest request;
    request.filePaths = filePaths;
    request.mode = VocabFile::ReadMode::Data;
    m_handler->readMultiple(request);

    // Check signals
    QCOMPARE(readInfoFinishedSpy.count(), 0);
    QCOMPARE(readInfosFinishedSpy.count(), 0);
    QCOMPARE(readDataFinishedSpy.count(), 0);
    QCOMPARE(readDatasFinishedSpy.count(), 1);

    // Parse vocab file read result from signal
    const QList<QVariant> arguments = readDatasFinishedSpy.takeFirst();
    const ReadVocabFileDatasResult result = qvariant_cast<ReadVocabFileDatasResult>(arguments.at(0));

    // Validate result
    QCOMPARE_NE(result.code, ErrorCode::Success);
    QCOMPARE(result.code, ErrorCode::NoVocabFilesGiven);
}

void TestVocabFileHandler::readsVocabInfos()
{
    QTemporaryFile file1;
    QVERIFY(file1.open());

    const QString f1vocabTitle = QStringLiteral("f1vocabTitle");
    const QString f1k1 = QStringLiteral("f1k1");
    const QString f1v1 = QStringLiteral("f1v1");

    const QStringList f1lines = {
        f1vocabTitle,
        QString(),
        QStringLiteral("%1=%2").arg(f1k1, f1v1)
    };

    const QByteArray f1content = f1lines.join('\n').toUtf8();

    QVERIFY(file1.write(f1content) != -1);
    file1.close();

    QTemporaryFile file2;
    QVERIFY(file2.open());

    const QString f2vocabTitle = QStringLiteral("f2vocabTitle");
    const QString f2k1 = QStringLiteral("f2k1");
    const QString f2v1 = QStringLiteral("f2v1");

    const QStringList f2lines = {
        f2vocabTitle,
        QString(),
        QStringLiteral("%1=%2").arg(f2k1, f2v1)
    };

    const QByteArray f2content = f2lines.join('\n').toUtf8();

    QVERIFY(file2.write(f2content) != -1);
    file2.close();

    // Spy on signals emitted by VocabFileHandler
    QSignalSpy readInfoFinishedSpy(m_handler.get(), &VocabFileHandler::readInfoFinished);
    QSignalSpy readInfosFinishedSpy(m_handler.get(), &VocabFileHandler::readInfosFinished);
    QSignalSpy readDataFinishedSpy(m_handler.get(), &VocabFileHandler::readDataFinished);
    QSignalSpy readDatasFinishedSpy(m_handler.get(), &VocabFileHandler::readDatasFinished);

    const QStringList filePaths = {
        file1.fileName(),
        file2.fileName()
    };

    // Read test vocab file
    ReadVocabFilesRequest request;
    request.filePaths = filePaths;
    request.mode = VocabFile::ReadMode::Info;
    m_handler->readMultiple(request);

    // Check signals
    QCOMPARE(readInfoFinishedSpy.count(), 0);
    QCOMPARE(readInfosFinishedSpy.count(), 1);
    QCOMPARE(readDataFinishedSpy.count(), 0);
    QCOMPARE(readDatasFinishedSpy.count(), 0);

    // Parse vocab file read result from signal
    const QList<QVariant> arguments = readInfosFinishedSpy.takeFirst();
    const ReadVocabFileInfosResult result = qvariant_cast<ReadVocabFileInfosResult>(arguments.at(0));

    // Validate result
    QCOMPARE(result.code, ErrorCode::Success);
    QCOMPARE(result.failedVocabCount, 0);
    QCOMPARE(result.infos.count(), filePaths.count());

    // Validate vocab file info
    QList<VocabFile::Info> infos = result.infos;
    QCOMPARE(infos.at(0).filePath, file1.fileName());
    QCOMPARE(infos.at(0).vocabTitle, f1vocabTitle);
    QCOMPARE(infos.at(1).filePath, file2.fileName());
    QCOMPARE(infos.at(1).vocabTitle, f2vocabTitle);
}

void TestVocabFileHandler::readsMultipleFaultyVocabInfos()
{
    QTemporaryFile file1;
    QVERIFY(file1.open());

    const QString f1vocabTitle = QStringLiteral("f1vocabTitle");
    const QString f1k1 = QStringLiteral("f1k1");
    const QString f1v1 = QStringLiteral("f1v1");

    const QStringList f1lines = {
        f1vocabTitle,
        QString(),
        QStringLiteral("%1=%2").arg(f1k1, f1v1)
    };

    const QByteArray f1content = f1lines.join('\n').toUtf8();

    QVERIFY(file1.write(f1content) != -1);
    file1.close();

    const QString invalidFileName = QStringLiteral("missing.vocab");

    // Spy on signals emitted by VocabFileHandler
    QSignalSpy readInfoFinishedSpy(m_handler.get(), &VocabFileHandler::readInfoFinished);
    QSignalSpy readInfosFinishedSpy(m_handler.get(), &VocabFileHandler::readInfosFinished);
    QSignalSpy readDataFinishedSpy(m_handler.get(), &VocabFileHandler::readDataFinished);
    QSignalSpy readDatasFinishedSpy(m_handler.get(), &VocabFileHandler::readDatasFinished);

    const QStringList filePaths = {
        file1.fileName(),
        invalidFileName
    };

    // Read test vocab file
    ReadVocabFilesRequest request;
    request.filePaths = filePaths;
    request.mode = VocabFile::ReadMode::Info;
    m_handler->readMultiple(request);

    // Check signals
    QCOMPARE(readInfoFinishedSpy.count(), 0);
    QCOMPARE(readInfosFinishedSpy.count(), 1);
    QCOMPARE(readDataFinishedSpy.count(), 0);
    QCOMPARE(readDatasFinishedSpy.count(), 0);

    // Parse vocab file read result from signal
    const QList<QVariant> arguments = readInfosFinishedSpy.takeFirst();
    const ReadVocabFileInfosResult result = qvariant_cast<ReadVocabFileInfosResult>(arguments.at(0));

    // Validate result
    QCOMPARE(result.code, ErrorCode::Success);
    QCOMPARE(result.failedVocabCount, 1);
    QCOMPARE(result.infos.count(), 1);

    // Validate vocab file info
    QList<VocabFile::Info> infos = result.infos;
    QCOMPARE(infos.at(0).filePath, file1.fileName());
    QCOMPARE(infos.at(0).vocabTitle, f1vocabTitle);
}

void TestVocabFileHandler::failsReadInfosWhenNoVocabsCanBeRead()
{
    // Spy on signals emitted by VocabFileHandler
    QSignalSpy readInfoFinishedSpy(m_handler.get(), &VocabFileHandler::readInfoFinished);
    QSignalSpy readInfosFinishedSpy(m_handler.get(), &VocabFileHandler::readInfosFinished);
    QSignalSpy readDataFinishedSpy(m_handler.get(), &VocabFileHandler::readDataFinished);
    QSignalSpy readDatasFinishedSpy(m_handler.get(), &VocabFileHandler::readDatasFinished);

    const QStringList filePaths = {
        QStringLiteral("Nofile1.vocab"),
        QStringLiteral("Nofile2.vocab")
    };

    // Read test vocab file
    ReadVocabFilesRequest request;
    request.filePaths = filePaths;
    request.mode = VocabFile::ReadMode::Info;
    m_handler->readMultiple(request);

    // Check signals
    QCOMPARE(readInfoFinishedSpy.count(), 0);
    QCOMPARE(readInfosFinishedSpy.count(), 1);
    QCOMPARE(readDataFinishedSpy.count(), 0);
    QCOMPARE(readDatasFinishedSpy.count(), 0);

    // Parse vocab file read result from signal
    const QList<QVariant> arguments = readInfosFinishedSpy.takeFirst();
    const ReadVocabFileInfosResult result = qvariant_cast<ReadVocabFileInfosResult>(arguments.at(0));

    // Validate result
    QCOMPARE_NE(result.code, ErrorCode::Success);
    QCOMPARE(result.code, ErrorCode::NoSucceededVocabs);
}

void TestVocabFileHandler::failsReadInfosWhenNoVocabFilesAreGiven()
{
    // Spy on signals emitted by VocabFileHandler
    QSignalSpy readInfoFinishedSpy(m_handler.get(), &VocabFileHandler::readInfoFinished);
    QSignalSpy readInfosFinishedSpy(m_handler.get(), &VocabFileHandler::readInfosFinished);
    QSignalSpy readDataFinishedSpy(m_handler.get(), &VocabFileHandler::readDataFinished);
    QSignalSpy readDatasFinishedSpy(m_handler.get(), &VocabFileHandler::readDatasFinished);

    const QStringList filePaths = {};

    // Read test vocab file
    ReadVocabFilesRequest request;
    request.filePaths = filePaths;
    request.mode = VocabFile::ReadMode::Info;
    m_handler->readMultiple(request);

    // Check signals
    QCOMPARE(readInfoFinishedSpy.count(), 0);
    QCOMPARE(readInfosFinishedSpy.count(), 1);
    QCOMPARE(readDataFinishedSpy.count(), 0);
    QCOMPARE(readDatasFinishedSpy.count(), 0);

    // Parse vocab file read result from signal
    const QList<QVariant> arguments = readInfosFinishedSpy.takeFirst();
    const ReadVocabFileInfosResult result = qvariant_cast<ReadVocabFileInfosResult>(arguments.at(0));

    // Validate result
    QCOMPARE_NE(result.code, ErrorCode::Success);
    QCOMPARE(result.code, ErrorCode::NoVocabFilesGiven);
}

QTEST_APPLESS_MAIN(TestVocabFileHandler)

#include "tst_vocabfilehandler.moc"
