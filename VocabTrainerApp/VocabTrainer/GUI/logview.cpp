#include "logview.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QGroupBox>
#include <QSplitter>
#include <QHeaderView>

namespace
{
    const QString &logTag()
    {
        static const QString tag = QStringLiteral("LogView");
        return tag;
    }
}

LogView::LogView(Logger &logger, const Settings &settings, QWidget *parent)
    : QWidget{parent}, m_logger(logger)
{
    m_logger.verbose(logTag(), QStringLiteral("Constructing log view..."));

    // Construct view panels
    auto* viewSettingsPanel = createViewSettingsPanel(settings);
    auto* displayPanel = createDisplayPanel();

    // Splitter is very flexible with adjusting panel sizes
    auto* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(viewSettingsPanel);
    splitter->addWidget(displayPanel);

    // Construct layout
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(splitter);

    // Create GUI component connections
    createConnections();

    m_logger.verbose(logTag(), QStringLiteral("Constructing log view done!"));
}

void LogView::createInitialView()
{
    m_logger.verbose(logTag(), QStringLiteral("Creating the initial log view..."));
    refreshLog();
}

void LogView::createQueriedView(const LogQuery &query)
{
    m_logger.verbose(logTag(), QStringLiteral("Creating a log view with query..."));
    m_logger.verbose(logTag(), QStringLiteral("Emitting log query request..."));
    emit requestLogQuery(query);
}

void LogView::updateView(const QList<LogEntry> &entries)
{
    m_logger.verbose(logTag(), QStringLiteral("Updating view..."));
    m_logEntries = entries;
    refreshLogDisplay();
    m_logger.verbose(logTag(), QStringLiteral("Updating view done!"));
}

void LogView::browseLogFile()
{
    m_logger.verbose(logTag(), QStringLiteral("Browsing log file..."));
    const QString filePath = QFileDialog::getOpenFileName(this, QStringLiteral("Select log file"), m_logFilePathEdit->text(), QStringLiteral("Log files (*.log);;All files (*.*)"));

    if (filePath.isEmpty())
    {
        m_logger.debug(logTag(), QStringLiteral("No log file selected."));
        return;
    }
    m_logger.debug(logTag(), QStringLiteral("Log file '%1' selected.").arg(filePath));
    m_logFilePathEdit->setText(filePath);

    m_logger.verbose(logTag(), QStringLiteral("Browsing log file done!"));
}

void LogView::refreshLog()
{
    m_logger.verbose(logTag(), "Creating new log query request...");

    // Construct query and emit log query request
    LogQuery query;
    query.filePath = m_logFilePathEdit->text();
    query.maxEntryCount = m_maxEntryCountSpinBox->value();
    query.entryOrder = m_newestRadioButton->isChecked() ? LogEntrySelectionOrder::Newest : LogEntrySelectionOrder::Oldest;
    query.searchPattern = m_searchPatternTextEdit->text();
    query.useRegularExpression = m_useRegularExpressionCheckBox->isChecked();
    query.searchEntireEntry = m_searchEntireEntryCheckBox->isChecked();
    if (m_fromCheckBox->isChecked())
    {
        query.from = m_fromDateTimeEdit->dateTime();
    }
    if (m_toCheckBox->isChecked())
    {
        query.to = m_toDateTimeEdit->dateTime();
    }

    m_logger.verbose(logTag(), "Creating new log query request done!");

    createQueriedView(query);
}

void LogView::toggleFromQueryAvailability(bool checked)
{
    m_logger.verbose(logTag(), "Toggled from log query.");
    m_fromDateTimeEdit->setEnabled(checked);
}

void LogView::toggleToQueryAvailability(bool checked)
{
    m_logger.verbose(logTag(), "Toggled to log query.");
    m_toDateTimeEdit->setEnabled(checked);
}

void LogView::toggleInfoLogVisible(bool checked)
{
    m_logger.verbose(logTag(), "Toggled log level info.");
    m_showInfo = checked;
    refreshLogDisplay();
}

