#ifndef LREGION_H
#define LREGION_H

#include <LNamespaces.h>
#include <LRect.h>
#include <pixman.h>
#include <LFramebuffer.h>

/**
 * @brief Collection of non-overlapping rectangles
 *
 * The LRegion class provides an efficient mechanism for creating sets of rectangles that do not overlap in their geometries.
 * It offers methods for performing operations such as additions, subtractions, intersections, and more on rectangles.
 * This class is extensively used by the library for tasks like calculating surface damage, defining opaque, translucent, and input regions, among others.
 * Internally, LRegion employs the algorithm and functions from the [Pixman](http://www.pixman.org/) library.
 */
class Louvre::LRegion
{
public:
    /**
     * @brief Constructor for creating an empty LRegion.
     */
    inline constexpr LRegion() noexcept
    {
        pixman_region32_init(&m_region);
    }

    /**
     * @brief Initialize the region with a rect.
     */
    inline constexpr LRegion(const LRect &rect) noexcept
    {
        pixman_region32_init_rect(&m_region, rect.x(), rect.y(), rect.w(), rect.h());
    }

    /**
     * @brief Destructor for freeing resources associated with LRegion.
     */
    inline ~LRegion() noexcept
    {
        pixman_region32_fini(&m_region);
    }

    /**
     * @brief Copy constructor for creating an LRegion by copying another LRegion.
     *
     * @param other The LRegion to copy from.
     */
    inline constexpr LRegion(const LRegion &other) noexcept
    {
        pixman_region32_init(&m_region);
        pixman_region32_copy(&m_region, &other.m_region);
    }

    /**
     * @brief Assignment operator for assigning the content of one LRegion to another.
     *
     * @param other The LRegion to assign from.
     * @return A reference to the modified LRegion.
     */
    inline LRegion &operator=(const LRegion &other) noexcept
    {
        pixman_region32_copy(&m_region, &other.m_region);
        return *this;
    }

    /**
     * @brief Clears the LRegion, deleting all rectangles.
     */
    inline void clear() noexcept
    {
        pixman_region32_clear(&m_region);
    }

    /**
     * @brief Adds a rectangle to the LRegion (union operation).
     *
     * @param rect The rectangle to add.
     */
    inline void addRect(const LRect &rect) noexcept
    {
        pixman_region32_union_rect(&m_region, &m_region, rect.x(), rect.y(), rect.w(), rect.h());
    }

    /**
     * @brief Adds a rectangle to the LRegion (union operation).
     *
     * @param pos The rectangle's top-left corner.
     * @param size The rectangle's size.
     */
    inline void addRect(const LPoint &pos, const LSize &size) noexcept
    {
        pixman_region32_union_rect(&m_region, &m_region, pos.x(), pos.y(), size.w(), size.h());
    }

    /**
     * @brief Adds a rectangle to the LRegion (union operation).
     *
     * @param x The x-coordinate of the rectangle's top-left corner.
     * @param y The y-coordinate of the rectangle's top-left corner.
     * @param size The rectangle's size.
     */
    inline void addRect(Int32 x, Int32 y, const LSize &size) noexcept
    {
        pixman_region32_union_rect(&m_region, &m_region, x, y, size.w(), size.h());
    }

    /**
     * @brief Adds a rectangle to the LRegion (union operation).
     *
     * @param pos The rectangle's top-left corner.
     * @param w The width of the rectangle.
     * @param h The height of the rectangle.
     */
    inline void addRect(const LPoint &pos, Int32 w, Int32 h) noexcept
    {
        pixman_region32_union_rect(&m_region, &m_region, pos.x(), pos.y(), w, h);
    }

    /**
     * @brief Adds a rectangle to the LRegion (union operation).
     *
     * @param x The x-coordinate of the rectangle's top-left corner.
     * @param y The y-coordinate of the rectangle's top-left corner.
     * @param w The width of the rectangle.
     * @param h The height of the rectangle.
     */
    inline void addRect(Int32 x, Int32 y, Int32 w, Int32 h) noexcept
    {
        pixman_region32_union_rect(&m_region, &m_region, x, y, w, h);
    }

