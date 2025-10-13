#include <CZ/Louvre/Protocols/Wayland/RDataDevice.h>
#include <CZ/Louvre/Protocols/Wayland/GSeat.h>
#include <CZ/Louvre/Protocols/XdgShell/GXdgWmBase.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/Cursor/LRoleCursorSource.h>
#include <CZ/Louvre/LClient.h>

LClient::LClient(const void *params) noexcept : LFactoryObject(FactoryObjectType), m_imp { std::make_unique<LClientPrivate>(this, ((Params*)params)->client) }
{
    imp()->cursor = LRoleCursorSource::MakeDefault(this);

    // Set all history events serials to 0

    for (Int32 i = 0; i < 5; i++)
    {
        imp()->eventHistory.pointer.button[i].serial = 0;
        imp()->eventHistory.keyboard.key[i].serial = 0;
    }

    imp()->eventHistory.pointer.enter.serial = 0;
    imp()->eventHistory.pointer.leave.serial = 0;
    imp()->eventHistory.pointer.holdBegin.serial = 0;
    imp()->eventHistory.pointer.holdEnd.serial = 0;
    imp()->eventHistory.pointer.pinchBegin.serial = 0;
    imp()->eventHistory.pointer.pinchEnd.serial = 0;
    imp()->eventHistory.pointer.swipeBegin.serial = 0;
    imp()->eventHistory.pointer.swipeEnd.serial = 0;

    imp()->eventHistory.keyboard.enter.serial = 0;
    imp()->eventHistory.keyboard.leave.serial = 0;
    imp()->eventHistory.keyboard.modifiers.serial = 0;
}

LClient::~LClient()
{
    notifyDestruction();

    if (imp()->pendingDestroyLater)
        compositor()->imp()->destroyedClients.erase(this);
}

void LClient::credentials(pid_t *pid, uid_t *uid, gid_t *gid) const noexcept
{
    wl_client_get_credentials(client(), pid, uid, gid);
}

const CZEvent *LClient::findEventBySerial(UInt32 serial) const noexcept
{
    if (imp()->eventHistory.keyboard.enter.serial == serial)
        return &imp()->eventHistory.keyboard.enter;
    else if (imp()->eventHistory.keyboard.leave.serial == serial)
        return &imp()->eventHistory.keyboard.leave;

    for (UInt8 i = 0; i < 5; i++)
        if (imp()->eventHistory.keyboard.key[i].serial == serial)
            return &imp()->eventHistory.keyboard.key[i];

    if (imp()->eventHistory.keyboard.modifiers.serial == serial)
        return &imp()->eventHistory.keyboard.modifiers;

    else if (imp()->eventHistory.pointer.enter.serial == serial)
        return &imp()->eventHistory.pointer.enter;
    else if (imp()->eventHistory.pointer.leave.serial == serial)
        return &imp()->eventHistory.pointer.leave;

    for (UInt8 i = 0; i < 5; i++)
        if (imp()->eventHistory.pointer.button[i].serial == serial)
            return &imp()->eventHistory.pointer.button[i];

    if (imp()->eventHistory.pointer.swipeBegin.serial == serial)
        return &imp()->eventHistory.pointer.swipeBegin;
    else if (imp()->eventHistory.pointer.swipeEnd.serial == serial)
        return &imp()->eventHistory.pointer.swipeEnd;

    else if (imp()->eventHistory.pointer.pinchBegin.serial == serial)
        return &imp()->eventHistory.pointer.pinchBegin;
    else if (imp()->eventHistory.pointer.pinchEnd.serial == serial)
        return &imp()->eventHistory.pointer.pinchEnd;

    else if (imp()->eventHistory.pointer.holdBegin.serial == serial)
        return &imp()->eventHistory.pointer.holdBegin;
    else if (imp()->eventHistory.pointer.holdEnd.serial == serial)
        return &imp()->eventHistory.pointer.holdEnd;

    for (auto it = imp()->eventHistory.touch.down.begin(); it != imp()->eventHistory.touch.down.end(); it++)
        if (it->serial == serial)
            return &(*it);

    for (auto it = imp()->eventHistory.touch.up.begin(); it != imp()->eventHistory.touch.up.end(); it++)
        if (it->serial == serial)
            return &(*it);

    for (auto *seat : seatGlobals())
        if (seat->dataDeviceRes() && seat->dataDeviceRes()->enterSerial() == serial)
            return &imp()->eventHistory.keyboard.enter;

    return nullptr;
}

