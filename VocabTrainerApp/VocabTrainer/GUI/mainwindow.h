#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Logging/logger.h"
#include "DataTypes/log.h"
#include "DataTypes/settings.h"
#include "DataTypes/training.h"
#include "DataTypes/statistics.h"
#include "Application/settingshandler.h"
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
    void handleInitializeStatisticsDatabaseFinished(const Statistics::InitializeDatabaseResult &result);

    void handleLogQueryRequest(const Log::QueryRequest &request);
    void handleLogQueryFinished(const Log::QueryResult &result);

    void handleApplyNewSettingsRequest(const Settings::ApplyNewRequest &request);
    void handleApplyNewSettingsFinished(const Settings::ApplyNewResult &result);

    void handleLoadVocabRequest(const Vocab::LoadRequest &request);
    void handleLoadVocabFinished(const Vocab::LoadResult &result);

    void handleClearVocabRequest(const Vocab::ClearRequest &request);
    void handleClearVocabFinished(const Vocab::ClearResult &result);

    void handleStartTrainingRequest(const Training::StartRequest &request);
    void handleQuestion(const Training::Question &question);
    void handleAnswer(const Training::Answer &answer);
    void handleChangeQuestionRequest(const Training::ChangeQuestionRequest &request);
    void handleChangeQuestionFinished(const Training::ChangeQuestionResult &result);
    void handleNextQuestionAvailable(bool available);
    void handleTrainingFinished(const Training::FinishedResult &result);
    void handleStopTrainingRequested(const Training::StopRequest &request);
    void handleStopTrainingFinished(const Training::StopResult &result);

signals:
    void requestLogQuery(const Log::QueryRequest &query);
    void requestApplyNewSettings(const Settings::ApplyNewRequest &request);
    void requestLoadVocab(const Vocab::LoadRequest &request);
    void requestClearVocab(const Vocab::ClearRequest &request);
    void requestStartTraining(const Training::StartRequest &request);
    void answer(const Training::Answer &answer);
    void requestChangeQuestion(const Training::ChangeQuestionRequest &request);
    void requestContinueTraining(const Training::ContinueRequest &request);
    void requestStopTraining(const Training::StopRequest &request);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void handleViewTabChanged(int index);

private:
    void initializeDialogService();
    void initializeMainView();
    void initializeSettingsView();
    void initializeLogView();
    void initializeTabWidget();

    enum class ApplicationState
    {
        Idle,
        LoadingVocabs,
        ClearingVocabs,
        Training,
        StoppingTraining,
        StoppingTrainingForClose,
        QueryingLog,
        ApplyingNewSettings
    };

    Logger &m_logger;
    SettingsHandler &m_settingsHandler;
    DialogService* m_dialogService = nullptr;
    ApplicationState m_trainingState;

    // Views
    MainView *m_mainView = nullptr;
    SettingsView *m_settingsView = nullptr;
    LogView *m_logView = nullptr;

    // GUI components
    QTabWidget *m_tabWidget;
};

#endif // MAINWINDOW_H
