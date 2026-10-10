#include <QCoreApplication>
#include <QtTest>

#include "tst_vocabfilehandler.h"
#include "tst_vocabgenerator.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    TestVocabFileHandler fileHandlerTests;
    TestVocabGenerator generatorTests;

    int result = 0;
    result |= QTest::qExec(&fileHandlerTests, argc, argv);
    result |= QTest::qExec(&generatorTests, argc, argv);

    return result;
}
