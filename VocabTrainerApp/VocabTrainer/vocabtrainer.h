#ifndef VOCABTRAINER_H
#define VOCABTRAINER_H

#include <QObject>

class VocabTrainer : public QObject
{
    Q_OBJECT

public:
    VocabTrainer(QObject *parent = nullptr);
    ~VocabTrainer();
};
#endif // VOCABTRAINER_H
