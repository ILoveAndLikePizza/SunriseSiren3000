default: SunriseSirenStudio.c UI.h
	if [ ! -d "./build" ]; then mkdir build; fi
	gcc -o build/sunrise-siren-studio SunriseSirenStudio.c `pkg-config --cflags --libs gtk+-3.0 libcurl`

clean:
	rm UI.h
	rm -rf build

test-install: build/sunrise-siren-studio
	cp build/sunrise-siren-studio ~/.local/bin
