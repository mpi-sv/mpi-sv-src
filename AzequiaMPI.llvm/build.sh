#/usr/bin/sh!
./configure --prefix=/usr/local/azeqmpi --with-device=none --with-model=blk --disable-hwloc --enable-profiling CFLAGS="-g -O0" && make CC=clang CLINKER=clang CFLAGS="-g -O0 -flto -c" CLINKFLAGS="-lglib-2.0 -flto -WI -plugin-opt=also-emit-llvm" AR="llvm-ar" RANLIB="llvm-ranlib" -j
