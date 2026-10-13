#include "mainview.h"

#include <QSplitter>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QFileIconProvider>
#include <QDirIterator>

namespace
{
    constexpr int CARD_MIN_WIDTH = 640;
    constexpr int CARD_MARGIN_HORIZONTAL = 36;
    constexpr int CARD_MARGIN_VERTICAL = 40;
    constexpr int CARD_CONTENT_SPACING = 36;

    constexpr int PANEL_MARGIN = 20;
    constexpr int PANEL_SPACING = 16;

    constexpr int INFO_FONT_SIZE = 20;
    constexpr int DIRECTION_FONT_SIZE = 15;

    constexpr int NAV_BUTTON_WIDTH = 44;
    constexpr int NAV_BUTTON_HEIGHT = 40;
    constexpr int NAV_BUTTON_FONT_SIZE = 20;

    constexpr int QUESTION_MIN_HEIGHT = 80;
    constexpr int ANSWER_MIN_HEIGHT = 56;
    constexpr int QUESTION_FONT_SIZE = 30;
    constexpr int ANSWER_FONT_SIZE = 20;
    constexpr int STATUS_FONT_SIZE = 40;
    constexpr int FEEDBACK_FONT_SIZE = 20;
    constexpr int FEEDBACK_SPACING = 8;
    constexpr int PROGRESS_BAR_MIN_HEIGHT = 24;

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

    // Construct view panels
    auto *trainingSettingsPanel = createTrainingSettingsPanel();
    auto *displayPanel = createDisplayPanel();

    // Splitter is very flexible with adjusting panel sizes
    auto* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(trainingSettingsPanel);
    splitter->addWidget(displayPanel);

    // Construct layout
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(splitter);

    // Create GUI component connections
    createConnections();

    m_logger.verbose(logTag(), QStringLiteral("Constructing main view done!"));
}

void MainView::createInitialView()
{
    // Show the welcome view initially
    m_displayStack->setCurrentWidget(m_welcomeView);
}

void MainView::updateVocabTree(const QList<Vocab::Info> vocabInfos)
{
    // TODO: How to keep previous settings and only modify those that are needed?
    m_vocabTree->clear();

    for (const Vocab::Info &info : vocabInfos)
    {
        // Create the vocabulary item
        auto *vocabItem = new QTreeWidgetItem(m_vocabTree);
        vocabItem->setText(0, info.title);
        vocabItem->setFlags(vocabItem->flags() | Qt::ItemIsUserCheckable);
        vocabItem->setCheckState(0, Qt::Checked);

        // Create a child item for each group
        for (const QString &groupTitle : info.groupTitles)
        {
            auto *groupItem = new QTreeWidgetItem(vocabItem);
            groupItem->setText(0, groupTitle);
            groupItem->setFlags(vocabItem->flags() | Qt::ItemIsUserCheckable);
            groupItem->setCheckState(0, Qt::Checked);
        }

        // Expand the vocabulary item only if one vocab is present
        vocabItem->setExpanded(!(vocabInfos.count() > 1));
    }
}

void MainView::browseVocabFiles()
{
    m_logger.verbose(logTag(), QStringLiteral("Browsing vocabulary files..."));

    const QStringList paths = QFileDialog::getOpenFileNames(this, QStringLiteral("Select vocabulary files"), QString(), QStringLiteral("Vocab files (*.txt);;All files (*.*)"));

    if (paths.isEmpty())
    {
        m_logger.debug(logTag(), QStringLiteral("No vocabulary files selected."));
        return;
    }

    for (const QString &path : paths)
    {
        addVocabSource(path);
    }

    m_logger.verbose(logTag(), QStringLiteral("Browsing vocabulary files done!"));
}

void MainView::browseVocabFolder()
{
    m_logger.verbose(logTag(), QStringLiteral("Browsing vocabulary folder..."));

    const QString path = QFileDialog::getExistingDirectory(this, QStringLiteral("Select vocabulary folder"), QString(), QFileDialog::ShowDirsOnly);

    if (path.isEmpty())
    {
        m_logger.debug(logTag(), QStringLiteral("No vocabulary folder selected."));
        return;
    }

    addVocabSource(path);

    m_logger.verbose(logTag(), QStringLiteral("Browsing vocabulary folder done!"));
}

