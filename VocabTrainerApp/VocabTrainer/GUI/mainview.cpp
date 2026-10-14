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
    auto *trainingPanel = createTrainingPanel();

    // Splitter is very flexible with adjusting panel sizes
    auto* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(trainingSettingsPanel);
    splitter->addWidget(trainingPanel);

    // Construct layout
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(splitter);

    // Create GUI component connections
    createConnections();

    m_logger.verbose(logTag(), QStringLiteral("Constructing main view done!"));
}

void MainView::showLobbyView()
{
    m_trainingStack->setCurrentWidget(m_lobbyView);
}

void MainView::showLoadingView()
{
    m_trainingStack->setCurrentWidget(m_loadingView);
}

void MainView::showErrorView(const QString &message)
{
    m_errorLabel->setText(message);
    m_trainingStack->setCurrentWidget(m_errorView);
}

void MainView::showTrainingView()
{
    m_trainingStack->setCurrentWidget(m_trainingView);
}

void MainView::updateView(const QList<Vocab::ContentInfo> &contentInfos)
{
    updateVocabTree(contentInfos);
}

void MainView::updateVocabTree(const QList<Vocab::ContentInfo> &contentInfos)
{
    removeObsoleteVocabs(contentInfos);

    int totalEntryCount = 0;
    for (const auto &info : contentInfos)
    {
        totalEntryCount += info.totalEntryCount;
        updateVocabItem(info);
    }

    updateVocabTreeLabel(totalEntryCount);
}

void MainView::removeObsoleteVocabs(const QList<Vocab::ContentInfo> &contentInfos)
{
    m_logger.debug(logTag(), QStringLiteral("Removing obsolete vocabs from vocab tree.."));

    auto *root = m_vocabTree->invisibleRootItem();
    for (int i = root->childCount() - 1; i >= 0; i--)
    {
        auto *item = root->child(i);
        const QString id = item->data(0, Qt::UserRole).toString();
        const bool exists = std::any_of(contentInfos.cbegin(), contentInfos.cend(), [&id](const Vocab::ContentInfo &info){return info.title == id;});

        if (!exists)
        {
            m_logger.debug(logTag(), QStringLiteral("Removing obsolete vocab '%1' from vocab tree.").arg(id));
            delete root->takeChild(i);
        }
    }

    m_logger.debug(logTag(), QStringLiteral("Removing obsolete vocabs from vocab tree done!"));
}

void MainView::updateVocabItem(const Vocab::ContentInfo &info)
{
    auto *root = m_vocabTree->invisibleRootItem();
    auto *vocabItem = findChildById(root, info.title);
    int column = 0;

    if (!vocabItem)
    {
        // Create new vocab item
        vocabItem = new QTreeWidgetItem(m_vocabTree);
        vocabItem->setData(column, Qt::UserRole, info.title);
        vocabItem->setFlags(vocabItem->flags() | Qt::ItemIsUserCheckable);
        vocabItem->setCheckState(0, Qt::Checked);
        vocabItem->setExpanded(m_vocabTree->topLevelItemCount() == 1);
    }

    vocabItem->setText(column, QStringLiteral("%1 (%2)").arg(info.title, QString::number(info.totalEntryCount)));
    updateVocabChildren(vocabItem, info);
}

void MainView::updateVocabChildren(QTreeWidgetItem *vocabItem, const Vocab::ContentInfo &info)
{
    QHash<QString, int> newChildrenContentInfo;

    if (info.entryCount > 0)
    {
        newChildrenContentInfo.insert(info.title, info.entryCount);
    }

    for (const auto &groupInfo : info.groupInfos)
    {
        newChildrenContentInfo.insert(groupInfo.title, groupInfo.entryCount);
    }

    // Remove obsolete children.
    for (int i = vocabItem->childCount() - 1; i >= 0; i--)
    {
        auto *child = vocabItem->child(i);
        const QString id = child->data(0, Qt::UserRole).toString();

        if (!newChildrenContentInfo.contains(id))
        {
            delete vocabItem->takeChild(i);
        }
    }

    // Update existing children and create new ones.
    for (auto it = newChildrenContentInfo.cbegin(); it != newChildrenContentInfo.cend(); ++it)
    {
        auto *child = getOrCreateChild(vocabItem, it.key());

        if (it.key() == info.title)
        {
            child->setText(0, QStringLiteral("Groupless (%1)").arg(it.value()));
        }
        else
        {
            child->setText(0, QStringLiteral("%1 (%2)").arg(it.key()).arg(it.value()));
        }
    }
}