void LogView::toggleWarningLogVisible(bool checked)
{
    m_logger.verbose(logTag(), "Toggled log level warning.");
    m_showWarning = checked;
    refreshLogDisplay();
}

void LogView::toggleErrorLogVisible(bool checked)
{
    m_logger.verbose(logTag(), "Toggled log level error.");
    m_showError = checked;
    refreshLogDisplay();
}

void LogView::toggleDebugLogVisible(bool checked)
{
    m_logger.verbose(logTag(), "Toggled log level debug.");
    m_showDebug = checked;
    refreshLogDisplay();
}

void LogView::toggleVerboseLogVisible(bool checked)
{
    m_logger.verbose(logTag(), "Toggled log level verbose.");
    m_showVerbose = checked;
    refreshLogDisplay();
}

QWidget *LogView::createViewSettingsPanel(const Settings &settings)
{
    m_logger.verbose(logTag(), "Creating view settings panel...");

    auto* panel = new QWidget(this);

    // Allocate GUI components
    m_refreshButton = new QPushButton(QStringLiteral("Refresh log"), panel);

    // Create groups inside panel
    auto* logQueryGroup = createLogQueryGroup(panel, settings);
    auto* sortGroup = createViewingOptionsGroup(panel);

    // Construct layout
    auto* layout = new QVBoxLayout(panel);
    layout->addWidget(logQueryGroup);
    layout->addWidget(sortGroup);
    layout->addWidget(m_refreshButton);

    m_logger.verbose(logTag(), "Creating view settings panel done!");

    return panel;
}

QWidget *LogView::createDisplayPanel()
{
    m_logger.verbose(logTag(), "Creating display panel...");

    auto* panel = new QWidget(this);

    // Allocate GUI components
    m_entryCountLabel = new QLabel(QStringLiteral("Displaying 0/0 log entries"), this);

    // Construct log level visibility toggle buttons
    auto topButtonRowLayout = createLogLevelButtons(panel);

    // Construct log table
    createLogTable(panel);

    // Construct layout
    auto* layout = new QVBoxLayout(panel);
    layout->addLayout(topButtonRowLayout);
    layout->addWidget(m_logTable, 1);
    layout->addWidget(m_entryCountLabel, 0, Qt::AlignHCenter);

    m_logger.verbose(logTag(), "Creating display panel done!");

    return panel;
}

QGroupBox *LogView::createLogQueryGroup(QWidget *parent, const Settings &settings)
{
    m_logger.verbose(logTag(), "Creating log query group...");

    auto* group = new QGroupBox(QStringLiteral("Log Query"), parent);

    // Allocate GUI components
    auto* fileSelectionGroup = createFileSelectionFrame(group, settings);
    auto* entryLimitGroup = createEntryLimitFrame(group);
    auto* searchGroup = createSearchFrame(group);
    auto* searchIntervalGroup = createSearchIntervalFrame(group);

    auto* layout = new QVBoxLayout(group);
    layout->addWidget(fileSelectionGroup);
    layout->addWidget(entryLimitGroup);
    layout->addWidget(searchGroup);
    layout->addWidget(searchIntervalGroup);

    m_logger.verbose(logTag(), "Creating log query group done!");

    return group;
}

QFrame *LogView::createFileSelectionFrame(QWidget *parent, const Settings &settings)
{
    m_logger.verbose(logTag(), "Creating file selection frame...");

    auto* frame = new QFrame(parent);
    frame->setFrameShape(QFrame::StyledPanel);
    frame->setFrameShadow(QFrame::Raised);

    // Allocate GUI components
    m_logFilePathEdit = new QLineEdit(frame);
    m_browseButton = new QPushButton(QStringLiteral("Browse..."), frame);

    // Modify GUI component states
    m_logFilePathEdit->setText(settings.logFilePath);

    // Construct layout
    auto *fileLayout = new QHBoxLayout(frame);
    fileLayout->addWidget(m_logFilePathEdit);
    fileLayout->addWidget(m_browseButton);

    m_logger.verbose(logTag(), "Creating file selection frame done!");

    return frame;
}