void MainView::switchTrainingDirection()
{
    switch (m_trainingDirection)
    {
    case Training::Direction::Forward:
        m_trainingDirection = Training::Direction::Reversed;
        m_leftFlagLabel->setText(QStringLiteral("🇫🇮"));
        m_rightFlagLabel->setText(QStringLiteral("🇮🇹"));
        break;

    case Training::Direction::Reversed:
        m_trainingDirection = Training::Direction::Forward;
        m_leftFlagLabel->setText(QStringLiteral("🇮🇹"));
        m_rightFlagLabel->setText(QStringLiteral("🇫🇮"));
        break;
    }

    m_logger.debug(logTag(), QStringLiteral("Training direction switched."));
}

void MainView::initializeVocabLoad()
{
    LoadVocabsRequest request;
    request.direction = m_trainingDirection;

    QList<VocabFile::Info> infos;
    for (const auto &path : m_vocabSourcePaths)
    {
        VocabFile::Info info;
        info.filePath = path;
        infos.append(info);
    }
    request.infos = infos;

    m_logger.verbose(logTag(), QStringLiteral("Emitting vocab load request..."));
    emit requestVocabLoad(request);
}

QWidget *MainView::createTrainingSettingsPanel()
{
    m_logger.verbose(logTag(), "Creating view settings panel...");

    auto *panel = new QWidget(this);

    auto *trainingSettingsGroup = createTrainingSettingsGroup(panel);
    m_startButton = new QPushButton(QStringLiteral("Start"), panel);

    auto *layout = new QVBoxLayout(panel);
    layout->addWidget(trainingSettingsGroup);
    layout->addWidget(m_startButton);

    m_logger.verbose(logTag(), "Creating view settings panel done!");

    return panel;
}

QWidget *MainView::createDisplayPanel()
{
    m_logger.verbose(logTag(), "Creating display panel...");

    auto *panel = new QWidget(this);

    // Create the stacked widget
    m_displayStack = new QStackedWidget(panel);

    // Create the individual views
    m_welcomeView = createWelcomeView(m_displayStack);
    m_loadingView = createLoadingView(m_displayStack);
    m_errorView = createErrorView(m_displayStack);
    m_trainingView = createTrainingView(m_displayStack);

    // Add views to the stack
    m_displayStack->addWidget(m_welcomeView);
    m_displayStack->addWidget(m_loadingView);
    m_displayStack->addWidget(m_errorView);
    m_displayStack->addWidget(m_trainingView);

    // Construct the layout
    auto *layout = new QVBoxLayout(panel);
    layout->addWidget(m_displayStack);

    m_logger.verbose(logTag(), "Creating display panel done!");

    return panel;
}

QGroupBox *MainView::createTrainingSettingsGroup(QWidget *parent)
{
    auto *group = new QGroupBox(QStringLiteral("Training settings"), parent);

    auto *sourceFrame = createVocabSourceFrame(group);
    auto *directionFrame = createTrainingDirectionFrame(group);
    auto *treeFrame = createVocabTreeFrame(group);

    auto *layout = new QVBoxLayout(group);
    layout->addWidget(sourceFrame);
    layout->addWidget(directionFrame);
    layout->addWidget(treeFrame, 1);

    return group;
}

