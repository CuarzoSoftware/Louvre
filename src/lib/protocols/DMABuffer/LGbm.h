#ifndef gbm_h_INCLUDED
#define gbm_h_INCLUDED

#include <cstdint>

void gbm_init(int drm_fd);
struct gbm_device *gbm_get_device();
struct gbm_bo *gbm_import_from_dmabuf(int num_fds, int32_t *dmabuf_fds,
uint32_t width, uint32_t height, uint32_t format, uint32_t *strides, uint32_t
*offsets, uint64_t modifier);
uint32_t gbm_get_handle(struct gbm_bo *gbm_bo);
void gbm_destroy(struct gbm_bo *gbm_bo);
void gbm_fini();

#endif // gbm_h_INCLUDED
