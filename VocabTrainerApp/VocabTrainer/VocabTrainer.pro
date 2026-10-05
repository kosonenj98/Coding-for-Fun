QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Application/errorcode.cpp \
    Application/logservice.cpp \
    Application/settingshandler.cpp \
    Application/vocabfilehandler.cpp \
    Application/vocabtrainer.cpp \
    GUI/dialogservice.cpp \
    GUI/logview.cpp \
    GUI/mainview.cpp \
    GUI/mainwindow.cpp \
    GUI/settingsview.cpp \
    Logging/logentryformatter.cpp \
    Logging/logfileHandler.cpp \
    Logging/logger.cpp \
    Logging/loglevel.cpp \
    main.cpp \

HEADERS += \
    Application/errorcode.h \
    Application/logservice.h \
    Application/settingshandler.h \
    Application/vocabfiledatatypes.h \
    Application/vocabfilehandler.h \
    Application/vocabtrainer.h \
    GUI/dialogservice.h \
    GUI/logview.h \
    GUI/mainview.h \
    GUI/mainwindow.h \
    GUI/settingsview.h \
    Logging/logdatatypes.h \
    Logging/logentryformatter.h \
    Logging/logfileHandler.h \
    Logging/logger.h \
    Logging/loglevel.h \

TRANSLATIONS += \


CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
