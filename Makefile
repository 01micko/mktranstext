CC?=cc
PREFIX?=/usr/local
MANDIR=man/man1
CFLAGS+=-Wall -pedantic -std=gnu99 $(shell pkg-config --cflags pangocairo)
LDFLAGS+=$(shell pkg-config --libs pangocairo)

all: mktranstext

mktranstext: mktranstext.o
	$(CC) -o $@ $^ $(LDFLAGS)

mkwallpaper.o: mkwallpaper.c
	$(CC) -o $@ $(CFLAGS) -c $^

install:
	install -d -m 0755 $(DESTDIR)$(PREFIX)/bin
	install -s -m 0755 mktranstext $(DESTDIR)$(PREFIX)/bin
	install -d -m 0755 $(DESTDIR)$(PREFIX)/$(MANDIR)
	install -m 0644 mktranstext.1.gz $(DESTDIR)$(PREFIX)/$(MANDIR)

clean:
	-rm -f *.o mktranstext