std::shared_ptr<LCursorSource> LClient::cursor() const noexcept
{
    return imp()->cursor;
}

void LClient::postErrorPrivate(wl_resource *resource, UInt32 code, const std::string &message)
{
    if (imp()->destroyed)
        return;

    imp()->destroyed = true;
    wl_resource_post_error(resource, code, "%s", message.c_str());
}

bool LClient::ping(UInt32 serial) const noexcept
{
    if (imp()->xdgWmBaseGlobals.empty())
        return false;

    imp()->xdgWmBaseGlobals.front()->ping(serial);
    return true;
}

wl_client *LClient::client() const noexcept
{
    return imp()->client;
}

void LClient::flush() noexcept
{
    wl_client_flush(client());
}

void LClient::destroyLater() noexcept
{
    if (imp()->pendingDestroyLater)
        return;

    imp()->pendingDestroyLater = true;
    compositor()->imp()->destroyedClients.insert(this);
}

void LClient::destroy()
{
    if (imp()->destroyed)
        return;

    imp()->destroyed = true;
    wl_client_destroy(client());
}

const std::vector<Wayland::GOutput*> &LClient::outputGlobals() const noexcept
{
    return imp()->outputGlobals;
}

const std::vector<Wayland::GCompositor*> &LClient::compositorGlobals() const noexcept
{
    return imp()->compositorGlobals;
}

const std::vector<Wayland::GSubcompositor*> &LClient::subcompositorGlobals() const noexcept
{
    return imp()->subcompositorGlobals;
}

const std::vector<Wayland::GSeat*> &LClient::seatGlobals() const noexcept
{
    return imp()->seatGlobals;
}

const std::vector<Wayland::GDataDeviceManager*> &LClient::dataDeviceManagerGlobals() const noexcept
{
    return imp()->dataDeviceManagerGlobals;
}

const std::vector<XdgShell::GXdgWmBase *> &LClient::xdgWmBaseGlobals() const noexcept
{
    return imp()->xdgWmBaseGlobals;
}

const std::vector<PresentationTime::GPresentation *> &LClient::presentationTimeGlobals() const noexcept
{
    return imp()->presentationTimeGlobals;
}

const std::vector<LinuxDMABuf::GZwpLinuxDmaBufV1 *> &LClient::linuxDMABufGlobals() const noexcept
{
    return imp()->linuxDMABufGlobals;
}

const std::vector<FractionalScale::GFractionalScaleManager *> &LClient::fractionalScaleManagerGlobals() const noexcept
{
    return imp()->fractionalScaleManagerGlobals;
}

const std::vector<GammaControl::GZwlrGammaControlManagerV1 *> &LClient::gammaControlManagerGlobals() const noexcept
{
    return imp()->gammaControlManagerGlobals;
}

const std::vector<TearingControl::GTearingControlManager *> &LClient::tearingControlManagerGlobals() const noexcept
{
    return imp()->tearingControlManagerGlobals;
}

const std::vector<Viewporter::GViewporter *> &LClient::viewporterGlobals() const noexcept
{
    return imp()->viewporterGlobals;
}

const std::vector<XdgDecoration::GXdgDecorationManager *> &LClient::xdgDecorationManagerGlobals() const noexcept
{
    return imp()->xdgDecorationManagerGlobals;
}

const std::vector<RelativePointer::GRelativePointerManager *> &LClient::relativePointerManagerGlobals() const noexcept
{
    return imp()->relativePointerManagerGlobals;
}

const std::vector<PointerGestures::GPointerGestures *> &LClient::pointerGesturesGlobals() const noexcept
{
    return imp()->pointerGesturesGlobals;
}

