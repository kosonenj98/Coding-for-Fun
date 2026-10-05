QT += testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += \
    tst_vocabfilehandler.cpp \
    ../../Application/VocabFileHandler.cpp \
    ../../Logging/logger.cpp

HEADERS += \
    ../../Application/VocabFileHandler.h \
    ../../Logging/logger.h