QFrame *MainView::createVocabSourceFrame(QWidget *parent)
{
    m_logger.verbose(logTag(), "Creating vocabulary source frame...");

    auto *frame = new QFrame(parent);
    frame->setFrameShape(QFrame::StyledPanel);
    frame->setFrameShadow(QFrame::Raised);


    // Allocate GUI components
    m_vocabLabel = new QLabel(QStringLiteral("Vocabulary sources:"), frame);

    m_addFilesButton = new QPushButton(QStringLiteral("Add files"), frame);

    m_addFoldersButton = new QPushButton(QStringLiteral("Add folders"), frame);

    m_removeSourceButton = new QPushButton(QStringLiteral("Remove selected"), frame);

    m_vocabSourceList = new QListWidget(frame);

    // Modify GUI component states
    m_vocabSourceList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_vocabSourceList->setMaximumHeight(100);
    m_vocabSourceList->setToolTip(QStringLiteral("Selected vocabulary files and folders"));

    // Construct button layout
    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_addFilesButton);
    buttonLayout->addWidget(m_addFoldersButton);

    // Construct layout
    auto *layout = new QVBoxLayout(frame);
    layout->addWidget(m_vocabLabel);
    layout->addLayout(buttonLayout);
    layout->addWidget(m_vocabSourceList);
    layout->addWidget(m_removeSourceButton);

    m_logger.verbose(logTag(), QStringLiteral("Creating vocabulary source frame done!"));

    return frame;
}

