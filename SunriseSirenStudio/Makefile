default: src/SunriseSirenStudio.c include/UI.h
	if [ ! -d "./build" ]; then mkdir build; fi
	gcc -o build/sunrise-siren-studio src/SunriseSirenStudio.c -lm `pkg-config --cflags --libs gtk+-3.0 libcurl json-c`

gschema: schema/org.x.sunrise-siren-studio.credentials.gschema.xml
	glib-compile-schemas schema/

clean:
	rm -rf build
	rm include/UI.h
	rm schema/gschemas.compiled

install: build/sunrise-siren-studio schema/org.x.sunrise-siren-studio.credentials.gschema.xml dist/sunrise-siren-studio.desktop dist/sunrise-siren-studio.png
	cp build/sunrise-siren-studio /usr/bin
	cp schema/org.x.sunrise-siren-studio.credentials.gschema.xml /usr/share/glib-2.0/schemas
	cp dist/sunrise-siren-studio.desktop /usr/share/applications
	cp dist/sunrise-siren-studio.png /usr/share/icons/hicolor/256x256@2/apps

	gtk-update-icon-cache /usr/share/icons/hicolor
	glib-compile-schemas /usr/share/glib-2.0/schemas

uninstall: /usr/bin/sunrise-siren-studio
	rm /usr/bin/sunrise-siren-studio
	rm /usr/share/glib-2.0/schemas/org.x.sunrise-siren-studio.credentials.gschema.xml
	rm /usr/share/applications/sunrise-siren-studio.desktop
	rm /usr/share/icons/hicolor/256x256@2/apps/sunrise-siren-studio.png

	gtk-update-icon-cache /usr/share/icons/hicolor
	glib-compile-schemas /usr/share/glib-2.0/schemas
