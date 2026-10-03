#ifndef MAINVIEW_H
#define MAINVIEW_H

#include "../Logging/logger.h"

#include <QWidget>

class MainView : public QWidget
{
    Q_OBJECT
public:
    explicit MainView(Logger &logger, QWidget *parent = nullptr);

signals:

private:
    Logger &m_logger;
};

#endif // MAINVIEW_H
