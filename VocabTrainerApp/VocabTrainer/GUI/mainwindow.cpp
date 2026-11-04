#include "mainwindow.h"

#include <QCloseEvent>

namespace
{
    const QString &logTag()
    {
        static const QString tag = QStringLiteral("MainWindow");
        return tag;
    }
}

MainWindow::MainWindow(Logger &logger, SettingsHandler &handler, QWidget *parent)
    : QMainWindow{parent}, m_logger(logger), m_settingsHandler(handler)
{
    m_logger.verbose(logTag(), QStringLiteral("Constructing main window..."));

    setWindowTitle(QStringLiteral("VocabTrainer"));

    // Initialize dialog service
    initializeDialogService();

    // Initialize views
    initializeMainView();
    initializeSettingsView();
    initializeLogView();

    // Initialize tab widget
    initializeTabWidget();

    // Set initial view
    m_mainView->showIdleView();
    m_tabWidget->setCurrentWidget(m_mainView);
    m_trainingState = ApplicationState::Idle;

    m_logger.verbose(logTag(), QStringLiteral("Constructing main window done!"));
}

void MainWindow::handleInitializeStatisticsDatabaseFinished(const Statistics::InitializeDatabaseResult &result)
{
    m_logger.debug(logTag(), QStringLiteral("Handling statistics database finished..."));
    if (result.code != Error::Code::Success)
    {
        m_logger.error(logTag(), QStringLiteral("Failed to initialize statistics database! ErrorCode %1").arg(Error::codeToQString(result.code)));
        m_dialogService->showError(QStringLiteral("Statistics database"), QStringLiteral("Failed to initialize statistics database! ErrorCode %1").arg(Error::codeToQString(result.code)));
        // TODO: Retry or something else?
        return;
    }
    m_logger.debug(logTag(), QStringLiteral("Handling statistics database finished done!"));
}

void MainWindow::handleLogQueryRequest(const Log::QueryRequest &request)
{
    m_logger.verbose(logTag(), QStringLiteral("Emitting log query request..."));
    m_trainingState = ApplicationState::QueryingLog;
    emit requestLogQuery(request);
}

void MainWindow::handleLogQueryFinished(const Log::QueryResult &result)
{
    m_logger.verbose(logTag(), QStringLiteral("Handling log query finished..."));
    m_trainingState = ApplicationState::Idle;

    if (result.code != Error::Code::Success)
    {
        m_logger.verbose(logTag(), QStringLiteral("Informing user about failed log query..."));
        m_dialogService->showError(QStringLiteral("Log Query"), QStringLiteral("Log query failed: (%1)").arg(Error::codeToQString(result.code)));
        m_logger.verbose(logTag(), QStringLiteral("Informing user about failed log query done!"));
        return;
    }

    if (result.failedEntryCount > 0)
    {
        m_dialogService->showWarning(QStringLiteral("Log Query"), QStringLiteral("Skipped %1 faulty entries in selected log file.").arg(QString::number(result.failedEntryCount)));
    }

    m_logger.verbose(logTag(), QStringLiteral("Updating log view..."));
    m_logView->updateView(result.entries);
    m_logger.verbose(logTag(), QStringLiteral("Updating log view done!"));

    m_logger.verbose(logTag(), QStringLiteral("Handling log query finished done!"));
}

void MainWindow::handleApplyNewSettingsRequest(const Settings::ApplyNewRequest &request)
{
    m_logger.verbose(logTag(), QStringLiteral("Emitting request to apply the new settings..."));
    m_trainingState = ApplicationState::ApplyingNewSettings;
    emit requestApplyNewSettings(request);
}

void MainWindow::handleApplyNewSettingsFinished(const Settings::ApplyNewResult &result)
{
    m_logger.verbose(logTag(), QStringLiteral("Handling applying new settings finished..."));
    m_trainingState = ApplicationState::Idle;

    if (result.code != Error::Code::Success)
    {
        m_dialogService->showError(QStringLiteral("Settings change"), QStringLiteral("Applying new settings failed: (%1)").arg(Error::codeToQString(result.code)));
        m_settingsView->updateView(result.settings);
        return;
    }

    m_dialogService->showInformation(QStringLiteral("Settings change"), QStringLiteral("Settings saved successfully."));
    m_logger.verbose(logTag(), QStringLiteral("Handling applying new settings finished done!"));
}