    /**
     * @brief Adds the content of another LRegion to this LRegion (union operation).
     *
     * @param region The LRegion to add.
     */
    inline void addRegion(const LRegion &region) noexcept
    {
        pixman_region32_union(&m_region, &m_region, &region.m_region);
    }

    /**
     * @brief Subtracts a rectangle from the LRegion.
     *
     * @param rect The rectangle to subtract.
     */
    inline void subtractRect(const LRect &rect) noexcept
    {
        pixman_region32_t tmp;
        pixman_region32_init_rect(&tmp, rect.x(), rect.y(), rect.w(), rect.h());
        pixman_region32_subtract(&m_region, &m_region, &tmp);
        pixman_region32_fini(&tmp);
    }

    /**
     * @brief Subtracts a rectangle from the LRegion.
     *
     * @param pos The rectangle's top-left corner.
     * @param size The rectangle's size.
     */
    inline void subtractRect(const LPoint &pos, const LSize &size) noexcept
    {
        pixman_region32_t tmp;
        pixman_region32_init_rect(&tmp, pos.x(), pos.y(), size.w(), size.h());
        pixman_region32_subtract(&m_region, &m_region, &tmp);
        pixman_region32_fini(&tmp);
    }

    /**
     * @brief Subtracts a rectangle from the LRegion.
     *
     * @param pos The rectangle's top-left corner.
     * @param w The width of the rectangle.
     * @param h The height of the rectangle.
     */
    inline void subtractRect(const LPoint &pos, Int32 w, Int32 h) noexcept
    {
        pixman_region32_t tmp;
        pixman_region32_init_rect(&tmp, pos.x(), pos.y(), w, h);
        pixman_region32_subtract(&m_region, &m_region, &tmp);
        pixman_region32_fini(&tmp);
    }

    /**
     * @brief Subtracts a rectangle from the LRegion.
     *
     * @param x The x-coordinate of the rectangle's top-left corner.
     * @param y The y-coordinate of the rectangle's top-left corner.
     * @param size The rectangle's size.
     */
    inline void subtractRect(Int32 x, Int32 y, const LSize &size) noexcept
    {
        pixman_region32_t tmp;
        pixman_region32_init_rect(&tmp, x, y, size.w(), size.h());
        pixman_region32_subtract(&m_region, &m_region, &tmp);
        pixman_region32_fini(&tmp);
    }

    /**
     * @brief Subtracts a rectangle from the LRegion.
     *
     * @param x The x-coordinate of the rectangle's top-left corner.
     * @param y The y-coordinate of the rectangle's top-left corner.
     * @param w The width of the rectangle.
     * @param h The height of the rectangle.
     */
    inline void subtractRect(Int32 x, Int32 y, Int32 w, Int32 h) noexcept
    {
        pixman_region32_t tmp;
        pixman_region32_init_rect(&tmp, x, y, w, h);
        pixman_region32_subtract(&m_region, &m_region, &tmp);
        pixman_region32_fini(&tmp);
    }

    /**
     * @brief Subtracts another LRegion from this LRegion.
     *
     * @param region The LRegion to subtract.
     */
    inline void subtractRegion(const LRegion &region) noexcept
    {
        pixman_region32_subtract(&m_region, &m_region, &region.m_region);
    }

    /**
     * @brief Intersects this LRegion with another LRegion.
     *
     * @param region The LRegion to intersect with.
     */
    inline void intersectRegion(const LRegion &region) noexcept
    {
        pixman_region32_intersect(&m_region, &m_region, &region.m_region);
    }

    /**
     * @brief Multiplies the components of each rectangle in the LRegion by the given factor.
     *
     * @param factor The factor to multiply by.
     */
    void multiply(Float32 factor) noexcept;

    /**
     * @brief Multiplies the components of each rectangle in the LRegion by the given factor.
     *
     * @param xFactor The x-axis factor to multiply by.
     * @param yFactor The y-axis factor to multiply by.
     */
    void multiply(Float32 xFactor, Float32 yFactor) noexcept;