QFrame *LogView::createSearchFrame(QWidget *parent)
{
    m_logger.verbose(logTag(), "Creating search frame...");

    auto* frame = new QFrame(parent);
    frame->setFrameShape(QFrame::StyledPanel);
    frame->setFrameShadow(QFrame::Raised);

    // Allocate GUI components
    m_searchPatternLabel = new QLabel(QStringLiteral("Search pattern:"), frame);
    m_searchPatternTextEdit = new QLineEdit(frame);
    m_useRegularExpressionCheckBox = new QCheckBox(QStringLiteral("Use regular expression"),frame);
    m_searchEntireEntryCheckBox = new QCheckBox(QStringLiteral("Search from entire entry"), frame);

    // Modify GUI component states
    m_useRegularExpressionCheckBox->setChecked(false);
    m_searchEntireEntryCheckBox->setChecked(false);

    // Construct layout
    auto* layout = new QVBoxLayout(frame);
    layout->addWidget(m_searchPatternLabel);
    layout->addWidget(m_searchPatternTextEdit);
    layout->addWidget(m_useRegularExpressionCheckBox);
    layout->addWidget(m_searchEntireEntryCheckBox);

    m_logger.verbose(logTag(), "Creating search frame done!");

    return frame;
}

QFrame *LogView::createEntryLimitFrame(QWidget *parent)
{
    m_logger.verbose(logTag(), "Creating entry limit frame...");

    auto* frame = new QFrame(parent);
    frame->setFrameShape(QFrame::StyledPanel);
    frame->setFrameShadow(QFrame::Raised);

    // Allocate GUI components
    m_maxEntryCountLabel = new QLabel(QStringLiteral("Max entries:"), frame);
    m_maxEntryCountSpinBox = new QSpinBox(frame);
    m_newestRadioButton = new QRadioButton(QStringLiteral("Newest"), frame);
    m_oldestRadioButton = new QRadioButton(QStringLiteral("Oldest"), frame);

    // Modify GUI component states
    m_maxEntryCountSpinBox->setRange(0, 1000000); // TODO: Max could be a setting
    m_maxEntryCountSpinBox->setValue(0);
    m_maxEntryCountSpinBox->setSpecialValueText(QStringLiteral("All"));
    m_newestRadioButton->setChecked(true);

    // Construct layout
    auto* layout = new QVBoxLayout(frame);
    layout->addWidget(m_maxEntryCountLabel);
    layout->addWidget(m_maxEntryCountSpinBox);
    layout->addWidget(m_newestRadioButton);
    layout->addWidget(m_oldestRadioButton);

    m_logger.verbose(logTag(), "Creating entry limit frame done!");

    return frame;
}

QGroupBox *LogView::createViewingOptionsGroup(QWidget *parent)
{
    m_logger.verbose(logTag(), "Creating viewing options group...");

    auto* group = new QGroupBox(QStringLiteral("Viewing"), parent);

    // TODO: Make sensible and clear options

    // Allocate GUI components
    m_timestampRadioButton = new QRadioButton(QStringLiteral("Timestamp"), group);
    m_tagRadioButton = new QRadioButton(QStringLiteral("Tag"), group);
    m_threadRadioButton = new QRadioButton(QStringLiteral("Thread"), group);

    // Modify GUI component states
    m_timestampRadioButton->setChecked(true);

    // Construct layout
    auto *layout = new QVBoxLayout(group);
    layout->addWidget(m_timestampRadioButton);
    layout->addWidget(m_tagRadioButton);
    layout->addWidget(m_threadRadioButton);

    m_logger.verbose(logTag(), "Creating viewing options group done!");

    return group;
}

