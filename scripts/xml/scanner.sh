
# Usage: ./scanner.sh <protocol name without .xml>

wayland-scanner server-header < "$1.xml" > "$1.h"
wayland-scanner private-code < "$1.xml" > "$1.c"