void MainView::updateVocabTreeLabel(int totalEntryCount)
{
    if (totalEntryCount > 0)
    {
        m_vocabTreeLabel->setText(QStringLiteral("Contents (%1 entries)").arg(totalEntryCount));
    }
    else
    {
        m_vocabTreeLabel->setText(QStringLiteral("Contents"));
    }
}

QTreeWidgetItem *MainView::getOrCreateChild(QTreeWidgetItem *parent, const QString &id)
{
    auto *item = findChildById(parent, id);

    if (!item)
    {
        item = new QTreeWidgetItem(parent);
        item->setData(0, Qt::UserRole, id);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(0, Qt::Checked);
    }

    return item;
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
    case Training::Direction::KeyToValue:
        m_trainingDirection = Training::Direction::ValueToKey;
        m_switchDirectionArrowLabel->setText(QStringLiteral("←"));
        break;

    case Training::Direction::ValueToKey:
        m_trainingDirection = Training::Direction::KeyToValue;
        m_switchDirectionArrowLabel->setText(QStringLiteral("→"));
        break;
    }

    m_logger.debug(logTag(), QStringLiteral("Training direction switched."));
    initializeVocabChange();
}

void MainView::handleVocabTreeItemChanged(QTreeWidgetItem *item, int column)
{
    if (m_updatingVocabTree || column != 0)
    {
        return;
    }

    m_updatingVocabTree = true;

    // A parent selection applies to its entire subtree.
    if (item->childCount() > 0 &&
        item->checkState(0) != Qt::PartiallyChecked)
    {
        setSubtreeCheckState(item, item->checkState(0));
    }

    // Update all ancestors based on their children's states.
    auto *parent = item->parent();

    while (parent)
    {
        updateParentCheckState(parent);
        parent = parent->parent();
    }

    m_updatingVocabTree = false;
}

void MainView::initializeVocabChange()
{
    m_logger.debug(logTag(), QStringLiteral("Handling initializing vocab change..."));
    if (m_vocabSourcePaths.isEmpty())
    {
        // No vocab source selected
        m_logger.debug(logTag(), QStringLiteral("Requesting to clear vocab data..."));
        Vocab::ClearRequest clearRequest;
        emit requestVocabClear(clearRequest);
        return;
    }

    Vocab::LoadRequest loadRequest;
    loadRequest.direction = m_trainingDirection;
    for (const auto &path : m_vocabSourcePaths)
    {
        loadRequest.filePaths.append(path);
    }

    m_logger.verbose(logTag(), QStringLiteral("Requesting to load vocab..."));
    emit requestVocabLoad(loadRequest);
}

QWidget *MainView::createTrainingSettingsPanel()
{
    m_logger.verbose(logTag(), "Creating training settings panel...");

    auto *panel = new QWidget(this);

    auto *trainingSettingsGroup = createTrainingSettingsGroup(panel);
    m_startButton = new QPushButton(QStringLiteral("Start"), panel);

    auto *layout = new QVBoxLayout(panel);
    layout->addWidget(trainingSettingsGroup);
    layout->addWidget(m_startButton);

    m_logger.verbose(logTag(), "Creating training settings panel done!");

    return panel;
}

