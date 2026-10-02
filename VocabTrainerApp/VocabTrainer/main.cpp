#include "vocabtrainer.h"

#include <QApplication>
#include <QThread>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setApplicationName(QStringLiteral("VocabTrainer"));

    QThread::currentThread()->setObjectName(QStringLiteral("MainThread"));
    VocabTrainer vt;
    vt.initialize();

    return a.exec();
}
