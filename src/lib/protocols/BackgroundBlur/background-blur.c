/* Generated by wayland-scanner 1.23.0 */

/*
 * Copyright © 2025 Cuarzo Software
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include "wayland-util.h"

#ifndef __has_attribute
# define __has_attribute(x) 0  /* Compatibility with non-clang compilers. */
#endif

#if (__has_attribute(visibility) || defined(__GNUC__) && __GNUC__ >= 4)
#define WL_PRIVATE __attribute__ ((visibility("hidden")))
#else
#define WL_PRIVATE
#endif

extern const struct wl_interface background_blur_interface;
extern const struct wl_interface svg_path_interface;
extern const struct wl_interface wl_region_interface;
extern const struct wl_interface wl_surface_interface;

static const struct wl_interface *background_blur_types[] = {
	NULL,
	&background_blur_interface,
	&wl_surface_interface,
	&wl_region_interface,
	&svg_path_interface,
};

static const struct wl_message background_blur_manager_requests[] = {
	{ "destroy", "", background_blur_types + 0 },
	{ "get_background_blur", "no", background_blur_types + 1 },
};

WL_PRIVATE const struct wl_interface background_blur_manager_interface = {
	"background_blur_manager", 2,
	2, background_blur_manager_requests,
	0, NULL,
};

static const struct wl_message background_blur_requests[] = {
	{ "destroy", "", background_blur_types + 0 },
	{ "set_region", "?o", background_blur_types + 3 },
	{ "ack_configure", "u", background_blur_types + 0 },
	{ "set_path", "2?o", background_blur_types + 4 },
};

static const struct wl_message background_blur_events[] = {
	{ "state", "u", background_blur_types + 0 },
	{ "style", "u", background_blur_types + 0 },
	{ "configure", "u", background_blur_types + 0 },
};

WL_PRIVATE const struct wl_interface background_blur_interface = {
	"background_blur", 2,
	4, background_blur_requests,
	3, background_blur_events,
};

