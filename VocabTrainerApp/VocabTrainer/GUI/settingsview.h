#ifndef SETTINGSVIEW_H
#define SETTINGSVIEW_H

#include "../Logging/logger.h"
#include "../Application/settingshandler.h"

#include <QWidget>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>

class SettingsView : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsView(Logger &logger, const Settings &settings, QWidget *parent = nullptr);

    void updateView(const Settings &settings);

public slots:
    void createRequestForNewSettingsAndEmit();

signals:
    void requestApplyNewSettings(const Settings &newSettings);

private:
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