void MainWindow::handleLoadVocabRequest(const Vocab::LoadRequest &request)
{
    m_logger.verbose(logTag(), QStringLiteral("Emitting request to load vocabs..."));
    m_trainingState = ApplicationState::LoadingVocabs;
    m_mainView->showLoadingView();
    emit requestLoadVocab(request);
}

void MainWindow::handleLoadVocabFinished(const Vocab::LoadResult &result)
{
    m_logger.verbose(logTag(), QStringLiteral("Handling load vocab datas finished..."));
    m_trainingState = ApplicationState::Idle;

    if (result.code != Error::Code::Success)
    {
        m_mainView->showErrorView(QStringLiteral("Failed to load the requested vocabs! ErrorCode %1").arg(Error::codeToQString(result.code)));
        m_dialogService->showError(QStringLiteral("Load vocab"), QStringLiteral("Loading selected vocabs failed: (%1)").arg(Error::codeToQString(result.code)));
        return;
    }

    if (result.failedVocabCount > 0 || result.partiallySucceededVocabCount > 0 || result.failedEntryCount > 0)
    {
        m_dialogService->showWarning(QStringLiteral("Load vocab"), QStringLiteral("Failed to load %1 vocabs and %2 faulty entries from %3 vocabs.").arg(QString::number(result.failedVocabCount), QString::number(result.failedEntryCount), QString::number(result.partiallySucceededVocabCount)));
    }

    m_mainView->updateView(result.contentInfos);
    m_mainView->showIdleView();
    m_logger.verbose(logTag(), QStringLiteral("Handling load vocab datas finished done!"));
}

void MainWindow::handleClearVocabRequest(const Vocab::ClearRequest &request)
{
    m_logger.debug(logTag(), QStringLiteral("Handling vocab clear request..."));
    m_trainingState = ApplicationState::ClearingVocabs;
    emit requestClearVocab(request);
}

void MainWindow::handleClearVocabFinished(const Vocab::ClearResult &result)
{
    m_logger.debug(logTag(), QStringLiteral("Log clear finished!"));
    m_trainingState = ApplicationState::Idle;

    if (result.code != Error::Code::Success)
    {
        m_logger.error(logTag(), QStringLiteral("Failed to clear log! ErrorCode %1").arg(Error::codeToQString(result.code)));
        m_dialogService->showError(QStringLiteral("Vocab clear"), QStringLiteral("Failed to clear log (%1).").arg(Error::codeToQString(result.code)));
        return;
    }

    m_mainView->updateView(QList<Vocab::ContentInfo>{});
    m_mainView->showIdleView();
}

void MainWindow::handleStartTrainingRequest(const Training::StartRequest &request)
{
    m_trainingState = ApplicationState::Training;
    m_mainView->showTrainingView();
    emit requestStartTraining(request);
}

void MainWindow::handleQuestion(const Training::Question &question)
{
    m_mainView->updateView(question);
}

void MainWindow::handleAnswer(const Training::Answer &answer)
{
    emit MainWindow::answer(answer);
}

void MainWindow::handleChangeQuestionRequest(const Training::ChangeQuestionRequest &request)
{
    emit requestChangeQuestion(request);
}

void MainWindow::handleChangeQuestionFinished(const Training::ChangeQuestionResult &result)
{
    if (result.code != Error::Code::Success)
    {
        m_logger.error(logTag(), QStringLiteral("Failed to change question! ErrorCode %1").arg(Error::codeToQString(result.code)));
        m_dialogService->showError(QStringLiteral("Change Question"), QStringLiteral("Failed to change question: %1").arg(Error::codeToQString(result.code)));
        return;
    }
    m_mainView->updateView(result.order);
}

void MainWindow::handleNextQuestionAvailable(bool available)
{
    m_mainView->updateView(available);
}

