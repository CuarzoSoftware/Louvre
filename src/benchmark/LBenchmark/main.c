#include <bits/time.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <wayland-client.h>
#include <wayland-client-protocol.h>

#include "shm.h"
#include "xdg-shell-client-protocol.h"

struct wl_callback_listener wl_callback_listener;
static void wl_callback_handle_done(void *data, struct wl_callback *callback, uint32_t ms);

int N;

// Global
FILE *fp;
struct wl_display *display;
int outputScale = 1;
int childSize = 256;

// Globals
static struct wl_shm *shm = NULL;
static struct wl_output *output = NULL;
static struct wl_subcompositor *subcompositor = NULL;
static struct wl_compositor *compositor = NULL;
static struct xdg_wm_base *xdg_wm_base = NULL;

struct wl_region *childRegion;

// Padre maximizado
struct ParentSurface
{
    struct wl_region *region;
    unsigned char *shm_data;
    struct wl_buffer *buffer;
    struct wl_surface *surface;
    struct xdg_surface *xdg_surface;
    struct xdg_toplevel *xdg_toplevel;
    int width, height;
}parent;

// Buffer para hijos opacos
unsigned char *shm_opaque;
struct wl_buffer *buffer_opaque;

// Buffer para hijos transparentes
unsigned char *shm_transparent;
struct wl_buffer *buffer_transparent;

struct ChildSurface
{
    struct wl_surface *surface;
    struct wl_subsurface *subsurface;
    float phaseX;
    float phaseY;
    float speed;
};

struct ChildSurface *children;

static struct wl_callback *callback = NULL;

struct timespec start, end;

static void noop(){}

long long int wait_ms = 0;

long long int ms = 0;
long long int total = 0;
int firstCallback = 0;
char fname[128];

void save()
{
    fprintf(fp, "RESULTS\n");
    fprintf(fp, "Output Scale: %d\n", outputScale);
    fprintf(fp, "Maximized Surface Size: %d x %d\n", parent.width, parent.height);
    fprintf(fp, "Total Frames: %lld\n", total);
    fprintf(fp, "Milisegundos: %lld\n", ms);
    fprintf(fp, "FPS: %f\n", (1000.f*(float)total)/(float)ms);
    fclose(fp);
    exit(0);
}

static void wl_callback_handle_done(void *data, struct wl_callback *callback, uint32_t m)
{


    /*
    unsigned char r = rand() %256;
    unsigned char g = rand() %256;
    unsigned char b = rand() %256;
    unsigned char a = rand() %256;

    for(int i = 0; i < childSize*outputScale*childSize*outputScale*4; i+=4)
    {
        shm_transparent[i] = r;
        shm_transparent[i+1] = g;
        shm_transparent[i+2] = b;
        shm_transparent[i+3] = a;
    }

    r = rand() %256;
    g = rand() %256;
    b = rand() %256;
    a = rand() %256;

    for(int i = 0; i < childSize*outputScale*childSize*outputScale*4; i+=4)
    {
        shm_opaque[i] = r;
        shm_opaque[i+1] = g;
        shm_opaque[i+2] = b;
        shm_opaque[i+3] = a;
    }
    */
    if(firstCallback == 0)
    {
        firstCallback = 1;
        clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    }
    else
    {
        //do stuff
        clock_gettime(CLOCK_MONOTONIC_RAW, &end);

        ms = ((end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec)) / 1000000;

        if(ms >= wait_ms)
            save();

        float t = ms/50.f;

        for(int i = 0; i < N; i++)
        {
            int x = (parent.width-childSize)* (sin(children[i].phaseX+t*children[i].speed)+1)/2;
            int y = (parent.height-childSize)* (sin(children[i].phaseY+t*children[i].speed)+1)/2;
            wl_subsurface_set_position(children[i].subsurface, x, y);


            if(i % 2 == 0)
            {
                //wl_surface_attach(children[i].surface, buffer_opaque, 0, 0);
                //wl_surface_damage(children[i].surface, 0, 0, 10, 10);
                //wl_surface_damage(children[i].surface, 50, 50, 10, 10);
                wl_surface_set_opaque_region(children[i].surface,childRegion);
            }
            else
            {
                //wl_surface_attach(children[i].surface, buffer_transparent, 0, 0);
                //wl_surface_damage(children[i].surface, 0, 0, 10, 10);
                //wl_surface_damage(children[i].surface, 50, 50, 10, 10);
                wl_surface_set_opaque_region(children[i].surface, NULL);
            }

            //wl_surface_commit(children[i].surface);
        }
    }

    //printf("Frame count %d\n", total);

    /*
    wl_surface_attach(parent.surface, parent.buffer, 0, 0);
    wl_surface_damage(parent.surface, 0,0,parent.width,parent.height);
    */
    /*
    unsigned char r = rand() %256;
    unsigned char g = rand() %256;
    unsigned char b = rand() %256;
    unsigned char a = rand() %256;

    for(int i = 0; i < parent.width*outputScale*4*10*outputScale;i+=4)
    {
        parent.shm_data[i] = r;
        parent.shm_data[i+1] = g;
        parent.shm_data[i+2] = b;
        parent.shm_data[i+3] = a;
    }
    */
    callback = wl_surface_frame(parent.surface);
    wl_callback_add_listener(callback, &wl_callback_listener, parent.surface);
    //wl_surface_attach(parent.surface, parent.buffer, 0, 0);
    //wl_surface_damage(parent.surface,0,0,parent.width,10);

    wl_surface_set_opaque_region(parent.surface,parent.region);

    wl_surface_commit(parent.surface);
    total++;
}

