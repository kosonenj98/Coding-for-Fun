#ifndef TST_VOCABFILEHANDLER_H
#define TST_VOCABFILEHANDLER_H

#include "Logging/logger.h"
#include "Application/vocabfilehandler.h"

#include <QObject>

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

#endif // TST_VOCABFILEHANDLER_H