void MainWindow::handleTrainingFinished(const Training::FinishedResult &result)
{
    if (result.code != Error::Code::Success)
    {
        m_logger.error(logTag(), QStringLiteral("Failure in training! ErrorCode %1").arg(Error::codeToQString(result.code)));
        m_mainView->showErrorView(QStringLiteral("Failure in training! ErrorCode %1").arg(Error::codeToQString(result.code)));
        m_dialogService->showError(QStringLiteral("Training"), QStringLiteral("Failure in training! ErrorCode %1").arg(Error::codeToQString(result.code)));
        m_mainView->showTrainingView();
        return;
    }

    bool continueTraining = m_dialogService->askYesOrNo(QStringLiteral("Training"), QStringLiteral("Training finished!\nContinue training?"));

    if (continueTraining)
    {
        Training::ContinueRequest request;
        emit requestContinueTraining(request);
    }
    else
    {
        Training::StopRequest request;
        emit requestStopTraining(request);
    }
}

void MainWindow::handleStopTrainingRequested(const Training::StopRequest &request)
{
    if (m_trainingState == ApplicationState::StoppingTraining)
    {
        return;
    }

    m_trainingState = ApplicationState::StoppingTraining;
    emit requestStopTraining(request);
}

void MainWindow::handleStopTrainingFinished(const Training::StopResult &result)
{
    if (result.code != Error::Code::Success)
    {
        switch (result.reason)
        {
        case Training::StopFailureReason::None:
        {
            m_logger.error(logTag(), QStringLiteral("Failed to stop training without evident reason! ErrorCode %1").arg(Error::codeToQString(result.code)));
            const auto userStopResult = m_dialogService->askYesOrNoOrCancel(QStringLiteral("Training"), QStringLiteral("Stopping training failed unexpectedly. ReasonCode %1.\nDo you want to retry?").arg(Error::codeToQString(result.code)), QMessageBox::Critical);

            if (userStopResult == QMessageBox::Cancel)
            {
                m_trainingState = ApplicationState::Training;
                return;
            }

            if (userStopResult == QMessageBox::Yes)
            {
                Training::StopRequest request;
                request.command = Training::StopCommand::Stop;
                emit requestStopTraining(request);
                return;
            }

            if (userStopResult == QMessageBox::No)
            {
                Training::StopRequest request;
                request.command = Training::StopCommand::ForcedStop;
                emit requestStopTraining(request);
                return;
            }

            break;
        }

        case Training::StopFailureReason::UnsavedData:
        {
            const auto userStopResult = m_dialogService->askYesOrNoOrCancel(QStringLiteral("Training"), QStringLiteral("You have unsaved training data.\nDo you want to save and stop?"), QMessageBox::Warning);

            if (userStopResult == QMessageBox::Cancel)
            {
                m_trainingState = ApplicationState::Training;
                return;
            }

            if (userStopResult == QMessageBox::Yes)
            {
                Training::StopRequest request;
                request.command = Training::StopCommand::SaveAndStop;
                emit requestStopTraining(request);
                return;
            }

            if (userStopResult == QMessageBox::No)
            {
                Training::StopRequest request;
                request.command = Training::StopCommand::OnlyStop;
                emit requestStopTraining(request);
                return;
            }

            break;
        }

        case Training::StopFailureReason::SaveFailed:
        {
            const auto userStopResult = m_dialogService->askYesOrNoOrCancel(QStringLiteral("Training"), QStringLiteral("Failed to save training data. ErrorCode %1.\nDo you want to retry?").arg(Error::codeToQString(result.code)), QMessageBox::Critical);

            if (userStopResult == QMessageBox::Cancel)
            {
                m_trainingState = ApplicationState::Training;
                return;
            }

            if (userStopResult == QMessageBox::Yes)
            {
                Training::StopRequest request;
                request.command = Training::StopCommand::SaveAndStop;
                emit requestStopTraining(request);
                return;
            }

            if (userStopResult == QMessageBox::No)
            {
                Training::StopRequest request;
                request.command = Training::StopCommand::OnlyStop;
                emit requestStopTraining(request);
                return;
            }

            break;
        }

        case Training::StopFailureReason::StopFailed:
        {
            const auto userStopResult = m_dialogService->askYesOrNoOrCancel(QStringLiteral("Training"), QStringLiteral("Failed to stop training. ErrorCode %1.\nDo you want to retry?").arg(Error::codeToQString(result.code)), QMessageBox::Critical);

            if (userStopResult == QMessageBox::Cancel)
            {
                m_trainingState = ApplicationState::Training;
                return;
            }

            if (userStopResult == QMessageBox::Yes)
            {
                Training::StopRequest request;
                request.command = Training::StopCommand::Stop;
                emit requestStopTraining(request);
                return;
            }

            if (userStopResult == QMessageBox::No)
            {
                Training::StopRequest request;
                request.command = Training::StopCommand::ForcedStop;
                emit requestStopTraining(request);
                return;
            }

            break;
        }
        }

        return;
    }

    m_dialogService->showInformation(QStringLiteral("Training"), QStringLiteral("Training stopped successfully!"));

    if (m_trainingState != ApplicationState::StoppingTrainingForClose)
    {
        m_trainingState = ApplicationState::Idle;
    }
    else
    {
        close();
        return;
    }

    m_mainView->updateView(result.sessionData);
    m_mainView->showTrainingSummaryView();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_trainingState != ApplicationState::Training) {
        event->accept();
        return;
    }

    event->ignore();
    m_trainingState = ApplicationState::StoppingTrainingForClose;

    Training::StopRequest request;
    emit requestStopTraining(request);
}

