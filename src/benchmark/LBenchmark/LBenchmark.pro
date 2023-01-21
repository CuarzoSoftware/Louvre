TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -L/usr/local/lib/x86_64-linux-gnu -lwayland-client -lrt

SOURCES += \
        main.c \
        shm.c \
        xdg-shell-protocol.c

HEADERS += \
    shm.h \
    xdg-shell-client-protocol.h
