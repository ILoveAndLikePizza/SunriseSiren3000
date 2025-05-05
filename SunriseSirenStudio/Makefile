PREFIX ?= /usr
BINDIR ?= $(PREFIX)/bin
SHAREDIR ?= $(PREFIX)/share

default: src/SunriseSirenStudio.c schema/org.x.sunrise-siren-studio.credentials.gschema.xml
	ui/minify-ui.sh
	if [ ! -f "./include/UI.h" ]; then echo "ui/minify.sh failed" && exit 1; fi

	mkdir -p build
	gcc -o build/sunrise-siren-studio src/SunriseSirenStudio.c `pkg-config --cflags --libs gtk+-3.0 libcurl json-c` -lm

	glib-compile-schemas schema/

install:
	mkdir -p $(BINDIR) $(SHAREDIR)/glib-2.0/schemas $(SHAREDIR)/applications $(SHAREDIR)/icons/hicolor/256x256@2/apps

	cp build/sunrise-siren-studio $(BINDIR)
	cp schema/org.x.sunrise-siren-studio.credentials.gschema.xml $(SHAREDIR)/glib-2.0/schemas
	cp dist/sunrise-siren-studio.desktop $(SHAREDIR)/applications
	cp dist/sunrise-siren-studio.png $(SHAREDIR)/icons/hicolor/256x256@2/apps

	gtk-update-icon-cache $(SHAREDIR)/icons/hicolor
	glib-compile-schemas $(SHAREDIR)/glib-2.0/schemas

uninstall:
	rm $(BINDIR)/sunrise-siren-studio
	rm $(SHAREDIR)/glib-2.0/schemas/org.x.sunrise-siren-studio.credentials.gschema.xml
	rm $(SHAREDIR)/applications/sunrise-siren-studio.desktop
	rm $(SHAREDIR)/icons/hicolor/256x256@2/apps/sunrise-siren-studio.png

	gtk-update-icon-cache $(SHAREDIR)/icons/hicolor
	glib-compile-schemas $(SHAREDIR)/glib-2.0/schemas

	dconf reset -f /org/x/sunrise-siren-studio/credentials/

deb:
	dist/make-deb.sh

clean:
	rm -rf build
	rm include/UI.h
	rm schema/gschemas.compiled
