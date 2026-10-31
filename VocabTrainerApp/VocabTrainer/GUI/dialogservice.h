#ifndef DIALOGSERVICE_H
#define DIALOGSERVICE_H

#include <QObject>
#include <QMessageBox>

class DialogService : public QObject
{
    Q_OBJECT
public:
    explicit DialogService(QWidget *parentWidget, QObject *parent = nullptr);

    void showInformation(const QString& title, const QString& message);
    void showWarning(const QString& title, const QString& message);
    void showError(const QString& title, const QString& message);

    bool askYesOrNo(const QString &title, const QString &prompt, QMessageBox::Icon icon = QMessageBox::Question, QMessageBox::StandardButton defaultButton = QMessageBox::StandardButton::No);

    QMessageBox::StandardButton askYesOrNoOrCancel(const QString &title, const QString &prompt, QMessageBox::Icon icon = QMessageBox::Question, QMessageBox::StandardButton defaultButton = QMessageBox::StandardButton::Cancel);

public slots:

signals:

private:
    QWidget *m_parent;
};

#endif // DIALOGSERVICE_H