void MainWindow::handleViewTabChanged(int index)
{
    m_logger.verbose(logTag(), QStringLiteral("Handling viewTabChanged..."));
    if (index == m_tabWidget->indexOf(m_logView)) {
        m_logView->updateView();
    }
    m_logger.verbose(logTag(), QStringLiteral("Handling viewTabChanged done!"));
}

void MainWindow::initializeDialogService()
{
    m_logger.verbose(logTag(), QStringLiteral("Initializing dialog service..."));
    m_dialogService = new DialogService(this, this);
    m_logger.verbose(logTag(), QStringLiteral("Initializing dialog service done!"));
}

void MainWindow::initializeMainView()
{
    m_logger.verbose(logTag(), QStringLiteral("Initializing main view..."));
    m_mainView = new MainView(m_logger, this);
    connect(m_mainView, &MainView::requestVocabLoad, this, &MainWindow::handleLoadVocabRequest);
    connect(m_mainView, &MainView::requestVocabClear, this, &MainWindow::handleClearVocabRequest);
    connect(m_mainView, &MainView::requestStartTraining, this, &MainWindow::handleStartTrainingRequest);
    connect(m_mainView, &MainView::answer, this, &MainWindow::handleAnswer);
    connect(m_mainView, &MainView::requestChangeQuestion, this, &MainWindow::handleChangeQuestionRequest);
    connect(m_mainView, &MainView::requestStopTraining, this, &MainWindow::handleStopTrainingRequested);
    m_logger.verbose(logTag(), QStringLiteral("Initializing main view done!"));
}

void MainWindow::initializeSettingsView()
{
    m_logger.verbose(logTag(), QStringLiteral("Initializing settings view..."));
    m_settingsView = new SettingsView(m_logger, m_settingsHandler.settings(), this);
    connect(m_settingsView, &::SettingsView::requestApplyNewSettings, this, &MainWindow::handleApplyNewSettingsRequest);
    m_logger.verbose(logTag(), QStringLiteral("Initializing settings view done!"));
}

void MainWindow::initializeLogView()
{
    m_logger.verbose(logTag(), QStringLiteral("Initializing log view..."));
    m_logView = new LogView(m_logger, m_settingsHandler.settings(), this);
    connect(m_logView, &LogView::requestLogQuery, this, &MainWindow::handleLogQueryRequest);
    m_logger.verbose(logTag(), QStringLiteral("Initializing log view done!"));
}

void MainWindow::initializeTabWidget()
{
    m_logger.verbose(logTag(), QStringLiteral("Initializing tab widget..."));
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->addTab(m_mainView, QStringLiteral("Main View"));
    m_tabWidget->addTab(m_settingsView, QStringLiteral("Settings View"));
    m_tabWidget->addTab(m_logView, QStringLiteral("Log View"));
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &MainWindow::handleViewTabChanged);
    setCentralWidget(m_tabWidget);
    m_logger.verbose(logTag(), QStringLiteral("Initializing tab widget done!"));
}
