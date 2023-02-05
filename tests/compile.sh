#!/bin/sh

meson setup build
meson configure -Db_sanitize=address build
ninja -C build