QFrame *MainView::createTrainingDirectionFrame(QWidget *parent)
{
    m_logger.verbose(logTag(), "Creating training direction frame...");

    auto *frame = new QFrame(parent);
    frame->setFrameShape(QFrame::StyledPanel);
    frame->setFrameShadow(QFrame::Raised);

    // Allocate GUI components
    m_switchDirectionLabel = new QLabel(QStringLiteral("Training direction:"), frame);
    m_leftFlagLabel = new QLabel(frame);
    auto *arrowLabel = new QLabel(QStringLiteral("→"), frame);
    m_rightFlagLabel = new QLabel(frame);
    m_switchDirectionButton = new QPushButton(QStringLiteral("⇄"), frame);

    // Modify GUI component states
    m_leftFlagLabel->setAlignment(Qt::AlignCenter);
    m_rightFlagLabel->setAlignment(Qt::AlignCenter);
    m_leftFlagLabel->setText(QStringLiteral("🇮🇹"));
    m_rightFlagLabel->setText(QStringLiteral("🇫🇮"));

    QFont font;
    font.setPointSize(DIRECTION_FONT_SIZE);
    m_leftFlagLabel->setFont(font);
    arrowLabel->setFont(font);
    m_rightFlagLabel->setFont(font);

    m_switchDirectionButton->setToolTip(QStringLiteral("Switch training direction"));

    // Construct layout
    auto *headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->addWidget(m_switchDirectionLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(m_switchDirectionButton);

    auto *directionLayout = new QHBoxLayout();
    directionLayout->addStretch();
    directionLayout->addWidget(m_leftFlagLabel);
    directionLayout->addWidget(arrowLabel);
    directionLayout->addWidget(m_rightFlagLabel);
    directionLayout->addStretch();

    auto *layout = new QVBoxLayout(frame);
    layout->addLayout(headerLayout);
    layout->addLayout(directionLayout);

    m_logger.verbose(logTag(), "Creating training direction frame done!");

    return frame;
}

QFrame *MainView::createVocabTreeFrame(QWidget *parent)
{
    m_logger.verbose(logTag(), "Creating vocabulary tree frame...");

    // Allocate the frame
    auto *frame = new QFrame(parent);
    frame->setFrameShape(QFrame::StyledPanel);
    frame->setFrameShadow(QFrame::Raised);

    // Allocate GUI components
    m_vocabTreeLabel = new QLabel(QStringLiteral("Vocab contents"), frame);
    m_vocabTree = createVocabularyTree(frame);

    // Construct the layout
    auto *layout = new QVBoxLayout(frame);
    layout->addWidget(m_vocabTreeLabel);
    layout->addWidget(m_vocabTree);

    m_logger.verbose(logTag(), "Creating vocabulary tree frame done!");

    return frame;
}

QTreeWidget *MainView::createVocabularyTree(QWidget *parent)
{
    m_logger.verbose(logTag(), "Creating vocabulary tree...");

    // Allocate the tree widget
    auto *tree = new QTreeWidget(parent);

    // Configure the tree
    tree->setHeaderHidden(true);
    tree->setRootIsDecorated(true);
    tree->setAlternatingRowColors(true);
    tree->setSelectionMode(QAbstractItemView::NoSelection);

    m_logger.verbose(logTag(), "Creating vocabulary tree done!");

    return tree;
}

QWidget *MainView::createWelcomeView(QWidget *parent)
{
    auto *view = new QWidget(parent);

    auto *label = new QLabel(QStringLiteral("Select vocab and start"), view);
    label->setAlignment(Qt::AlignCenter);

    QFont font = label->font();
    font.setPointSize(INFO_FONT_SIZE);
    label->setFont(font);

    auto *layout = new QVBoxLayout(view);
    layout->addStretch();
    layout->addWidget(label);
    layout->addStretch();

    return view;
}

QWidget *MainView::createLoadingView(QWidget *parent)
{
    auto *view = new QWidget(parent);

    auto *label = new QLabel(QStringLiteral("Loading vocabulary..."), view);
    label->setAlignment(Qt::AlignCenter);

    QFont font = label->font();
    font.setPointSize(INFO_FONT_SIZE);
    label->setFont(font);

    auto *layout = new QVBoxLayout(view);
    layout->addStretch();
    layout->addWidget(label);
    layout->addStretch();

    return view;
}

QWidget *MainView::createErrorView(QWidget *parent)
{
    auto *view = new QWidget(parent);

    m_errorLabel = new QLabel(view);
    m_errorLabel->setAlignment(Qt::AlignCenter);
    m_errorLabel->setWordWrap(true);

    QFont font = m_errorLabel->font();
    font.setPointSize(INFO_FONT_SIZE);
    m_errorLabel->setFont(font);

    auto *layout = new QVBoxLayout(view);
    layout->addStretch();
    layout->addWidget(m_errorLabel);
    layout->addStretch();

    return view;
}

QWidget *MainView::createTrainingView(QWidget *parent)
{
    auto *panel = new QWidget(parent);

    // Create navigation
    auto *navigationBar = createNavigationBar(panel);

    auto *separator = new QFrame(panel);
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);

    // Create the question card
    auto *cardFrame = new QFrame(panel);
    cardFrame->setFrameShape(QFrame::StyledPanel);
    cardFrame->setFrameShadow(QFrame::Raised);
    cardFrame->setMinimumWidth(CARD_MIN_WIDTH);

    m_questionLabel = createQuestionLabel(cardFrame);
    m_answerLineEdit = createAnswerInput(cardFrame);

    auto *cardLayout = new QVBoxLayout(cardFrame);
    cardLayout->setContentsMargins(
        CARD_MARGIN_HORIZONTAL,
        CARD_MARGIN_VERTICAL,
        CARD_MARGIN_HORIZONTAL,
        CARD_MARGIN_VERTICAL);
    cardLayout->setSpacing(CARD_CONTENT_SPACING);
    cardLayout->addWidget(m_questionLabel);
    cardLayout->addWidget(m_answerLineEdit);

    // Center the question card
    auto *cardRow = new QHBoxLayout();
    cardRow->addStretch();
    cardRow->addWidget(cardFrame);
    cardRow->addStretch();

    // Create feedback outside the card
    m_answerResult = createAnswerResult(panel);
    m_answerResult->setMinimumWidth(CARD_MIN_WIDTH);

    auto *resultRow = new QHBoxLayout();
    resultRow->addStretch();
    resultRow->addWidget(m_answerResult);
    resultRow->addStretch();

    // Create progress bar
    m_progressBar = createProgressBar(panel);

    // Construct the main layout
    auto *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(
        PANEL_MARGIN,
        PANEL_MARGIN,
        PANEL_MARGIN,
        PANEL_MARGIN);
    layout->setSpacing(PANEL_SPACING);

    layout->addWidget(navigationBar);
    layout->addWidget(separator);
    layout->addStretch(1);
    layout->addLayout(cardRow);
    layout->addLayout(resultRow);
    layout->addStretch(1);
    layout->addWidget(m_progressBar);

    m_logger.verbose(logTag(), "Creating display panel done!");

    return panel;
}