const std::vector<SessionLock::GSessionLockManager *> &LClient::sessionLockManagerGlobals() const noexcept
{
    return imp()->sessionLockManagerGlobals;
}

const std::vector<PointerConstraints::GPointerConstraints *> &LClient::pointerConstraintsGlobals() const noexcept
{
    return imp()->pointerConstraintsGlobals;
}

const std::vector<XdgOutput::GXdgOutputManager *> &LClient::xdgOutputManagerGlobals() const noexcept
{
    return imp()->xdgOutputManagerGlobals;
}

const std::vector<ImageCaptureSource::GOutputImageCaptureSourceManager *> &LClient::outputImageCaptureSourceManagerGlobals() const noexcept
{
    return imp()->outputImageCaptureSourceManagerGlobals;
}

const std::vector<ImageCaptureSource::GForeignToplevelImageCaptureSourceManager *> &LClient::foreignToplevelImageCaptureSourceManagerGlobals() const noexcept
{
    return imp()->foreignToplevelImageCaptureSourceManagerGlobals;
}

const std::vector<LayerShell::GLayerShell *> &LClient::layerShellGlobals() const noexcept
{
    return imp()->layerShellGlobals;
}

const std::vector<ForeignToplevelManagement::GForeignToplevelManager *> &LClient::foreignToplevelManagerGlobals() const noexcept
{
    return imp()->foreignToplevelManagerGlobals;
}

const std::vector<ForeignToplevelList::GForeignToplevelList *> &LClient::foreignToplevelListGlobals() const noexcept
{
    return imp()->foreignToplevelListGlobals;
}

const std::vector<SinglePixelBuffer::GWpSinglePixelBufferManagerV1 *> &LClient::singlePixelBufferManagerGlobals() const noexcept
{
    return imp()->singlePixelBufferManagerGlobals;
}

const std::vector<ContentType::GWpContentTypeManagerV1*> &LClient::contentTypeManagerGlobals() const noexcept
{
    return imp()->contentTypeManagerGlobals;
}

const std::vector<IdleNotify::GIdleNotifier *> &LClient::idleNotifierGlobals() const noexcept
{
    return imp()->idleNotifierGlobals;
}

const std::vector<IdleInhibit::GIdleInhibitManager *> &LClient::idleInhibitManagerGlobals() const noexcept
{
    return imp()->idleInhibitManagerGlobals;
}

const std::vector<XdgActivation::GXdgActivation *> &LClient::xdgActivationGlobals() const noexcept
{
    return imp()->xdgActivationGlobals;
}

const std::vector<DRMLease::GDRMLeaseDevice *> &LClient::drmLeaseDeviceGlobals() const noexcept
{
    return imp()->drmLeaseDeviceGlobals;
}

const std::vector<WlrOutputManagement::GWlrOutputManager *> &LClient::wlrOutputManagerGlobals() const noexcept
{
    return imp()->wlrOutputManagerGlobals;
}

const std::vector<BackgroundBlur::GBackgroundBlurManager *> &LClient::backgroundBlurManagerGlobals() const noexcept
{
    return imp()->backgroundBlurManagerGlobals;
}

const std::vector<CursorShape::GCursorShapeManager *> &LClient::cursorShapeManagerGlobals() const noexcept
{
    return imp()->cursorShapeManagerGlobals;
}

const std::vector<SvgPath::GSvgPathManager *> &LClient::svgPathManagerGlobals() const noexcept
{
    return imp()->svgPathManagerGlobals;
}

const std::vector<InvisibleRegion::GInvisibleRegionManager *> &LClient::invisibleRegionManagerGlobals() const noexcept
{
    return imp()->invisibleRegionManagerGlobals;
}

const std::vector<WaylandDRM::GWlDRM *> LClient::wlDRMGlobals() const noexcept
{
    return imp()->wlDRMGlobals;
}

const std::vector<DRMSyncObj::GDRMSyncObjManager *> LClient::drmSyncObjManager() const noexcept
{
    return imp()->drmSyncObjManagerGlobals;
}

const LClient::EventHistory &LClient::eventHistory() const noexcept
{
    return imp()->eventHistory;
}