    /**
    * @brief Check if the LRegion contains a specific point.
     *
     * @param point The point to check.
     * @return true if the region contains the point, false otherwise.
     */
    inline bool containsPoint(const LPoint &point) const noexcept
    {
        return pixman_region32_contains_point(&m_region, point.x(), point.y(), NULL);
    }

    /**
     * @brief Translate each rectangle in the LRegion by the specified offset.
     *
     * @param offset The offset to apply.
     */
    inline void offset(const LPoint &offset) noexcept
    {
        if (offset.x() == 0 && offset.y() == 0)
            return;

        pixman_region32_translate(&m_region, offset.x(), offset.y());
    }

    /**
     * @brief Translate each rectangle in the LRegion by the specified offset.
     *
     * @param x The x offset to apply.
     * @param y The y offset to apply.
     */
    inline void offset(Int32 x, Int32 y) noexcept
    {
        if (x == 0 && x == 0)
            return;

        pixman_region32_translate(&m_region, x, y);
    }

    /**
     * @brief Invert the region contained within the specified rectangle.
     *
     * @param rect The rectangle to define the area of inversion.
     */
    inline void inverse(const LRect &rect) noexcept
    {
        pixman_box32_t r {
            rect.x(),
            rect.y(),
            rect.x() + rect.w(),
            rect.y() + rect.h()
        };

        pixman_region32_inverse(&m_region, &m_region, &r);
    }

    /**
     * @brief Check if the LRegion is empty (contains no rectangles).
     *
     * @return true if the region is empty, false otherwise.
     */
    inline bool empty() const noexcept
    {
        return !pixman_region32_not_empty(&m_region);
    }

    /**
     * @brief Clips the LRegion to the area defined by the specified rectangle.
     *
     * @param rect The rectangle used for clipping.
     */
    inline void clip(const LRect &rect) noexcept
    {
        pixman_region32_intersect_rect(&m_region, &m_region, rect.x(), rect.y(), rect.w(), rect.h());
    }

    /**
     * @brief Clips the LRegion to the area defined by the specified rectangle.
     */
    inline void clip(const LPoint &pos, const LSize &size) noexcept
    {
        pixman_region32_intersect_rect(&m_region, &m_region, pos.x(), pos.y(), size.w(), size.h());
    }

    /**
     * @brief Clips the LRegion to the area defined by the specified rectangle.
     */
    inline void clip(Int32 x, Int32 y, Int32 w, Int32 h) noexcept
    {
        pixman_region32_intersect_rect(&m_region, &m_region, x, y, w, h);
    }

    /**
     * @brief Get the extents of the LRegion.
     *
     * This method returns a pointer to an LBox that represents the bounding box of the LRegion.
     * The LBox contains the minimum and maximum coordinates that enclose the LRegion.
     *
     * @return A pointer to the extents of the LRegion as an LBox.
     */
    inline const LBox &extents() const noexcept
    {
        return *(LBox*)pixman_region32_extents(&m_region);
    }

    /**
     * @brief Retrieves the list of rectangles that form the LRegion.
     *
     * @param n A pointer to an integer that will be set to the number of rectangles.
     * @return A pointer to an array of LBox objects representing the rectangles.
     *
     * @note This variant allows you to access the rectangles using the native Pixman method, which is considerably faster.
     */
    inline LBox *boxes(Int32 *n) const noexcept
    {
        return (LBox*)pixman_region32_rectangles(&m_region, n);
    }

    /**
     * @brief Applies a specified transforma to all rectangles within the given size.
     *
     * This function clips the rectangles within the specified size and applies the specified transform.
     *
     * @param size The size used for clipping and transformation.
     * @param transform The transform to be applied to the rectangles.
     */
    void transform(const LSize &size, LFramebuffer::Transform transform) noexcept;

    // TODO returns an empty region
    static const LRegion &EmptyRegion() noexcept;

    /// @cond OMIT
    static void multiply(LRegion *dst, LRegion *src, Float32 factor) noexcept;
    mutable pixman_region32_t m_region;
    /// @endcond
};

#endif // LREGION_H
