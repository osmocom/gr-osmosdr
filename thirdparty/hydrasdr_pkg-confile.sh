#!/usr/bin/bash


cp -R libhydrasdr /usr/include/

sudo tee /usr/local/lib/pkgconfig/libhydrasdr.pc << EOF
prefix=/usr/local
exec_prefix=\${prefix}
libdir=\${exec_prefix}/lib
includedir=\${prefix}/include

Name: libhydrasdr
Description: HydraSDR Library
Version: 1.0
Libs: -L\${libdir} -lhydrasdr
Cflags: -I\${includedir}
EOF
