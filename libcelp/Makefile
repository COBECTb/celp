

CFLAGS += -Wall -O4

LOBJECTS = celp.o

LSRC = $(LOBJECTS:.o=.c)

LIBS = -lm

libcelp.a: $(LOBJECTS)
	/bin/rm -f libcelp.a
	ar cr libcelp.a $(LOBJECTS)
	-if test -s /bin/ranlib; then /bin/ranlib libcelp.a; \
      else if test -s /usr/bin/ranlib; then /usr/bin/ranlib libcelp.a; \
	else exit 0; fi; fi

clean:
	rm -f $(PROGRAM) $(LOBJECTS) $(ARCH) core *.bak *.a

depend:
	makedepend -- $(CFLAGS) -- $(LSRC)


# DO NOT DELETE
