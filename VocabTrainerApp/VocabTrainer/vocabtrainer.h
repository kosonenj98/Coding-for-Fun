#ifndef VOCABTRAINER_H
#define VOCABTRAINER_H

#include "logger.h"
#include "logfilewriter.h"

#include <QObject>

class VocabTrainer : public QObject
{
    Q_OBJECT

public:
    VocabTrainer(QObject *parent = nullptr);
    ~VocabTrainer();

    void initialize();

private:
    Logger m_logger;

    QThread *m_logThread;
    LogFileWriter *m_logFileWriter;
};
#endif // VOCABTRAINER_H
