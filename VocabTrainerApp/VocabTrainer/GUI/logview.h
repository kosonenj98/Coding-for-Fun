#ifndef LOGVIEW_H
#define LOGVIEW_H

#include "Logging/logger.h"
#include "DataTypes/requests.h"

#include <QWidget>
#include <QDateTimeEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QLineEdit>
#include <QTableView>
#include <QStandardItemModel>
#include <QSpinBox>
#include <QLabel>
#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>

enum class LogSortMode
{
    Timestamp,
    Tag,
    Thread
};

class LogView : public QWidget
{
    Q_OBJECT
public:
    explicit LogView(Logger &logger, const Settings &settings, QWidget *parent = nullptr);

    void createInitialView();

    void createQueriedView(const LogQueryRequest &query);

    void updateView(const QList<Log::Entry> &entries);

public slots:

signals:
    void requestLogQuery(const LogQueryRequest &query);

private slots:
    void browseLogFile();
    void refreshLog();
    void toggleFromQueryAvailability(bool checked);
    void toggleToQueryAvailability(bool checked);
    void toggleInfoLogVisible(bool checked);
    void toggleWarningLogVisible(bool checked);
    void toggleErrorLogVisible(bool checked);
    void toggleDebugLogVisible(bool checked);
    void toggleVerboseLogVisible(bool checked);

private:
    // View settings panel
    QWidget *createViewSettingsPanel(const Settings &settings);

    // Log query
    QGroupBox *createLogQueryGroup(QWidget *parent, const Settings& settings);
    QFrame *createFileSelectionFrame(QWidget *parent, const Settings &settings);
    QFrame *createEntryLimitFrame(QWidget *parent);
    QFrame *createSearchFrame(QWidget *parent);
    QFrame *createSearchIntervalFrame(QWidget *parent);

    // Viewing options
    QGroupBox *createViewingOptionsGroup(QWidget *parent);

    /*-----------------------------------------------------*/

    // Display panel
    QWidget *createDisplayPanel();
    QHBoxLayout *createLogLevelButtons(QWidget *parent);
    void createLogTable(QWidget *parent);

    /*-----------------------------------------------------*/

    // GUI component connections
    void createConnections();

    /*-----------------------------------------------------*/

    void refreshLogDisplay();
    bool isEntryVisible(const Log::Entry &entry) const;

    /*-----------------------------------------------------*/

    // LogView logic attributes
    Logger &m_logger;
    LogSortMode m_sortMode = LogSortMode::Timestamp;
    bool m_showInfo = true;
    bool m_showWarning = true;
    bool m_showError = true;
    bool m_showDebug = true;
    bool m_showVerbose = true;
    QList<Log::Entry> m_logEntries;

    /*-----------------------------------------------------*/

    // Query control attributes

    // Log file selection
    QLineEdit *m_logFilePathEdit;
    QPushButton *m_browseButton;

    // Entry limit
    QLabel *m_maxEntryCountLabel;
    QSpinBox *m_maxEntryCountSpinBox;
    QRadioButton* m_oldestRadioButton;
    QRadioButton* m_newestRadioButton;

    // Search pattern
    QLabel *m_searchPatternLabel;
    QLineEdit *m_searchPatternTextEdit;
    QCheckBox* m_useRegularExpressionCheckBox;
    QCheckBox* m_searchEntireEntryCheckBox;

    // Search interval
    QCheckBox *m_fromCheckBox;
    QDateTimeEdit *m_fromDateTimeEdit;
    QCheckBox *m_toCheckBox;
    QDateTimeEdit *m_toDateTimeEdit;

    // Log refresh
    QPushButton *m_refreshButton;

    /*-----------------------------------------------------*/

    // Viewing options attributes
    QRadioButton *m_timestampRadioButton;
    QRadioButton *m_tagRadioButton;
    QRadioButton *m_threadRadioButton;

    /*-----------------------------------------------------*/

    // Display control attributes

    // Log level buttons
    QPushButton *m_infoLogVisibleButton;
    QPushButton *m_warningLogVisibleButton;
    QPushButton *m_errorLogVisibleButton;
    QPushButton *m_debugLogVisibleButton;
    QPushButton *m_verboseLogVisibleButton;

    // Log table
    QTableView *m_logTable;
    QStandardItemModel *m_logTableModel;

    // Visible entries and total entries count
    QLabel* m_entryCountLabel;
};

#endif // LOGVIEW_H
