#ifdef LOUVRE_DMA_ENABLE

#include "LGbm.h"

#include <assert.h>
#include <gbm.h>
#include <stdio.h>

static struct gbm_device *gbm;

void gbm_init(int drm_fd) {
    gbm = gbm_create_device(drm_fd);
    assert(gbm);
}

struct gbm_device *gbm_get_device() {
    return gbm;
}

struct gbm_bo *gbm_import_from_dmabuf(int num_fds, int32_t *dmabuf_fds,
uint32_t width, uint32_t height, uint32_t format, uint32_t *strides, uint32_t
*offsets, uint64_t modifier)
{
    struct gbm_import_fd_modifier_data data = {
        .width = width,
        .height = height,
        .format = format,
        .num_fds = 1,
        .modifier = modifier
    };

    data.fds[0] = dmabuf_fds[0];
    data.strides[0] = strides[0];
    data.offsets[0] = offsets[0];


    struct gbm_bo *gbm_bo = gbm_bo_import(gbm,
     GBM_BO_IMPORT_FD_MODIFIER, &data, GBM_BO_USE_SCANOUT);

    if (!gbm_bo) {
        fprintf(stderr, "gbm_bo_import failed\n");
        return 0;
    }

    return gbm_bo;
}

void gbm_destroy(struct gbm_bo *gbm_bo) { gbm_bo_destroy(gbm_bo); }

uint32_t gbm_get_handle(struct gbm_bo *gbm_bo) { return gbm_bo_get_handle(gbm_bo).u32; }

void gbm_fini() {
    gbm_device_destroy(gbm);
}

#endif
