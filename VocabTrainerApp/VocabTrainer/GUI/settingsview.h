#ifndef SETTINGSVIEW_H
#define SETTINGSVIEW_H

#include "../Logging/logger.h"
#include "../settingshandler.h"

#include <QWidget>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>

class SettingsView : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsView(Logger &logger, const Settings &settings, QWidget *parent = nullptr);

public slots:
    void formChangeSettingsRequest();

    void settingsChangeResponded(const Settings &settings);

signals:
    void settingsChangeRequested(const Settings &newSettings);

private:
    void updateView(const Settings &settings);

    Logger &m_logger;

    QCheckBox* m_logCheckBox;
    QCheckBox* m_logInfoCheckBox;
    QCheckBox* m_logWarningCheckBox;
    QCheckBox* m_logErrorCheckBox;
    QCheckBox* m_logDebugCheckBox;
    QCheckBox* m_logVerboseCheckBox;

    QLineEdit* m_logFilePathEdit;

    QPushButton* m_applyButton;
};

#endif // SETTINGSVIEW_H
