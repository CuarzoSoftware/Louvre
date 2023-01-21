TEMPLATE = subdirs

CONFIG -= qt
CONFIG -= app_bundle
CONFIG += ordered
CONFIG += c++17

SUBDIRS = graphic/DRM/LGraphicBackendDRM.pro \
          graphic/X11/LGraphicBackendX11.pro \
          input/Libinput/LInputBackendLibinput.pro \
          input/X11/LInputBackendX11.pro \


