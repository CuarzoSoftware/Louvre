#include <CZ/Louvre/Protocols/XdgForeign/RXdgExportedBase.h>
#include <CZ/Louvre/Protocols/XdgForeign/RXdgImportedBase.h>
#include <CZ/Louvre/Roles/LToplevelRole.h>
#include <format>
#include <random>
#include <unordered_map>

using namespace CZ;
using namespace CZ::Protocols::XdgForeign;

/*
 * Live handles of every client. Everything runs on the main thread (client dispatch and role
 * destruction), so no lock is needed. One compositor per process: a static is enough.
 */
static std::unordered_map<std::string, RXdgExportedBase*> &Handles() noexcept
{
    static std::unordered_map<std::string, RXdgExportedBase*> handles;
    return handles;
}

/*
 * Handles are capabilities (anyone who has one can parent windows to the exported toplevel): they
 * are minted from the OS CSPRNG (std::random_device is getrandom backed on Linux), not a seeded
 * userspace PRNG whose state could be recovered from observed handles. 32 bits per draw.
 */
static std::string MintHandle() noexcept
{
    static std::random_device device;
    const auto rand64 = []{ return (UInt64(device()) << 32) | device(); };
    std::string handle;

    do
        handle = std::format("{:016x}{:016x}", rand64(), rand64());
    while (Handles().contains(handle));

    return handle;
}

RXdgExportedBase *RXdgExportedBase::Find(const std::string &handle) noexcept
{
    const auto it { Handles().find(handle) };
    return it == Handles().end() ? nullptr : it->second;
}

RXdgExportedBase::RXdgExportedBase
    (
        wl_client *client,
        const wl_interface *iface,
        Int32 version,
        UInt32 id,
        LToplevelRole *toplevel,
        const void *imp
    ) noexcept
    :LResource
    (
        client,
        iface,
        version,
        id,
        imp
    ),
    m_toplevel(toplevel)
{
    // Inert (no toplevel): no handle.
    if (!toplevel)
        return;

    m_handle = MintHandle();
    Handles().emplace(m_handle, this);
    m_published = true;

    // The exported toplevel is gone: the handle is no longer valid. This object stays alive (the
    // client still owns it) but can't be imported anymore.
    m_toplevel.setOnDestroyCallback([this](LToplevelRole *) noexcept { invalidate(); });
}

RXdgExportedBase::~RXdgExportedBase() noexcept
{
    invalidate();
}

void RXdgExportedBase::invalidate() noexcept
{
    if (m_published)
    {
        Handles().erase(m_handle);
        m_published = false;
    }

    // exportInvalidated() may run client code paths: iterate a copy.
    const auto importers { std::move(m_importers) };
    m_importers.clear();

    for (auto *importer : importers)
        importer->exportInvalidated();
}
