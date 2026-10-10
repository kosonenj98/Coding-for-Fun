QT += testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

INCLUDEPATH += ../../

SOURCES += \
    main.cpp \
    tst_vocabfilehandler.cpp \
    tst_vocabgenerator.cpp \
    ../../Logging/logger.cpp \
    ../../Application/errorcode.cpp \
    ../../Application/vocabfilehandler.cpp \
    ../../Application/vocabgenerator.cpp

HEADERS += \
    tst_vocabfilehandler.h \
    tst_vocabgenerator.h \
    ../../DataTypes/datatypes.h \
    ../../DataTypes/requests.h \
    ../../DataTypes/results.h \
    ../../Logging/logger.h \
    ../../Application/errorcode.h \
    ../../Application/vocabfilehandler.h \
    ../../Application/vocabgenerator.h
