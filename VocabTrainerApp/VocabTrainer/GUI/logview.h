#ifndef LOGVIEW_H
#define LOGVIEW_H

#include "../logger.h"

#include <QWidget>

class LogView : public QWidget
{
    Q_OBJECT
public:
    explicit LogView(Logger &logger, QWidget *parent = nullptr);

signals:

private:
    Logger &m_logger;
};

#endif // LOGVIEW_H
