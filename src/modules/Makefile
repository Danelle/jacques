CFLAGS = -I../jac `pkg-config --cflags gtk+-3.0`


all: hello


hello:
	gcc -shared -fPIC $(CFLAGS) hello.c -o hello.so


clean:
	rm -f *~ *.so

distclean: clean

install:


uninstall:
