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
