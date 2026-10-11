#ifndef TST_VOCABGENERATOR_H
#define TST_VOCABGENERATOR_H

#include "Application/vocabgenerator.h"

#include <QObject>

class TestVocabGenerator : public QObject
{
    Q_OBJECT

private slots:
    void init();

    void generatesSimpleData();
    void generatesVariantsWithCurlyBraces();
    void generatesVariantsWithParenthesis();
    void generatesVariantsWithSquareBrackets();
    void generatesVariantsWithNestedBrackets();
    void supportsVariantConcatenationWithoutSpaces();

private:
    std::unique_ptr<Logger> m_logger;
    std::unique_ptr<VocabGenerator> m_vocabGenerator;
};

#endif // TST_VOCABGENERATOR_H
