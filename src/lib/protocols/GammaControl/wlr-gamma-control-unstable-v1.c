/* Generated by wayland-scanner 1.20.0 */

/*
 * Copyright © 2015 Giulio camuffo
 * Copyright © 2018 Simon Ser
 *
 * Permission to use, copy, modify, distribute, and sell this
 * software and its documentation for any purpose is hereby granted
 * without fee, provided that the above copyright notice appear in
 * all copies and that both that copyright notice and this permission
 * notice appear in supporting documentation, and that the name of
 * the copyright holders not be used in advertising or publicity
 * pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */

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

extern const struct wl_interface wl_output_interface;
extern const struct wl_interface zwlr_gamma_control_v1_interface;

static const struct wl_interface *wlr_gamma_control_unstable_v1_types[] = {
	NULL,
	&zwlr_gamma_control_v1_interface,
	&wl_output_interface,
};

static const struct wl_message zwlr_gamma_control_manager_v1_requests[] = {
	{ "get_gamma_control", "no", wlr_gamma_control_unstable_v1_types + 1 },
	{ "destroy", "", wlr_gamma_control_unstable_v1_types + 0 },
};

WL_PRIVATE const struct wl_interface zwlr_gamma_control_manager_v1_interface = {
	"zwlr_gamma_control_manager_v1", 1,
	2, zwlr_gamma_control_manager_v1_requests,
	0, NULL,
};

static const struct wl_message zwlr_gamma_control_v1_requests[] = {
	{ "set_gamma", "h", wlr_gamma_control_unstable_v1_types + 0 },
	{ "destroy", "", wlr_gamma_control_unstable_v1_types + 0 },
};

static const struct wl_message zwlr_gamma_control_v1_events[] = {
	{ "gamma_size", "u", wlr_gamma_control_unstable_v1_types + 0 },
	{ "failed", "", wlr_gamma_control_unstable_v1_types + 0 },
};

WL_PRIVATE const struct wl_interface zwlr_gamma_control_v1_interface = {
	"zwlr_gamma_control_v1", 1,
	2, zwlr_gamma_control_v1_requests,
	2, zwlr_gamma_control_v1_events,
};
