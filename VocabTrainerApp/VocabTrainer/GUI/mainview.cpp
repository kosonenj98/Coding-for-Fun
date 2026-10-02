#include "mainview.h"

#include <QVBoxLayout>
#include <QLabel>

namespace
{
    const QString &logTag()
    {
        static const QString tag = QStringLiteral("MainView");
        return tag;
    }
}

MainView::MainView(Logger &logger, QWidget *parent)
    : QWidget{parent}, m_logger(logger)
{
    m_logger.verbose(logTag(), QStringLiteral("Constructing main view..."));
    auto *layout = new QVBoxLayout(this);

    auto *label = new QLabel(QStringLiteral("Hello World!"), this);
    label->setAlignment(Qt::AlignCenter);

    layout->addWidget(label);

    m_logger.verbose(logTag(), QStringLiteral("Constructing main view done!"));
}
