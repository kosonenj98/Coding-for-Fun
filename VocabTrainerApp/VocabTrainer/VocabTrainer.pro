QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Application/dataservice.cpp \
    Application/logservice.cpp \
    Application/settingshandler.cpp \
    Application/vocabfilehandler.cpp \
    Application/vocabgenerator.cpp \
    Application/vocabtrainer.cpp \
    DataTypes/error.cpp \
    DataTypes/log.cpp \
    GUI/dialogservice.cpp \
    GUI/logview.cpp \
    GUI/mainview.cpp \
    GUI/mainwindow.cpp \
    GUI/settingsview.cpp \
    Logging/logentryformatter.cpp \
    Logging/logfilehandler.cpp \
    Logging/logger.cpp \
    main.cpp \

HEADERS += \
    Application/dataservice.h \
    Application/logservice.h \
    Application/settingshandler.h \
    Application/vocabfilehandler.h \
    Application/vocabgenerator.h \
    Application/vocabtrainer.h \
    DataTypes/error.h \
    DataTypes/log.h \
    DataTypes/settings.h \
    DataTypes/training.h \
    DataTypes/vocab.h \
    DataTypes/vocabfile.h \
    DataTypes/vocabgeneration.h \
    GUI/dialogservice.h \
    GUI/logview.h \
    GUI/mainview.h \
    GUI/mainwindow.h \
    GUI/settingsview.h \
    Logging/logentryformatter.h \
    Logging/logfilehandler.h \
    Logging/logger.h \

TRANSLATIONS += \


CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
