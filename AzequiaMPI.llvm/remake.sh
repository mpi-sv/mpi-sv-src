make CC=clang CLINKER=clang CFLAGS="-g -flto -c" CLINKFLAGS="-lglib-2.0 -flto -WI -plugin-opt=also-emit-llvm" AR="llvm-ar" RANLIB="llvm-ranlib" -j 
