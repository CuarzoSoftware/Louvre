#ifndef LCONTENTTYPE_H
#define LCONTENTTYPE_H

namespace Louvre
{
    /**
     * @brief Content Type Hint.
     *
     * Hint indicating the type of content being displayed by an LSurface or LOutput.
     *
     * @see LOutput::contentType() and LOutput::setContentType().
     * @see LSurface::contentType() and LSurface::contentTypeChanged().
     */
    enum LContentType
    {
        LContentTypeNone   = 0, ///< The content doesn't fit into one of the other categories.
        LContentTypePhoto  = 1, ///< Digital still pictures that may be presented with minimal processing.
        LContentTypeVideo  = 2, ///< Video or animation that may be presented with more accurate timing to avoid stutter. Where scaling is needed, scaling methods more appropriate for video may be used.
        LContentTypeGame   = 3, ///< A running game. Its content may be presented with reduced latency.
    };
}

#endif // LCONTENTTYPE_H