QFrame *LogView::createSearchIntervalFrame(QWidget *parent)
{
    m_logger.verbose(logTag(), "Creating search interval frame...");

    auto* frame = new QFrame(parent);
    frame->setFrameShape(QFrame::StyledPanel);
    frame->setFrameShadow(QFrame::Raised);

    // Allocate GUI components
    m_fromCheckBox = new QCheckBox(QStringLiteral("From:"), frame);
    m_fromDateTimeEdit = new QDateTimeEdit(frame);
    m_toCheckBox = new QCheckBox(QStringLiteral("To:"), frame);
    m_toDateTimeEdit = new QDateTimeEdit(frame);

    // Modify GUI component states
    QDateTime now = QDateTime::currentDateTime();
    m_fromCheckBox->setChecked(true);
    m_toCheckBox->setChecked(false);
    m_fromDateTimeEdit->setCalendarPopup(true);
    m_toDateTimeEdit->setCalendarPopup(true);
    m_fromDateTimeEdit->setDateTime(now.addSecs(-5 * 60)); // 5 minutes from current time
    m_toDateTimeEdit->setDateTime(now);
    m_fromDateTimeEdit->setEnabled(true);
    m_toDateTimeEdit->setEnabled(false);

    // Construct layout
    auto* layout = new QGridLayout(frame);
    layout->addWidget(m_fromCheckBox, 0, 0);
    layout->addWidget(m_fromDateTimeEdit, 0, 1);
    layout->addWidget(m_toCheckBox, 1, 0);
    layout->addWidget(m_toDateTimeEdit, 1, 1);

    m_logger.verbose(logTag(), "Creating search interval frame done!");

    return frame;
}

QHBoxLayout *LogView::createLogLevelButtons(QWidget *parent)
{
    m_logger.verbose(logTag(), "Creating log level buttons...");

    // Allocate GUI components
    m_infoLogVisibleButton = new QPushButton(QStringLiteral("INFO"), parent);
    m_warningLogVisibleButton = new QPushButton(QStringLiteral("WARNING"), parent);
    m_errorLogVisibleButton = new QPushButton(QStringLiteral("ERROR"), parent);
    m_debugLogVisibleButton = new QPushButton(QStringLiteral("DEBUG"), parent);
    m_verboseLogVisibleButton = new QPushButton(QStringLiteral("VERBOSE"), parent);

    // Modify GUI component states
    const QList<QPushButton*> levelButtons = {
        m_infoLogVisibleButton,
        m_warningLogVisibleButton,
        m_errorLogVisibleButton,
        m_debugLogVisibleButton,
        m_verboseLogVisibleButton
    };

    for (QPushButton *button : levelButtons)
    {
        button->setCheckable(true);
        button->setChecked(true);
    }

    // Construct layout
    auto* layout = new QHBoxLayout;
    for (QPushButton *button : levelButtons)
    {
        layout->addWidget(button);
    }

    m_logger.verbose(logTag(), "Creating log level buttons done!");

    return layout;
}

void LogView::createLogTable(QWidget *parent)
{
    m_logger.verbose(logTag(), "Creating log table...");

    // Allocate GUI components
    m_logTableModel = new QStandardItemModel(parent);
    m_logTable = new QTableView(parent);

    // Modify GUI component state
    m_logTableModel->setColumnCount(7);
    m_logTableModel->setHorizontalHeaderLabels({
        QStringLiteral("Timestamp"),
        QStringLiteral("Sequence"),
        QStringLiteral("Thread"),
        QStringLiteral("Thread Name"),
        QStringLiteral("Level"),
        QStringLiteral("Tag"),
        QStringLiteral("Message")
    });

    m_logTable->setModel(m_logTableModel);
    m_logTable->setSortingEnabled(true);
    m_logTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_logTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_logTable->verticalHeader()->setVisible(false);

    auto *header = m_logTable->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents); // Timestamp
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents); // Sequence
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents); // Thread
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents); // Thread Name
    header->setSectionResizeMode(4, QHeaderView::ResizeToContents); // Level
    header->setSectionResizeMode(5, QHeaderView::ResizeToContents); // Tag
    header->setSectionResizeMode(6, QHeaderView::Stretch);          // Message

    m_logger.verbose(logTag(), "Creating log table done!");
}

