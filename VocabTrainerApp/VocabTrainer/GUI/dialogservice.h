#ifndef DIALOGSERVICE_H
#define DIALOGSERVICE_H

#include <QObject>

class DialogService : public QObject
{
    Q_OBJECT
public:
    explicit DialogService(QWidget *parentWidget, QObject *parent = nullptr);

    void showInformation(const QString& title, const QString& message);

    void showWarning(const QString& title, const QString& message);

    void showError(const QString& title, const QString& message);

public slots:

signals:

private:
    QWidget *m_parent;
};

#endif // DIALOGSERVICE_H
