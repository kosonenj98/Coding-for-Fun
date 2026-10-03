#include "errorcode.h"

QString errorCodeToString(ErrorCode code)
{
    return QString::number(static_cast<int>(code));
}