QWidget *MainView::createTrainingPanel()
{
    m_logger.verbose(logTag(), "Creating training panel...");

    auto *panel = new QWidget(this);

    // Create the stacked widget
    m_trainingStack = new QStackedWidget(panel);

    // Create the individual views
    m_lobbyView = createWelcomeView(m_trainingStack);
    m_loadingView = createLoadingView(m_trainingStack);
    m_errorView = createErrorView(m_trainingStack);
    m_trainingView = createTrainingView(m_trainingStack);

    // Add views to the stack
    m_trainingStack->addWidget(m_lobbyView);
    m_trainingStack->addWidget(m_loadingView);
    m_trainingStack->addWidget(m_errorView);
    m_trainingStack->addWidget(m_trainingView);

    // Construct the layout
    auto *layout = new QVBoxLayout(panel);
    layout->addWidget(m_trainingStack);

    m_logger.verbose(logTag(), "Creating training panel done!");

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
    m_vocabSourcesLabel = new QLabel(QStringLiteral("Sources:"), frame);
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
    layout->addWidget(m_vocabSourcesLabel);
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
    m_switchDirectionLabel = new QLabel(QStringLiteral("Direction:"), frame);
    m_switchDirectionKeyLabel = new QLabel(frame);
    m_switchDirectionArrowLabel = new QLabel(frame);
    m_switchDirectionValueLabel = new QLabel(frame);
    m_switchDirectionButton = new QPushButton(QStringLiteral("⇄"), frame);

    // Modify GUI component states
    m_switchDirectionKeyLabel->setAlignment(Qt::AlignCenter);
    m_switchDirectionValueLabel->setAlignment(Qt::AlignCenter);
    m_switchDirectionArrowLabel->setAlignment(Qt::AlignCenter);
    m_switchDirectionKeyLabel->setText(QStringLiteral("Question"));
    m_switchDirectionArrowLabel->setText(QStringLiteral("→"));
    m_switchDirectionValueLabel->setText(QStringLiteral("Answer"));

    QFont font;
    font.setBold(true);
    m_switchDirectionKeyLabel->setFont(font);
    m_switchDirectionArrowLabel->setFont(font);
    m_switchDirectionValueLabel->setFont(font);

    m_switchDirectionButton->setToolTip(QStringLiteral("Switch training direction"));

    // Construct layout
    auto *headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->addWidget(m_switchDirectionLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(m_switchDirectionButton);

    auto *directionLayout = new QHBoxLayout();
    directionLayout->addStretch();
    directionLayout->addWidget(m_switchDirectionKeyLabel);
    directionLayout->addWidget(m_switchDirectionArrowLabel);
    directionLayout->addWidget(m_switchDirectionValueLabel);
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
    m_vocabTreeLabel = new QLabel(QStringLiteral("Contents"), frame);
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

    auto *label = new QLabel(QStringLiteral("Loading vocabulary data..."), view);
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

    m_logger.verbose(logTag(), "Creating training panel done!");

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
    connect(m_vocabTree, &QTreeWidget::itemChanged, this, &MainView::handleVocabTreeItemChanged);

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
        if (info.suffix().compare(QStringLiteral("txt"), Qt::CaseInsensitive) == 0)
        {
            return {info.absoluteFilePath()};
        }

        m_logger.warning(logTag(), QStringLiteral("Skipping non-txt file '%1'...").arg(path));
        return {};
    }

    if (!info.isDir())
    {
        m_logger.warning(logTag(), QStringLiteral("Skipping invalid folder path '%1'...").arg(path));
        return {};
    }

    // Extract txt file paths inside given folder
    QStringList files;
    QDirIterator iterator(info.absoluteFilePath(), QStringList{QStringLiteral("*.txt")}, QDir::Files, QDirIterator::Subdirectories);
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

    initializeVocabChange();
}

void MainView::setSubtreeCheckState(QTreeWidgetItem *item, Qt::CheckState state)
{
    for (int i = 0; i < item->childCount(); i++)
    {
        auto *child = item->child(i);
        child->setCheckState(0, state);
        setSubtreeCheckState(child, state);
    }
}

void MainView::updateParentCheckState(QTreeWidgetItem *parent)
{
    if (!parent || parent->childCount() == 0)
    {
        return;
    }

    bool allChecked = true;
    bool allUnchecked = true;

    for (int i = 0; i < parent->childCount(); i++)
    {
        const auto state = parent->child(i)->checkState(0);

        allUnchecked &= state == Qt::Unchecked;
        allChecked &= state == Qt::Checked;

        if (!allChecked && !allUnchecked)
        {
            // Parent has both checked and unchecked children
            break;
        }
    }

    if (allChecked)
    {
        parent->setCheckState(0, Qt::Checked);
    }
    else if (allUnchecked)
    {
        parent->setCheckState(0, Qt::Unchecked);
    }
    else
    {
        parent->setCheckState(0, Qt::PartiallyChecked);
    }
}

QTreeWidgetItem *MainView::findChildById(QTreeWidgetItem *parent, const QString &id)
{
    for (int i = 0; i < parent->childCount(); ++i)
    {
        auto *child = parent->child(i);

        if (child->data(0, Qt::UserRole).toString() == id)
        {
            return child;
        }
    }

    return nullptr;
}
