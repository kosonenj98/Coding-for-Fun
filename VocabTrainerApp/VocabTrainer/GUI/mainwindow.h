#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "../Logging/logger.h"
#include "../settingshandler.h"
#include "../errorcode.h"
#include "mainview.h"
#include "settingsview.h"
#include "logview.h"
#include "dialogservice.h"

#include <QMainWindow>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(Logger &logger, SettingsHandler &handler, QWidget *parent = nullptr);

public slots:
    void handleLogQueryRequest(const LogQuery &query);
    void handleLogQuerySucceeded(const QList<LogEntry> &entries);
    void handleLogQuerySucceededPartially(const QList<LogEntry> &entries, int failedEntriesCount);
    void handleLogQueryFailed(ErrorCode code);

    void handleApplyNewSettingsRequest(const Settings &newSettings);
    void handleApplyNewSettingsSucceeded();
    void handleApplyNewSettingsFailed(ErrorCode code, const Settings &settings);

    void handleViewTabChanged(int index);

signals:
    void requestLogQuery(const LogQuery &query);
    void requestApplyNewSettings(const Settings &newSettings);

private:
    Logger &m_logger;
    SettingsHandler &m_settingsHandler;

    QTabWidget *m_tabWidget;

    MainView *m_mainView = nullptr;
    SettingsView *m_settingsView = nullptr;
    LogView *m_logView = nullptr;

    DialogService* m_dialogService;
};

#endif // MAINWINDOW_H
