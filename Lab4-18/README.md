gcc -fPIC -shared lib1/prime.c lib1/e.c -lm -o lib1/libimpl1.so
gcc -fPIC -shared lib2/prime.c lib2/e.c -o lib2/libimpl2.so
gcc program1.c -Llib1 -limpl1 -lm -o program1
gcc program2.c -ldl -o program2


export LD_LIBRARY_PATH=./lib1
