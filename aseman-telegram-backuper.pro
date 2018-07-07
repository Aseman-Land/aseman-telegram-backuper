#-------------------------------------------------
#
# Project created by QtCreator 2018-06-26T15:59:20
#
#-------------------------------------------------

QT += core gui telegram widgets network
ios|osx: QMAKE_CXXFLAGS += -Wno-narrowing

TARGET = aseman-tg-backuper
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui \
    proxy.ui \
    about.ui \
    dialog.ui

RESOURCES += \
    resource.qrc