QWidget *MainView::createNavigationBar(QWidget *parent)
{
    auto *bar = new QWidget(parent);

    m_previousButton = new QPushButton(QStringLiteral("←"), bar);
    m_nextButton = new QPushButton(QStringLiteral("→"), bar);

    m_roundLabel = new QLabel(bar);
    m_cardCounterLabel = new QLabel(bar);

    const QSize buttonSize(NAV_BUTTON_WIDTH, NAV_BUTTON_HEIGHT);
    m_previousButton->setFixedSize(buttonSize);
    m_nextButton->setFixedSize(buttonSize);

    QFont arrowFont;
    arrowFont.setPointSize(NAV_BUTTON_FONT_SIZE);
    m_previousButton->setFont(arrowFont);
    m_nextButton->setFont(arrowFont);

    m_previousButton->setToolTip(QStringLiteral("Previous card"));
    m_nextButton->setToolTip(QStringLiteral("Next card"));

    m_cardCounterLabel->setAlignment(Qt::AlignCenter);
    m_roundLabel->setAlignment(Qt::AlignCenter);

    QFont roundFont = m_roundLabel->font();
    roundFont.setBold(true);
    m_roundLabel->setFont(roundFont);

    auto *counterLayout = new QVBoxLayout();
    counterLayout->setContentsMargins(0, 0, 0, 0);
    counterLayout->addWidget(m_roundLabel);
    counterLayout->addWidget(m_cardCounterLabel);

    auto *layout = new QHBoxLayout(bar);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_previousButton);
    layout->addStretch();
    layout->addLayout(counterLayout);
    layout->addStretch();
    layout->addWidget(m_nextButton);

    return bar;
}

QLabel *MainView::createQuestionLabel(QWidget *parent)
{
    auto *label = new QLabel(parent);

    label->setAlignment(Qt::AlignCenter);
    label->setWordWrap(true);
    label->setMinimumHeight(QUESTION_MIN_HEIGHT);

    QFont font = label->font();
    font.setPointSize(QUESTION_FONT_SIZE);
    font.setBold(true); // TODO: Bold only parts marked with *
    label->setFont(font);

    return label;
}

QLineEdit *MainView::createAnswerInput(QWidget *parent)
{
    auto *input = new QLineEdit(parent);

    input->setPlaceholderText(QStringLiteral("Type your answer..."));
    input->setAlignment(Qt::AlignCenter);
    input->setMinimumHeight(ANSWER_MIN_HEIGHT);

    QFont font = input->font();
    font.setPointSize(ANSWER_FONT_SIZE);
    input->setFont(font);

    return input;
}

QWidget *MainView::createAnswerResult(QWidget *parent)
{
    auto *result = new QWidget(parent);

    auto *layout = new QVBoxLayout(result);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(FEEDBACK_SPACING);

    m_answerStatusLabel = new QLabel(result);
    m_answerStatusLabel->setAlignment(Qt::AlignCenter);

    QFont iconFont = m_answerStatusLabel->font();
    iconFont.setPointSize(STATUS_FONT_SIZE);
    m_answerStatusLabel->setFont(iconFont);

    m_feedbackLabel = new QLabel(result);
    m_feedbackLabel->setAlignment(Qt::AlignCenter);
    m_feedbackLabel->setWordWrap(true);

    QFont feedbackFont = m_feedbackLabel->font();
    feedbackFont.setPointSize(FEEDBACK_FONT_SIZE);
    m_feedbackLabel->setFont(feedbackFont);

    layout->addWidget(m_answerStatusLabel);
    layout->addWidget(m_feedbackLabel);

    return result;
}

QProgressBar *MainView::createProgressBar(QWidget *parent)
{
    auto *progressBar = new QProgressBar(parent);

    progressBar->setFormat(QStringLiteral("%v / %m (%p%)"));
    progressBar->setMinimumHeight(PROGRESS_BAR_MIN_HEIGHT);

    return progressBar;
}

