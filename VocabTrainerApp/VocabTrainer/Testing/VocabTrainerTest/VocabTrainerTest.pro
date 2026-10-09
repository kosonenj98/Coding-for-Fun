QT += testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

INCLUDEPATH += ../..

SOURCES += \
    tst_vocabfilehandler.cpp \
    ../../Application/vocabfilehandler.cpp \
    ../../Logging/logger.cpp \
    ../../Application/errorcode.cpp

HEADERS += \
    ../../DataTypes/datatypes.h \
    ../../DataTypes/requests.h \
    ../../DataTypes/results.h \
    ../../Application/vocabfilehandler.h \
    ../../Logging/logger.h \
    ../../Application/errorcode.h
