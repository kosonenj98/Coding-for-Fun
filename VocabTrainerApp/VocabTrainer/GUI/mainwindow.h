#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "../logger.h"

#include <QMainWindow>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(Logger &logger, QWidget *parent = nullptr);

signals:

private:
    Logger &m_logger;

    QTabWidget *m_tabWidget;
};

#endif // MAINWINDOW_H
