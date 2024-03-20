#ifndef GOUTPUT_H
#define GOUTPUT_H

#include <LResource.h>

class Louvre::Protocols::Wayland::GOutput final : public LResource
{
public:

    LOutput *output() const noexcept
    {
        return m_output.get();
    }

    /******************** REQUESTS ********************/

    static void bind(wl_client *client, void *output, UInt32 version, UInt32 id) noexcept;
#if LOUVRE_WL_OUTPUT_VERSION >= 3
    static void release(wl_client *client, wl_resource *resource) noexcept;
#endif

    /******************** EVENTS ********************/

    // Send all events
    void sendConfiguration() noexcept;

    // Since 1
    void geometry(Int32 x, Int32 y,
                  Int32 physicalWidth, Int32 physicalHeight,
                  Int32 subpixel, const char *make,
                  const char *model, Int32 transform) noexcept;
    void mode(UInt32 flags, Int32 width, Int32 height, Int32 refresh) noexcept;

    // Since 2
    bool done() noexcept;
    bool scale(Int32 factor) noexcept;

    // Since 4
    bool name(const char *name) noexcept;
    bool description(const char *description) noexcept;

private:
    friend class GammaControl::RGammaControl;
    friend class Louvre::LCompositor;

    GOutput(LOutput *output,
            wl_client *client,
            Int32 version,
            UInt32 id) noexcept;

    ~GOutput() noexcept;

    LWeak<LOutput> m_output;
    std::vector<GammaControl::RGammaControl*> m_gammaControlRes;
};

#endif // GOUTPUT_H
