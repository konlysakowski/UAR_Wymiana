QT       += core gui
QT       += core network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    NetworkClient.cpp \
    NetworkServer.cpp \
    arx.cpp \
    dialog.cpp \
    main.cpp \
    mainwindow.cpp \
    pid.cpp \
    qcustomplot.cpp \
    symulacja.cpp \
    wartosc_zadana.cpp

HEADERS += \
    NetworkClient.h \
    NetworkServer.h \
    arx.h \
    dialog.h \
    klasy.h \
    mainwindow.h \
    pid.h \
    qcustomplot.h \
    symulacja.h \
    typ_sygnalu.h \
    wartosc_zadana.h

FORMS += \
    dialog.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
