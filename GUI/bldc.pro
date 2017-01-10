QT += widgets serialport
greaterThan(QT_MAJOR_VERSION, 4): QT += printsupport
TARGET = bldc
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    settingsdialog.cpp \
    qcustomplot.cpp

HEADERS += \
    mainwindow.h \
    settingsdialog.h \
    commands.h \
    qcustomplot.h

FORMS += \
    mainwindow.ui \
    settingsdialog.ui

RESOURCES += \
    bldc.qrc
