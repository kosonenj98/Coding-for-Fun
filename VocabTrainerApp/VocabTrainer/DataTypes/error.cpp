#include "error.h"

QString Error::codeToQString(Code code)
{
    return QString::number(static_cast<int>(code));
}
