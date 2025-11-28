
all: lib
	$(CC) main.c -L./ ./libcelp/libcelp.a -lm  -o celp
lib:
	make -C ./libcelp
libclean:
	make -C ./libcelp clean
clean: libclean
	rm -rf *.o celp
