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
    LRegion();

    /**
     * @brief Initialize the region with a rect.
     */
    LRegion(const LRect &rect);

    /**
     * @brief Destructor for freeing resources associated with LRegion.
     */
    ~LRegion();

    /**
     * @brief Copy constructor for creating an LRegion by copying another LRegion.
     *
     * @param other The LRegion to copy from.
     */
    LRegion(const LRegion &other);

    /**
     * @brief Assignment operator for assigning the content of one LRegion to another.
     *
     * @param other The LRegion to assign from.
     * @return A reference to the modified LRegion.
     */
    LRegion &operator=(const LRegion &other);

    /**
     * @brief Clears the LRegion, deleting all rectangles.
     */
    void clear();

    /**
     * @brief Adds a rectangle to the LRegion (union operation).
     *
     * @param rect The rectangle to add.
     */
    void addRect(const LRect &rect);

    /**
     * @brief Adds a rectangle to the LRegion (union operation).
     *
     * @param pos The rectangle's top-left corner.
     * @param size The rectangle's size.
     */
    void addRect(const LPoint &pos, const LSize &size);

    /**
     * @brief Adds a rectangle to the LRegion (union operation).
     *
     * @param x The x-coordinate of the rectangle's top-left corner.
     * @param y The y-coordinate of the rectangle's top-left corner.
     * @param size The rectangle's size.
     */
    void addRect(Int32 x, Int32 y, const LSize &size);

    /**
     * @brief Adds a rectangle to the LRegion (union operation).
     *
     * @param pos The rectangle's top-left corner.
     * @param w The width of the rectangle.
     * @param h The height of the rectangle.
     */
    void addRect(const LPoint &pos, Int32 w, Int32 h);

    /**
     * @brief Adds a rectangle to the LRegion (union operation).
     *
     * @param x The x-coordinate of the rectangle's top-left corner.
     * @param y The y-coordinate of the rectangle's top-left corner.
     * @param w The width of the rectangle.
     * @param h The height of the rectangle.
     */
    void addRect(Int32 x, Int32 y, Int32 w, Int32 h);

    /**
     * @brief Adds the content of another LRegion to this LRegion (union operation).
     *
     * @param region The LRegion to add.
     */
    void addRegion(const LRegion &region);

    /**
     * @brief Subtracts a rectangle from the LRegion.
     *
     * @param rect The rectangle to subtract.
     */
    void subtractRect(const LRect &rect);

    /**
     * @brief Subtracts a rectangle from the LRegion.
     *
     * @param pos The rectangle's top-left corner.
     * @param size The rectangle's size.
     */
    void subtractRect(const LPoint &pos, const LSize &size);

    /**
     * @brief Subtracts a rectangle from the LRegion.
     *
     * @param pos The rectangle's top-left corner.
     * @param w The width of the rectangle.
     * @param h The height of the rectangle.
     */
    void subtractRect(const LPoint &pos, Int32 w, Int32 h);

    /**
     * @brief Subtracts a rectangle from the LRegion.
     *
     * @param x The x-coordinate of the rectangle's top-left corner.
     * @param y The y-coordinate of the rectangle's top-left corner.
     * @param size The rectangle's size.
     */
    void subtractRect(Int32 x, Int32 y, const LSize &size);

    /**
     * @brief Subtracts a rectangle from the LRegion.
     *
     * @param x The x-coordinate of the rectangle's top-left corner.
     * @param y The y-coordinate of the rectangle's top-left corner.
     * @param w The width of the rectangle.
     * @param h The height of the rectangle.
     */
    void subtractRect(Int32 x, Int32 y, Int32 w, Int32 h);

    /**
     * @brief Subtracts another LRegion from this LRegion.
     *
     * @param region The LRegion to subtract.
     */
    void subtractRegion(const LRegion &region);

    /**
     * @brief Intersects this LRegion with another LRegion.
     *
     * @param region The LRegion to intersect with.
     */
    void intersectRegion(const LRegion &region);

    /**
     * @brief Multiplies the components of each rectangle in the LRegion by the given factor.
     *
     * @param factor The factor to multiply by.
     */
    void multiply(Float32 factor);

    /**
     * @brief Multiplies the components of each rectangle in the LRegion by the given factor.
     *
     * @param xFactor The x-axis factor to multiply by.
     * @param yFactor The y-axis factor to multiply by.
     */
    void multiply(Float32 xFactor, Float32 yFactor);

    /**
    * @brief Check if the LRegion contains a specific point.
     *
     * @param point The point to check.
     * @return true if the region contains the point, false otherwise.
     */
    bool containsPoint(const LPoint &point) const;

    /**
     * @brief Translate each rectangle in the LRegion by the specified offset.
     *
     * @param offset The offset to apply.
     */
    void offset(const LPoint &offset);

    /**
     * @brief Translate each rectangle in the LRegion by the specified offset.
     *
     * @param x The x offset to apply.
     * @param y The y offset to apply.
     */
    void offset(Int32 x, Int32 y);

    /**
     * @brief Invert the region contained within the specified rectangle.
     *
     * @param rect The rectangle to define the area of inversion.
     */
    void inverse(const LRect &rect);

    /**
     * @brief Check if the LRegion is empty (contains no rectangles).
     *
     * @return true if the region is empty, false otherwise.
     */
    bool empty() const;

    /**
     * @brief Clips the LRegion to the area defined by the specified rectangle.
     *
     * @param rect The rectangle used for clipping.
     */
    void clip(const LRect &rect);

    /**
     * @brief Clips the LRegion to the area defined by the specified rectangle.
     */
    void clip(const LPoint &pos, const LSize &size);

    /**
     * @brief Clips the LRegion to the area defined by the specified rectangle.
     */
    void clip(Int32 x, Int32 y, Int32 w, Int32 h);

    /**
     * @brief Get the extents of the LRegion.
     *
     * This method returns a pointer to an LBox that represents the bounding box of the LRegion.
     * The LBox contains the minimum and maximum coordinates that enclose the LRegion.
     *
     * @return A pointer to the extents of the LRegion as an LBox.
     */
    const LBox &extents() const;

    /**
     * @brief Retrieves the list of rectangles that form the LRegion.
     *
     * @param n A pointer to an integer that will be set to the number of rectangles.
     * @return A pointer to an array of LBox objects representing the rectangles.
     *
     * @note This variant allows you to access the rectangles using the native Pixman method, which is considerably faster.
     */
    LBox *boxes(Int32 *n) const;

    /**
     * @brief Applies a specified transforma to all rectangles within the given size.
     *
     * This function clips the rectangles within the specified size and applies the specified transform.
     *
     * @param size The size used for clipping and transformation.
     * @param transform The transform to be applied to the rectangles.
     */
    void transform(const LSize &size, LFramebuffer::Transform transform);

    /// @cond OMIT
    static void multiply(LRegion *dst, LRegion *src, Float32 factor);
    mutable pixman_region32_t m_region;
    /// @endcond
};

#endif // LREGION_H
