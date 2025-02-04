default: src/SunriseSirenStudio.c include/UI.h
	if [ ! -d "./build" ]; then mkdir build; fi

	glib-compile-schemas schema/
	gcc -o build/sunrise-siren-studio src/SunriseSirenStudio.c -lm `pkg-config --cflags --libs gtk+-3.0 libcurl json-c`

clean:
	rm -rf build
	rm include/UI.h
	rm schema/gschemas.compiled

test-install: build/sunrise-siren-studio
	cp build/sunrise-siren-studio ~/.local/bin
