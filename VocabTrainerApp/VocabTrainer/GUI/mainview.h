#ifndef MAINVIEW_H
#define MAINVIEW_H

#include "Logging/logger.h"
#include "DataTypes/vocab.h"
#include "DataTypes/training.h"

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
#include <QVBoxLayout>

class MainView : public QWidget
{
    Q_OBJECT
public:
    explicit MainView(Logger &logger, QWidget *parent = nullptr);

    void showLobbyView();
    void showLoadingView();
    void showErrorView(const QString &message);
    void showTrainingView();

    void updateView(const QList<Vocab::ContentInfo> &contentInfos);
    void updateView(const Training::Question &question);
    void updateView(const Training::QuestionOrder &questionOrder);
    void updateView(bool nextQuestionAvailable);

public slots:

signals:
    void requestVocabLoad(const Vocab::LoadRequest &request);
    void requestVocabClear(const Vocab::ClearRequest &request);
    void requestStartTraining(const Training::StartRequest &request);
    void answer(const Training::Answer &answer);
    void requestChangeQuestion(const Training::ChangeQuestionRequest &request);

private slots:
    void handleBrowseVocabFiles();
    void handleBrowseVocabFolder();
    void handleSwitchTrainingDirection();
    void handleVocabTreeItemChanged(QTreeWidgetItem *item, int column);
    void handleStartTrainingPressed();
    void handleEnterPressed();
    void handleShowAnswerDetailsPressed(bool checked);
    void handlePreviousQuestionPressed();
    void handleNextQuestionPressed();

private:
    void initializeVocabChange();
    void updateVocabTree(const QList<Vocab::ContentInfo> &contentInfos);
    void removeObsoleteVocabs(const QList<Vocab::ContentInfo> &contentInfos);
    void updateVocabItem(const Vocab::ContentInfo &info);
    void updateVocabChildren(QTreeWidgetItem *vocabItem, const Vocab::ContentInfo &info);
    void updateVocabTreeLabel(int totalEntryCount);
    QTreeWidgetItem *getOrCreateChild(QTreeWidgetItem *parent, const QString &id);
    void addVocabSource(const QString &filePath);
    void removeSelectedVocabSource();
    QStringList findVocabFiles(const QString &path) const;
    void rebuildVocabSourcePaths();
    void setSubtreeCheckState(QTreeWidgetItem *item, Qt::CheckState state);
    void updateParentCheckState(QTreeWidgetItem *parent);
    QTreeWidgetItem *findChildById(QTreeWidgetItem *parent, const QString &id);
    void setFeedback(bool isCorrect, const QString &answer, const QSet<QString> &correctAnswers);
    void handleChangeQuestion(const Training::ChangeQuestion change);

    Logger &m_logger;
    QStringList m_selectedSourcePaths;
    QStringList m_vocabSourcePaths;
    Vocab::Direction m_trainingDirection = Vocab::Direction::KeyToValue;
    bool m_updatingVocabTree = false;

    /*-----------------------------------------------------*/

    // Training settings panel
    QWidget *createTrainingSettingsPanel();

    // Training settings group
    QGroupBox *createTrainingSettingsGroup(QWidget *parent);
    QFrame *createVocabSourceFrame(QWidget *parent);
    QFrame *createTrainingDirectionFrame(QWidget *parent);
    QFrame *createVocabTreeFrame(QWidget *parent);
    QTreeWidget *createVocabularyTree(QWidget *parent);

    /*-----------------------------------------------------*/

    // Training panel
    QWidget *createTrainingPanel();

    // Training widgets
    QWidget *createNavigationBar(QWidget *parent);
    QLabel *createQuestionLabel(QWidget *parent);
    QLineEdit *createAnswerInput(QWidget *parent);
    QWidget *createAnswerResult(QWidget *parent);
    QProgressBar *createProgressBar(QWidget *parent);

    /*-----------------------------------------------------*/

    // Views
    QWidget *createWelcomeView(QWidget *parent);
    QWidget *createLoadingView(QWidget *parent);
    QWidget *createErrorView(QWidget *parent);
    QWidget *createTrainingView(QWidget *parent);

    /*-----------------------------------------------------*/

    // GUI component connections
    void createConnections();

    /*-----------------------------------------------------*/

    // Stacked view for different situations
    QStackedWidget *m_trainingStack;
    QWidget *m_lobbyView;
    QWidget *m_loadingView;
    QWidget *m_errorView;
    QLabel *m_errorLabel;
    QWidget *m_trainingView;

    /*-----------------------------------------------------*/

    // Training settings attributes

    // Vocab file selection
    QLabel *m_vocabSourcesLabel;
    QPushButton *m_addFilesButton;
    QPushButton *m_addFoldersButton;
    QListWidget *m_vocabSourceList;
    QPushButton *m_removeSourceButton;

    // Training direction
    QLabel *m_switchDirectionLabel;
    QLabel *m_switchDirectionKeyLabel;
    QLabel *m_switchDirectionArrowLabel;
    QLabel *m_switchDirectionValueLabel;
    QPushButton *m_switchDirectionButton;

    // Vocab tree
    QLabel *m_vocabTreeLabel;
    QTreeWidget *m_vocabTree;

    // Start training
    QPushButton *m_startButton;

    /*-----------------------------------------------------*/

    // Training attributes

    // Navigation bar
    QPushButton *m_previousButton;
    QLabel *m_roundLabel;
    QLabel *m_cardCounterLabel;
    QPushButton *m_nextButton;

    // Question and answer
    QLabel *m_questionLabel;
    QLineEdit *m_answerLineEdit;
    QLabel *m_answerStatusLabel;
    QPushButton *m_answerDetailsButton;
    QWidget *m_answerDetailsWidget;
    QVBoxLayout *m_answerDetailsLayout;
    QWidget *m_answerResult;
    QLabel *m_feedbackLabel;

    // Progress bar
    QProgressBar *m_progressBar;
};

#endif // MAINVIEW_H
