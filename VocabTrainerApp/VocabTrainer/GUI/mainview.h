#ifndef MAINVIEW_H
#define MAINVIEW_H

#include "Logging/logger.h"
#include "DataTypes/requests.h"

#include <QWidget>
#include <QGroupBox>
#include <QTreeWidget>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QRadioButton>
#include <QPushButton>
#include <QStackedWidget>
#include <QListWidget>

class MainView : public QWidget
{
    Q_OBJECT
public:
    explicit MainView(Logger &logger, QWidget *parent = nullptr);

    void createInitialView();
    void updateVocabTree(const QList<Vocab::Info> vocabInfos);

public slots:
    void browseVocabFiles();
    void browseVocabFolder();
    void switchTrainingDirection();


signals:
    void requestVocabLoad(const LoadVocabsRequest &request);

private:
    void initializeVocabLoad();

    Logger &m_logger;

    QWidget *createTrainingSettingsPanel();
    QWidget *createDisplayPanel();

    QGroupBox *createTrainingSettingsGroup(QWidget *parent);

    QFrame *createVocabSourceFrame(QWidget *parent);
    QFrame *createTrainingDirectionFrame(QWidget *parent);
    QFrame *createVocabTreeFrame(QWidget *parent);

    QTreeWidget *createVocabularyTree(QWidget *parent);

    QWidget *createWelcomeView(QWidget *parent);
    QWidget *createLoadingView(QWidget *parent);
    QWidget *createErrorView(QWidget *parent);
    QWidget *createTrainingView(QWidget *parent);

    QWidget *createNavigationBar(QWidget *parent);
    QLabel *createQuestionLabel(QWidget *parent);
    QLineEdit *createAnswerInput(QWidget *parent);
    QWidget *createAnswerResult(QWidget *parent);
    QProgressBar *createProgressBar(QWidget *parent);

    void createConnections();



    QLabel *m_vocabLabel;

    QListWidget *m_vocabSourceList;
    QStringList m_selectedSourcePaths;
    QStringList m_vocabSourcePaths;
    QPushButton *m_addFilesButton;
    QPushButton *m_addFoldersButton;
    QPushButton *m_removeSourceButton;

    // Vocabulary source methods

    void addVocabSource(const QString &filePath);
    void removeSelectedVocabSource();

    QStringList findVocabFiles(const QString &path) const;
    void rebuildVocabSourcePaths();

    Training::Direction m_trainingDirection = Training::Direction::Forward;
    QLabel *m_switchDirectionLabel;
    QLabel *m_leftFlagLabel;
    QLabel *m_rightFlagLabel;
    QPushButton *m_switchDirectionButton;

    QLabel *m_vocabTreeLabel;
    QTreeWidget *m_vocabTree;

    QPushButton *m_startButton;

    QStackedWidget *m_displayStack;

    QWidget *m_welcomeView;
    QWidget *m_loadingView;
    QWidget *m_errorView;
    QWidget *m_trainingView;

    QLabel *m_errorLabel;


    QPushButton *m_previousButton;
    QLabel *m_roundLabel;
    QLabel *m_cardCounterLabel;
    QPushButton *m_nextButton;


    QLabel *m_questionLabel;
    QLineEdit *m_answerLineEdit;
    QLabel *m_answerStatusLabel;
    QWidget *m_answerResult;
    QLabel *m_feedbackLabel;
    QProgressBar *m_progressBar;
};

#endif // MAINVIEW_H
