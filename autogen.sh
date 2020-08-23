#!/bin/sh
autoreconf -vfi || exit 1
./configure --prefix=/usr \
            --exec-prefix=/usr \
            --sysconfdir=/etc \
            --datarootdir=/usr/share \
            --libdir=/usr/lib \
            --bindir=/usr/bin \
            $* || exit 1