void LogView::createConnections()
{
    m_logger.verbose(logTag(), "Creating GUI component connections...");

    // Push button connections
    connect(m_browseButton, &QPushButton::clicked, this, &LogView::browseLogFile);
    connect(m_refreshButton, &QPushButton::clicked, this, &LogView::refreshLog);

    // Checkbox connections
    connect(m_fromCheckBox, &QCheckBox::toggled, this, &LogView::toggleFromQueryAvailability);
    connect(m_toCheckBox, &QCheckBox::toggled, this, &LogView::toggleToQueryAvailability);

    // Log level visibilty toggle buttons
    connect(m_infoLogVisibleButton, &QPushButton::toggled, this, &LogView::toggleInfoLogVisible);
    connect(m_warningLogVisibleButton, &QPushButton::toggled, this, &LogView::toggleWarningLogVisible);
    connect(m_errorLogVisibleButton, &QPushButton::toggled, this, &LogView::toggleErrorLogVisible);
    connect(m_debugLogVisibleButton, &QPushButton::toggled, this, &LogView::toggleDebugLogVisible);
    connect(m_verboseLogVisibleButton, &QPushButton::toggled, this, &LogView::toggleVerboseLogVisible);

    m_logger.verbose(logTag(), "Creating GUI component connections done!");
}

void LogView::refreshLogDisplay()
{
    m_logger.verbose(logTag(), "Refreshing log display...");

    // Clear old log table
    m_logTableModel->removeRows(0, m_logTableModel->rowCount());

    // Go through log entries and add those that are toggled visible
    int visibleEntryCount = 0;
    for (const LogEntry &entry : m_logEntries)
    {
        if (!isEntryVisible(entry))
        {
            continue;
        }

        // Create row items
        auto *timeStampItem = new QStandardItem(entry.timestamp.toString(Qt::ISODateWithMs));
        auto *sequenceItem = new QStandardItem(QString::number(entry.sequence));
        auto *threadIdItem = new QStandardItem(QStringLiteral("0x%1").arg(QString::number(reinterpret_cast<quintptr>(entry.threadId), 16)));
        auto *threadNameItem = new QStandardItem(entry.threadName);
        auto *levelItem = new QStandardItem(logLevelToString(entry.level));
        auto *tagItem = new QStandardItem(entry.tag);
        auto *messageItem = new QStandardItem(entry.message);

        // Set data for non string entries to enable logical sorting
        timeStampItem->setData(entry.timestamp);
        sequenceItem->setData(entry.sequence);
        threadIdItem->setData(reinterpret_cast<quintptr>(entry.threadId));

        // Construct table row
        QList<QStandardItem*> row;
        row.append(timeStampItem);
        row.append(sequenceItem);
        row.append(threadIdItem);
        row.append(threadNameItem);
        row.append(levelItem);
        row.append(tagItem);
        row.append(messageItem);

        // Add row to log table
        m_logTableModel->appendRow(row);

        visibleEntryCount++;
    }

    m_entryCountLabel->setText(QStringLiteral("Displaying %1/%2 log entries").arg(QString::number(visibleEntryCount), QString::number(m_logEntries.size())));

    m_logger.verbose(logTag(), "Refreshing log display done!");
}

bool LogView::isEntryVisible(const LogEntry &entry) const
{
    switch (entry.level)
    {
    case LogLevel::Info:
        return m_showInfo;

    case LogLevel::Warning:
        return m_showWarning;

    case LogLevel::Error:
        return m_showError;

    case LogLevel::Debug:
        return m_showDebug;

    case LogLevel::Verbose:
        return m_showVerbose;
    }

    return true;
}
