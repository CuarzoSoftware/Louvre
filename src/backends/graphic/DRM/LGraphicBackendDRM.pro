TEMPLATE = lib

CONFIG -= qt
CONFIG += c++17

DEFINES += LGraphicBackendDRM

DESTDIR = $$PWD/../../../../build

QMAKE_CXXFLAGS_DEBUG *= -O
QMAKE_CXXFLAGS_RELEASE -= -O
QMAKE_CXXFLAGS_RELEASE -= -O1
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE *= -O3

LIBS += -L/usr/local/lib/x86_64-linux-gnu -ldrm -lgbm -ludev
INCLUDEPATH += /usr/include/libdrm ../../../lib ../../../lib/classes  ../ /usr/local/include /usr/include/pixman-1


# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

SOURCES += \
    LDRM.cpp \
    LDRMConnector.cpp \
    LDRMCrtc.cpp \
    LDRMDevice.cpp \
    LDRMEncoder.cpp \
    LDRMOutput.cpp \
    LDRMOutputMode.cpp \
    LDRMPlane.cpp \
    LGraphicBackendDRM.cpp

HEADERS += \
    LDRM.h \
    LDRMConnector.h \
    LDRMCrtc.h \
    LDRMDevice.h \
    LDRMEncoder.h \
    LDRMOutput.h \
    LDRMOutputMode.h \
    LDRMPlane.h