void MainView::createConnections()
{
    m_logger.verbose(logTag(), "Creating GUI component connections...");

    // Push button connections
    connect(m_addFilesButton, &QPushButton::clicked, this, &MainView::browseVocabFiles);
    connect(m_addFoldersButton, &QPushButton::clicked, this, &MainView::browseVocabFolder);
    connect(m_removeSourceButton, &QPushButton::clicked, this, &MainView::removeSelectedVocabSource);
    connect(m_switchDirectionButton, &QPushButton::clicked, this,&MainView::switchTrainingDirection);

    m_logger.verbose(logTag(), "Creating GUI component connections done!");
}

void MainView::addVocabSource(const QString &filePath)
{
    const QFileInfo info(filePath);

    if (!info.exists() || (!info.isFile() && !info.isDir()))
    {
        m_logger.warning(logTag(), QStringLiteral("Invalid vocabulary source: %1").arg(filePath));
        return;
    }

    const QString normalizedPath = QDir::cleanPath(info.absoluteFilePath());

    // Avoid adding the same source twice.
    for (const QString &existingPath : m_selectedSourcePaths)
    {
        if (existingPath.compare(normalizedPath, Qt::CaseInsensitive) == 0)
        {
            m_logger.debug(logTag(), QStringLiteral("Vocabulary source already added: %1").arg(normalizedPath));
            return;
        }
    }

    m_selectedSourcePaths.append(normalizedPath);

    QFileIconProvider iconProvider;
    const QIcon icon = info.isDir() ? iconProvider.icon(QFileIconProvider::Folder) : iconProvider.icon(QFileIconProvider::File);

    auto *item = new QListWidgetItem(icon, QStringLiteral("%1\n%2").arg(info.fileName(), normalizedPath), m_vocabSourceList);

    item->setData(Qt::UserRole, normalizedPath);
    item->setToolTip(normalizedPath);

    rebuildVocabSourcePaths();

    m_logger.debug(logTag(), QStringLiteral("Vocabulary source added: %1").arg(normalizedPath));
}

void MainView::removeSelectedVocabSource()
{
    const QList<QListWidgetItem *> selectedItems = m_vocabSourceList->selectedItems();

    if (selectedItems.isEmpty())
    {
        return;
    }

    // Collect selected row indices before removing anything.
    QList<int> rows;

    for (QListWidgetItem *item : selectedItems)
    {
        rows.append(m_vocabSourceList->row(item));
    }

    // Remove items from the bottom upwards to preserve row indices.
    std::sort(rows.begin(), rows.end(), std::greater<int>());

    for (const int row : rows)
    {
        const QString path = m_selectedSourcePaths.at(row);

        m_selectedSourcePaths.removeAt(row);
        delete m_vocabSourceList->takeItem(row);

        m_logger.debug(logTag(), QStringLiteral("Vocabulary source removed: %1").arg(path));
    }

    rebuildVocabSourcePaths();
}

QStringList MainView::findVocabFiles(const QString &path) const
{
    const QFileInfo info(path);

    if (info.isFile())
    {
        if (info.suffix().compare(
                QStringLiteral("txt"), Qt::CaseInsensitive) == 0)
        {
            return {info.absoluteFilePath()};
        }

        return {};
    }

    if (!info.isDir())
        return {};

    QStringList files;

    QDirIterator iterator(
        info.absoluteFilePath(),
        QStringList{QStringLiteral("*.txt")},
        QDir::Files,
        QDirIterator::Subdirectories);

    while (iterator.hasNext())
    {
        files.append(QFileInfo(iterator.next()).absoluteFilePath());
    }

    return files;
}

void MainView::rebuildVocabSourcePaths()
{
    QSet<QString> uniquePaths;

    for (const QString &sourcePath : m_selectedSourcePaths)
    {
        const QStringList files = findVocabFiles(sourcePath);

        for (const QString &filePath : files)
        {
            uniquePaths.insert(QDir::cleanPath(filePath));
        }
    }

    m_vocabSourcePaths = uniquePaths.values();
    m_vocabSourcePaths.sort(Qt::CaseInsensitive);

    initializeVocabLoad();
}
