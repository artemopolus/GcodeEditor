#-------------------------------------------------
#
# Project created by QtCreator 2017-12-16T00:08:11
#
#-------------------------------------------------
#QMAKE_EXTRA_TARGETS += before_build makefilehook

#makefilehook.target = $(MAKEFILE)
#makefilehook.depends = .beforebuild

#PRE_TARGETDEPS += .beforebuild

#before_build.target = .beforebuild
#before_build.depends = FORCE
#before_build.commands = chcp 1251

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = GCodeEditor
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp \
    gcodeanalyzator.cpp \
    json.cpp \
    captureparams.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h \
    gcodeanalyzator.h \
    json.h \
    captureparams.h

FORMS    += mainwindow.ui \
    captureparams.ui
