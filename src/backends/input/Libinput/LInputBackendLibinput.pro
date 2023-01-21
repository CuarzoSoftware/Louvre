TEMPLATE = lib

CONFIG -= qt
CONFIG += c++17

DEFINES += LInputBackendLibinput

DESTDIR = $$PWD/../../../../build

QMAKE_CXXFLAGS_DEBUG *= -O
QMAKE_CXXFLAGS_RELEASE -= -O
QMAKE_CXXFLAGS_RELEASE -= -O1
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE *= -O3

LIBS += -L/usr/local/lib/x86_64-linux-gnu -linput -ludev -lxkbcommon -lseat
INCLUDEPATH += ../../../lib/classes ../../../lib ../ /usr/local/include

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

SOURCES += \
    LInputBackendLibinput.cpp



