QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES -= UNICODE
DEFINES += UMBCS
QMAKE_CXXFLAGS -= -Zc:strictStrings

INCLUDEPATH += include/

SOURCES += \
    main.cpp \
    widget.cpp

HEADERS += \
    include/CameraImageInfo.h \
    include/Common/avglobal.h \
    include/Common/dhconfigsdk.h \
    include/Common/dhnetsdk.h \
    include/play.h \
    widget.h

FORMS += \
    widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

Debug {
        contains(QMAKE_COMPILER_DEFINES, _WIN64) {
                LIBS += -L./thirdparty	-ldhconfigsdk -ldhnetsdk -lplay
                DESTDIR = ./Bin/x64
        }
        else {
                LIBS += -L./thirdparty	-ldhconfigsdk -ldhnetsdk -lplay
                DESTDIR = ./Bin/win32
        }
}
else {
        contains(QMAKE_COMPILER_DEFINES, _WIN64) {
                LIBS += -L./lib	-ldhconfigsdk -ldhnetsdk -lplay -lopencv_world455
                DESTDIR = ./Bin/x64
        }
        else {
                LIBS += -L./thirdparty	-ldhconfigsdk -ldhnetsdk -lplay -lMVSDKmd
                DESTDIR = ./Bin/win32
    }
}
