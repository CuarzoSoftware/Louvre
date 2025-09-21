#ifndef LTEXTUREPRIVATE_H
#define LTEXTUREPRIVATE_H

#include <GL/gl.h>
#include <LSize.h>
#include <LTexture.h>

using namespace Louvre;

class LTexture::LTexturePrivate {
 public:
  inline static void setTextureParams(GLuint textureId, GLenum target,
                                      GLenum wrapS, GLenum wrapT,
                                      GLenum minFilter,
                                      GLenum magFilter) noexcept {
    glBindTexture(target, textureId);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, wrapS);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, wrapT);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilter);
  }

  inline static void readPixels(const LRect &src, const LPoint &dstOffset,
                                Int32 dstWidth, GLenum format, GLenum type,
                                UChar8 *buffer) noexcept {
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glPixelStorei(GL_PACK_ROW_LENGTH, dstWidth);
    glPixelStorei(GL_PACK_SKIP_PIXELS, dstOffset.x());
    glPixelStorei(GL_PACK_SKIP_ROWS, dstOffset.y());
    glReadPixels(src.x(), src.y(), src.w(), src.h(), format, type, buffer);
    glPixelStorei(GL_PACK_ROW_LENGTH, 0);
    glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_PACK_SKIP_ROWS, 0);
  }
};

#endif  // LTEXTUREPRIVATE_H
