#include "logview.h"

#include <QVBoxLayout>
#include <QLabel>

namespace
{
    const QString &logTag()
    {
        static const QString tag = QStringLiteral("LogView");
        return tag;
    }
}

LogView::LogView(Logger &logger, QWidget *parent)
    : QWidget{parent}, m_logger(logger)
{
    m_logger.verbose(logTag(), QStringLiteral("Constructing log view..."));
    auto *layout = new QVBoxLayout(this);

    auto *label = new QLabel(QStringLiteral("Log View"), this);
    label->setAlignment(Qt::AlignCenter);

    layout->addWidget(label);

    m_logger.verbose(logTag(), QStringLiteral("Constructing log view done!"));
}
