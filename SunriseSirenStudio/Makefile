default: src/SunriseSirenStudio.c include/UI.h
	if [ ! -d "./build" ]; then mkdir build; fi
	gcc -o build/sunrise-siren-studio src/SunriseSirenStudio.c -lm `pkg-config --cflags --libs gtk+-3.0 libcurl json-c`

schema: schema/org.x.sunrise-siren-studio.credentials.gschema.xml
	glib-compile-schemas schema/

clean:
	rm -rf build
	rm include/UI.h
	rm schema/gschemas.compiled

install: build/sunrise-siren-studio schema/org.x.sunrise-siren-studio.credentials.gschema.xml icon/sunrise-siren-studio.png
	cp build/sunrise-siren-studio /usr/bin
	cp schema/org.x.sunrise-siren-studio.credentials.gschema.xml /usr/share/glib-2.0/schemas
	cp icon/sunrise-siren-studio.png /usr/share/icons/hicolor/48x48/apps
	# todo: .desktop file
	glib-compile-schemas /usr/share/glib-2.0/schemas

uninstall: /usr/bin/sunrise-siren-studio
	rm /usr/bin/sunrise-siren-studio
	rm /usr/share/glib-2.0/schemas/org.x.sunrise-siren-studio.credentials.gschema.xml
	rm /usr/share/icons/hicolor/48x48/apps/sunrise-siren-studio.png
	# todo: .desktop file
	glib-compile-schemas /usr/share/glib-2.0/schemas
