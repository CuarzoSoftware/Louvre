#include <protocols/Wayland/RDataSource.h>
#include <LClipboard.h>
#include <LSeat.h>
#include <cassert>

using namespace Louvre;

LClipboard::LClipboard(const void *params) noexcept
{
    assert(params != nullptr && "Invalid parameter passed to LClipboard() constructor. LClipboard can only be created from LCompositor::createClipboardRequest().");
    LClipboard**ptr { (LClipboard**) params };
    assert(*ptr == nullptr && *ptr == seat()->clipboard() && "Only a single LClipboard() instance can exist.");
    *ptr = this;
}

const std::vector<LClipboard::MimeTypeFile> &LClipboard::mimeTypes() const noexcept
{
    if (m_dataSource.get())
        return m_dataSource.get()->m_mimeTypes;

    return m_persistentMimeTypes;
}

void LClipboard::clear() noexcept
{
    while (!m_persistentMimeTypes.empty())
    {
        fclose(m_persistentMimeTypes.back().tmp);
        m_persistentMimeTypes.pop_back();
    }
}
