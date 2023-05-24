TEMPLATE = lib

CONFIG -= qt
CONFIG += c++17

DEFINES += LouvreProject

DESTDIR = $$PWD/../../build

QMAKE_CXXFLAGS_DEBUG *= -O
QMAKE_CXXFLAGS_RELEASE -= -O
QMAKE_CXXFLAGS_RELEASE -= -O1
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE *= -O3

LIBS += -L/usr/local/lib/x86_64-linux-gnu -lwayland-server -lEGL -lGL -lGLESv2 -linput -ludev -lpthread -lXcursor -lxkbcommon -lpixman-1 -ldrm -lgbm -lrt
LIBS += -L/lib/x86_64-linux-gnu -lseat
INCLUDEPATH += /usr/include/libdrm ./core /usr/local/include /usr/include/pixman-1


# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    core/LBaseSurfaceRole.h \
    core/LClient.h \
    core/LCompositor.h \
    core/LCursor.h \
    core/LCursorRole.h \
    core/LDNDIconRole.h \
    core/LDNDManager.h \
    core/LDataDevice.h \
    core/LDataOffer.h \
    core/LDataSource.h \
    core/LKeyboard.h \
    core/LLog.h \
    core/LNamespaces.h \
    core/LOpenGL.h \
    core/LOutput.h \
    core/LOutputManager.h \
    core/LOutputMode.h \
    core/LPainter.h \
    core/LPoint.h \
    core/LPointer.h \
    core/LPopupRole.h \
    core/LPositioner.h \
    core/LRect.h \
    core/LRegion.h \
    core/LResource.h \
    core/LSeat.h \
    core/LSize.h \
    core/LSubsurfaceRole.h \
    core/LSurface.h \
    core/LTexture.h \
    core/LTime.h \
    core/LToplevelRole.h \
    core/LWayland.h \
    core/LXCursor.h \
    core/private/LBaseSurfaceRolePrivate.h \
    core/private/LClientPrivate.h \
    core/private/LCompositorPrivate.h \
    core/private/LCursorPrivate.h \
    core/private/LCursorRolePrivate.h \
    core/private/LDNDIconRolePrivate.h \
    core/private/LDNDManagerPrivate.h \
    core/private/LDataDevicePrivate.h \
    core/private/LDataOfferPrivate.h \
    core/private/LDataSourcePrivate.h \
    core/private/LKeyboardPrivate.h \
    core/private/LOutputManagerPrivate.h \
    core/private/LOutputModePrivate.h \
    core/private/LOutputPrivate.h \
    core/private/LPainterPrivate.h \
    core/private/LPointerPrivate.h \
    core/private/LPopupRolePrivate.h \
    core/private/LPositionerPrivate.h \
    core/private/LResourcePrivate.h \
    core/private/LSeatPrivate.h \
    core/private/LSubsurfaceRolePrivate.h \
    core/private/LSurfacePrivate.h \
    core/private/LTexturePrivate.h \
    core/private/LToplevelRolePrivate.h \
    core/private/LXCursorPrivate.h \
    other/cursor.h \
    other/lodepng.h \
    protocols/DMABuffer/DMA.h \
    protocols/DMABuffer/Feedback.h \
    protocols/DMABuffer/LGbm.h \
    protocols/DMABuffer/LinuxDMABuffer.h \
    protocols/DMABuffer/Params.h \
    protocols/DMABuffer/linux-dmabuf-unstable-v1.h \
    protocols/DMABuffer/wl_buffer_dmabuf.h \
    protocols/PresentationTime/Feedback.h \
    protocols/PresentationTime/Presentation.h \
    protocols/PresentationTime/presentation-time.h \
    protocols/Viewporter/Viewporter.h \
    protocols/Viewporter/viewporter.h \
    protocols/Wayland/GCompositor.h \
    protocols/Wayland/GDataDeviceManager.h \
    protocols/Wayland/RDataDevice.h \
    protocols/Wayland/RDataOffer.h \
    protocols/Wayland/RDataSource.h \
    protocols/Wayland/RKeyboard.h \
    protocols/Wayland/Output.h \
    protocols/Wayland/RPointer.h \
    protocols/Wayland/RRegion.h \
    protocols/Wayland/GSeat.h \
    protocols/Wayland/Subcompositor.h \
    protocols/Wayland/Subsurface.h \
    protocols/Wayland/RSurface.h \
    protocols/Wayland/private/GCompositorPrivate.h \
    protocols/Wayland/private/GDataDeviceManagerPrivate.h \
    protocols/Wayland/private/RDataDevicePrivate.h \
    protocols/Wayland/private/RDataOfferPrivate.h \
    protocols/Wayland/private/RDataSourcePrivate.h \
    protocols/Wayland/private/RKeyboardPrivate.h \
    protocols/Wayland/private/RPointerPrivate.h \
    protocols/Wayland/private/RRegionPrivate.h \
    protocols/Wayland/private/GSeatPrivate.h \
    protocols/Wayland/private/RSurfacePrivate.h \
    protocols/Wayland/wayland.h \
    protocols/XdgDecoration/XdgDecorationManager.h \
    protocols/XdgDecoration/XdgToplevelDecoration.h \
    protocols/XdgDecoration/xdg-decoration-unstable-v1.h \
    protocols/XdgShell/XdgPopup.h \
    protocols/XdgShell/XdgPositioner.h \
    protocols/XdgShell/XdgSurface.h \
    protocols/XdgShell/XdgToplevel.h \
    protocols/XdgShell/XdgWmBase.h \
    protocols/XdgShell/xdg-shell.h

