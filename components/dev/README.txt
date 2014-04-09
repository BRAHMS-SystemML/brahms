
to build on unbuntu, you'll need:

sudo apt-get install
wx2.8-headers
wx2.8-i18n
libwxbase2.8-dev
libwxgtk2.8-dev

tried building my own wx, since the above keeps segfaulting. changes are:
	--disable-shared (build a static library)
	--with-gtk
	--enable-monolithic (this actually is no help at all i think, and may make linking much slower)

../configure --disable-shared --with-gtk --enable-monolithic


