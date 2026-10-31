#include "dialogservice.h"

#include <QMessageBox>

DialogService::DialogService(QWidget *parentWidget, QObject *parent)
    : QObject{parent}, m_parent(parentWidget)
{
}

void DialogService::showInformation(const QString &title, const QString &message)
{
    QMessageBox::information(m_parent, title, message);
}

void DialogService::showWarning(const QString &title, const QString &message)
{
    QMessageBox::warning(m_parent, title, message);
}

void DialogService::showError(const QString &title, const QString &message)
{
    QMessageBox::critical(m_parent, title, message);
}

bool DialogService::askYesOrNo(const QString &title, const QString &prompt, QMessageBox::Icon icon, QMessageBox::StandardButton defaultButton)
{
    QMessageBox messageBox(m_parent);
    messageBox.setWindowTitle(title);
    messageBox.setText(prompt);
    messageBox.setIcon(icon);
    messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    messageBox.setDefaultButton(defaultButton);

    return messageBox.exec() == QMessageBox::Yes;
}

QMessageBox::StandardButton DialogService::askYesOrNoOrCancel(const QString &title, const QString &prompt, QMessageBox::Icon icon, QMessageBox::StandardButton defaultButton)
{
    QMessageBox messageBox(m_parent);
    messageBox.setWindowTitle(title);
    messageBox.setText(prompt);
    messageBox.setIcon(icon);
    messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    messageBox.setDefaultButton(defaultButton);

    return static_cast<QMessageBox::StandardButton>(messageBox.exec());
}
