
# Usage: ./scanner.sh <protocol name without .xml>

wayland-scanner client-header < "$1.xml" > "$1-client.h"
wayland-scanner server-header < "$1.xml" > "$1.h"
wayland-scanner private-code < "$1.xml" > "$1.c"
