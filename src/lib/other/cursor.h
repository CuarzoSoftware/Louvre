#include <LNamespaces.h>

#define LOUVRE_DEFAULT_CURSOR_WIDTH 64
#define LOUVRE_DEFAULT_CURSOR_HEIGHT 64
#define LOUVRE_DEFAULT_CURSOR_BPP 32
#define LOUVRE_DEFAULT_CURSOR_STRIDE (LOUVRE_DEFAULT_CURSOR_WIDTH * LOUVRE_DEFAULT_CURSOR_BPP) / 8

Louvre::UInt32 *louvre_default_cursor_data();
