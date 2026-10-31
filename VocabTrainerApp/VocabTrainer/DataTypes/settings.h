#ifndef SETTINGS_H
#define SETTINGS_H

#include "error.h"

namespace Settings
{
    struct Settings
    {
        bool logEnabled = true;
        bool logInfoEnabled = true;
        bool logWarningEnabled = true;
        bool logErrorEnabled = true;
        bool logDebugEnabled = false;
        bool logVerboseEnabled = false;
        QString logFilePath;
    };

    struct ApplyNewRequest
    {
        Settings settings;
    };

    struct ApplyNewResult
    {
        Error::Code code = Error::Code::Success;
        Settings settings;
    };
}

#endif // SETTINGS_H