struct wl_callback_listener wl_callback_listener =
{
    .done = &wl_callback_handle_done
};

static void xdg_surface_handle_configure(void *data, struct xdg_surface *xdg_surface, uint32_t serial)
{
	xdg_surface_ack_configure(xdg_surface, serial);
    wl_surface_commit(parent.surface);
}

static const struct xdg_surface_listener xdg_surface_listener =
{
	.configure = xdg_surface_handle_configure,
};

static void xdg_toplevel_handle_configure(void *data, struct xdg_toplevel *xdg_toplevel, int32_t w, int32_t h, struct wl_array *states)
{
    printf("Parent configured %d %d\n",w,h);

    if(w*h != 0)
    {
        parent.width = w;
        parent.height = h;
    }
}

static const struct xdg_toplevel_listener xdg_toplevel_listener =
{
    .configure = &xdg_toplevel_handle_configure,
    .close = &noop,
};

static void wl_output_handle_scale(void *data, struct wl_output *output, int32_t scale)
{
    //printf("Output scale %d\n", scale);
    outputScale = scale;
}

static const struct wl_output_listener wl_output_listener =
{
    .scale = &wl_output_handle_scale,
    .description = &noop,
    .done = &noop,
    .geometry = &noop,
    .mode = &noop
};

static void xdg_wm_base_handle_ping(void *data, struct xdg_wm_base *wm, uint32_t serial)
{
    xdg_wm_base_pong(wm,serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener =
{
    .ping = &xdg_wm_base_handle_ping
};

static void handle_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
{
    (void)data; (void)version;

    if (strcmp(interface, wl_shm_interface.name) == 0)
    {
		shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
    }
    else if (strcmp(interface, wl_output_interface.name) == 0)
    {
        output = wl_registry_bind(registry, name, &wl_output_interface, 3);
        wl_output_add_listener(output, &wl_output_listener, NULL);
    }
    else if (strcmp(interface, wl_compositor_interface.name) == 0)
    {
        compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 3);
    }
    else if (strcmp(interface, wl_subcompositor_interface.name) == 0)
    {
        subcompositor = wl_registry_bind(registry, name, &wl_subcompositor_interface, 1);
    }
    else if (strcmp(interface, xdg_wm_base_interface.name) == 0)
    {
		xdg_wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(xdg_wm_base, &xdg_wm_base_listener, NULL);
	}
}

static void handle_global_remove(void *data, struct wl_registry *registry, uint32_t name)
{

}

static const struct wl_registry_listener registry_listener =
{
	.global = handle_global,
	.global_remove = handle_global_remove,
};

static struct wl_buffer *create_buffer(int w, int h)
{
    int stride = w * 4;
    int size = stride * h;

	int fd = create_shm_file(size);

    if (fd < 0)
    {
        //printf(stderr, "creating a buffer file for %d B failed: %m\n", size);
		return NULL;
	}

    void *data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (data == MAP_FAILED)
    {
        //printf(stderr, "mmap failed: %m\n");
		close(fd);
		return NULL;
	}

	struct wl_shm_pool *pool = wl_shm_create_pool(shm, fd, size);
    struct wl_buffer *buffer = wl_shm_pool_create_buffer(pool, 0, w, h, stride, WL_SHM_FORMAT_ARGB8888);
    wl_buffer_set_user_data(buffer, data);
	wl_shm_pool_destroy(pool);

	return buffer;
}

void initParent()
{
    parent.surface = wl_compositor_create_surface(compositor);
    parent.xdg_surface = xdg_wm_base_get_xdg_surface(xdg_wm_base, parent.surface);
    parent.xdg_toplevel = xdg_surface_get_toplevel(parent.xdg_surface);

    xdg_surface_add_listener(parent.xdg_surface, &xdg_surface_listener, NULL);
    xdg_toplevel_add_listener(parent.xdg_toplevel, &xdg_toplevel_listener, NULL);

    wl_display_roundtrip(display);

    wl_surface_set_buffer_scale(parent.surface, outputScale);
    wl_surface_commit(parent.surface);

    xdg_toplevel_set_maximized(parent.xdg_toplevel);
    wl_surface_commit(parent.surface);

    wl_display_roundtrip(display);

    parent.buffer = create_buffer(parent.width*outputScale, parent.height*outputScale);
    parent.shm_data = wl_buffer_get_user_data(parent.buffer);

    for(int i = 0; i < parent.width*parent.height*outputScale*outputScale*4; i+=4)
    {
        parent.shm_data[i] = 255;
        parent.shm_data[i+1] = 255;
        parent.shm_data[i+2] = 255;
        parent.shm_data[i+3] = 255;
    }

    parent.region = wl_compositor_create_region(compositor);
    wl_region_add(parent.region,0,0,parent.width,parent.height);
    wl_surface_set_opaque_region(parent.surface,parent.region);

    wl_surface_attach(parent.surface, parent.buffer, 0, 0);
    wl_surface_commit(parent.surface);
    wl_display_roundtrip(display);
}

void initChildren()
{
    children = malloc(sizeof(struct ChildSurface)*N);
    childRegion = wl_compositor_create_region(compositor);
    wl_region_add(childRegion,0,0,childSize,childSize);
    wl_display_roundtrip(display);

    for(int i = 0; i < N; i++)
    {
        children[i].phaseX = 6.28f*(float)(rand() % 10000)/10000.f;
        children[i].phaseY = 6.28f*(float)(rand() % 10000)/10000.f;
        children[i].speed = 0.01 + 0.1*(float)(rand() % 10000)/10000.f;
        children[i].surface = wl_compositor_create_surface(compositor);
        wl_surface_set_buffer_scale(children[i].surface, outputScale);
        wl_display_roundtrip(display);
        children[i].subsurface = wl_subcompositor_get_subsurface(subcompositor,children[i].surface, parent.surface);
        //wl_subsurface_set_desync(children[i].subsurface);
        wl_surface_commit(children[i].surface);
        wl_display_roundtrip(display);
        wl_subsurface_set_position(children[i].subsurface, 100, 100);
        wl_surface_commit(parent.surface);
        wl_display_roundtrip(display);
        wl_surface_commit(children[i].surface);

        // Opaca
        if(i % 2 == 0)
        {
            wl_surface_attach(children[i].surface, buffer_opaque, 0, 0);
            wl_surface_set_opaque_region(children[i].surface,childRegion);
        }
        else
        {
            wl_surface_attach(children[i].surface, buffer_transparent, 0, 0);
            wl_surface_set_opaque_region(children[i].surface, NULL);

        }


        wl_surface_commit(children[i].surface);
        wl_surface_commit(parent.surface);
    }
}

int main(int argc, char *argv[])
{
    srand(1);
    N = atoi(argv[1]);
    wait_ms = atoi(argv[2]);
    sprintf(fname,"%s_N_%d_MS_%d.txt",argv[3],N,atoi(argv[2]));
    fp = fopen(fname, "w");
    display = wl_display_connect(NULL);
	struct wl_registry *registry = wl_display_get_registry(display);
	wl_registry_add_listener(registry, &registry_listener, NULL);
	wl_display_roundtrip(display);
    wl_display_roundtrip(display);

    if (shm == NULL || compositor == NULL || xdg_wm_base == NULL || subcompositor == NULL)
    {
        //printf(stderr, "Missing globals.\n");
		return EXIT_FAILURE;
	}

    initParent();

    // Crea buffers para los hijos
    buffer_opaque = create_buffer(childSize*outputScale, childSize*outputScale);
    shm_opaque = wl_buffer_get_user_data(buffer_opaque);
    for(int i = 0; i < childSize*outputScale*childSize*outputScale*4; i+=4)
    {
        shm_opaque[i] = 0;
        shm_opaque[i+1] = 0;
        shm_opaque[i+2] = 255;
        shm_opaque[i+3] = 255;
    }

    wl_display_roundtrip(display);

    // Crea buffers para los hijos
    buffer_transparent = create_buffer(childSize*outputScale, childSize*outputScale);
    shm_transparent = wl_buffer_get_user_data(buffer_transparent);
    for(int i = 0; i < childSize*outputScale*childSize*outputScale*4; i+=4)
    {
        shm_transparent[i] = 255;
        shm_transparent[i+1] = 0;
        shm_transparent[i+2] = 0;
        shm_transparent[i+3] = 100;
    }

    wl_display_roundtrip(display);

    initChildren();

    wl_display_roundtrip(display);

    callback = wl_surface_frame(parent.surface);
    wl_callback_add_listener(callback, &wl_callback_listener, parent.surface);
    wl_surface_damage(parent.surface, 0,0,parent.width,parent.height);
    wl_surface_commit(parent.surface);

    for(int i = 0; i < N; i++)
    {

        wl_subsurface_set_position(children[i].subsurface, 0, 0);

        if(i % 2 == 0)
        {
            wl_surface_attach(children[i].surface, buffer_opaque, 0, 0);
            wl_surface_damage(children[i].surface, 0, 0, childSize, childSize);
        }
        else
        {
            wl_surface_attach(children[i].surface, buffer_transparent, 0, 0);
            wl_surface_damage(children[i].surface, 0, 0, childSize, childSize);
        }

        wl_surface_commit(children[i].surface);
    }


    while (wl_display_dispatch(display) != -1)
    {
		// This space intentionally left blank
	}

	return EXIT_SUCCESS;
}
