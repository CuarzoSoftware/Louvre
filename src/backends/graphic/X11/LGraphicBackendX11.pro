TEMPLATE = lib

CONFIG -= qt
CONFIG += c++17

DEFINES += LGraphicBackendX11

DESTDIR = $$PWD/../../../../build

QMAKE_CXXFLAGS_DEBUG *= -O
QMAKE_CXXFLAGS_RELEASE -= -O
QMAKE_CXXFLAGS_RELEASE -= -O1
QMAKE_CXXFLAGS_RELEASE *= -O2
QMAKE_CXXFLAGS_RELEASE -= -O3

LIBS += -L/usr/local/lib/x86_64-linux-gnu -lX11 -lXfixes -lXrandr
INCLUDEPATH += /usr/include/libdrm ../../../lib ../../../lib/core  ../ /usr/local/include /usr/include/pixman-1


# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

SOURCES += \
    LGraphicBackendX11.cpp