SOURCES += \
    core/LBaseSurfaceRole.cpp \
    core/LClient.cpp \
    core/LCompositor.cpp \
    core/LCursor.cpp \
    core/LCursorRole.cpp \
    core/LDNDIconRole.cpp \
    core/LDNDManager.cpp \
    core/LDataDevice.cpp \
    core/LDataOffer.cpp \
    core/LDataSource.cpp \
    core/LKeyboard.cpp \
    core/LLog.cpp \
    core/LOpenGL.cpp \
    core/LOutput.cpp \
    core/LOutputManager.cpp \
    core/LOutputMode.cpp \
    core/LPainter.cpp \
    core/LPointer.cpp \
    core/LPopupRole.cpp \
    core/LPositioner.cpp \
    core/LRegion.cpp \
    core/LResource.cpp \
    core/LSeat.cpp \
    core/LSubsurfaceRole.cpp \
    core/LSurface.cpp \
    core/LTexture.cpp \
    core/LTime.cpp \
    core/LToplevelRole.cpp \
    core/LWayland.cpp \
    core/LXCursor.cpp \
    core/default/LCompositorDefault.cpp \
    core/default/LCursorRoleDefault.cpp \
    core/default/LDNDIconRoleDefault.cpp \
    core/default/LDNDManagerDefault.cpp \
    core/default/LKeyboardDefault.cpp \
    core/default/LOutputDefault.cpp \
    core/default/LOutputManagerDefault.cpp \
    core/default/LPointerDefault.cpp \
    core/default/LPopupRoleDefault.cpp \
    core/default/LSeatDefault.cpp \
    core/default/LSubsurfaceRoleDefault.cpp \
    core/default/LSurfaceDefault.cpp \
    core/default/LToplevelRoleDefault.cpp \
    other/cursor.cpp \
    other/lodepng.cpp \
    protocols/DMABuffer/Feedback.cpp \
    protocols/DMABuffer/LGbm.cpp \
    protocols/DMABuffer/LinuxDMABuffer.cpp \
    protocols/DMABuffer/Params.cpp \
    protocols/DMABuffer/linux-dmabuf-unstable-v1.c \
    protocols/DMABuffer/wl_buffer_dmabuf.cpp \
    protocols/PresentationTime/Feedback.cpp \
    protocols/PresentationTime/Presentation.cpp \
    protocols/PresentationTime/presentation-time.c \
    protocols/Viewporter/Viewporter.cpp \
    protocols/Viewporter/viewporter.c \
    protocols/Wayland/GCompositor.cpp \
    protocols/Wayland/GDataDeviceManager.cpp \
    protocols/Wayland/RDataDevice.cpp \
    protocols/Wayland/RDataOffer.cpp \
    protocols/Wayland/RDataSource.cpp \
    protocols/Wayland/RKeyboard.cpp \
    protocols/Wayland/Output.cpp \
    protocols/Wayland/RPointer.cpp \
    protocols/Wayland/RRegion.cpp \
    protocols/Wayland/GSeat.cpp \
    protocols/Wayland/Subcompositor.cpp \
    protocols/Wayland/Subsurface.cpp \
    protocols/Wayland/RSurface.cpp \
    protocols/Wayland/private/GCompositorPrivate.cpp \
    protocols/Wayland/private/GDataDeviceManagerPrivate.cpp \
    protocols/Wayland/private/RDataDevicePrivate.cpp \
    protocols/Wayland/private/RDataOfferPrivate.cpp \
    protocols/Wayland/private/RDataSourcePrivate.cpp \
    protocols/Wayland/private/RKeyboardPrivate.cpp \
    protocols/Wayland/private/RPointerPrivate.cpp \
    protocols/Wayland/private/RRegionPrivate.cpp \
    protocols/Wayland/private/GSeatPrivate.cpp \
    protocols/Wayland/private/RSurfacePrivate.cpp \
    protocols/Wayland/wayland.c \
    protocols/XdgDecoration/XdgDecorationManager.cpp \
    protocols/XdgDecoration/XdgToplevelDecoration.cpp \
    protocols/XdgDecoration/xdg-decoration-unstable-v1.c \
    protocols/XdgShell/XdgPopup.cpp \
    protocols/XdgShell/XdgPositioner.cpp \
    protocols/XdgShell/XdgSurface.cpp \
    protocols/XdgShell/XdgToplevel.cpp \
    protocols/XdgShell/XdgWmBase.cpp \
    protocols/XdgShell/xdg-shell.c

