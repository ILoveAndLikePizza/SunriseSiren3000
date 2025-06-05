#!/bin/bash

DIR=$(dirname -- "$(readlink -f -- "$0")");
VERSION=$(sed -nE 's/#define VERSION "v(.*?)"/\1/p' "$DIR/../include/SunriseSirenStudio.h")
DPKG_NAME="SunriseSirenStudio_${VERSION}_$(dpkg --print-architecture)"

if [ ! -f "$DIR/../build/sunrise-siren-studio" ]; then
    echo "Sunrise Siren Studio has not been compiled yet. Please do that first."
    exit 1
fi

BUILD_TARGET="$DIR/../build/$DPKG_NAME"

if [ -d "$BUILD_TARGET" ]; then
    rm -rf "$BUILD_TARGET"
fi

mkdir -p "$BUILD_TARGET/DEBIAN"
mkdir -p "$BUILD_TARGET/usr/bin"
mkdir -p "$BUILD_TARGET/usr/share/applications"
mkdir -p "$BUILD_TARGET/usr/share/glib-2.0/schemas"
mkdir -p "$BUILD_TARGET/usr/share/icons/hicolor/256x256@2/apps"
mkdir -p "$BUILD_TARGET/usr/share/man/man1"

cp "$DIR/../build/sunrise-siren-studio" "$BUILD_TARGET/usr/bin"
cp "$DIR/../dist/sunrise-siren-studio.desktop" "$BUILD_TARGET/usr/share/applications"
cp "$DIR/../dist/sunrise-siren-studio.png" "$BUILD_TARGET/usr/share/icons/hicolor/256x256@2/apps"
cp "$DIR/../schema/org.x.sunrise-siren-studio.credentials.gschema.xml" "$BUILD_TARGET/usr/share/glib-2.0/schemas"
cp "$DIR/../man/sunrise-siren-studio.1.gz" "$BUILD_TARGET/usr/share/man/man1"

echo "Package: sunrise-siren-studio
Version: $VERSION
Section: utils
Priority: optional
Installed-Size: $(du -s "$BUILD_TARGET/usr" | awk '{print $1}')
Architecture: $(dpkg --print-architecture)
Depends: libgtk-3-common,libcurl4,libjson-c5
Maintainer: ngkon
Description: Control program for Sunrise Siren 3000" > "$BUILD_TARGET/DEBIAN/control"

echo "#!/bin/sh
gtk-update-icon-cache /usr/share/icons/hicolor
glib-compile-schemas /usr/share/glib-2.0/schemas" > "$BUILD_TARGET/DEBIAN/postinst"

chmod +x "$BUILD_TARGET/DEBIAN/postinst"

dpkg-deb --build "$BUILD_TARGET"
