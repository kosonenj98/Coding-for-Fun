#ifndef VOCABTRAINER_H
#define VOCABTRAINER_H

#include "logger.h"
#include "logfilewriter.h"

#include <QObject>

class VocabTrainer : public QObject
{
    Q_OBJECT

public:
    VocabTrainer(Logger &logger, QObject *parent = nullptr);
    ~VocabTrainer();

    void initialize();
    void shutdown();

private:
    Logger &m_logger;

};
#endif // VOCABTRAINER_H
