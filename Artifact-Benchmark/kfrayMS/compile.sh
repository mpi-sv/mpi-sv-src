./configure 
make clean
make CC=mpisecc.sh CFLAGS="-g -O0"
rm build/bc/*.o > /dev/null 2>&1
rm build/bc/*.bc > /dev/null 2>&1
cp src/textures/*.o build/bc/
cp src/raycaster/*.o build/bc/
cp src/parallel/*.o build/bc/
cp src/objects/*.o build/bc/
cp src/models/*.o build/bc/
cp src/misc/*.o build/bc/
cp src/loader/*.o build/bc/
cp src/*.o build/bc/
rm build/bc/parser_yy.o > /dev/null 2>&1
#rm build/bc/raytracer.o > /dev/null 2>&1
llvm-link build/bc/*.o -o build/kfray.bc
