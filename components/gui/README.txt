
to build on unbuntu, you'll need:

sudo apt-get install
wx2.8-headers
wx2.8-i18n
libwxbase2.8-dev
libwxgtk2.8-dev

this is a shared library build, which may not work with our shared libraries. in
any case we don't want to distribute a dll. so, tried building my own wx. options
were:

	--disable-shared (build a static library)
	--with-gtk
	--enable-monolithic (this actually is no help at all i think, and may make linking much slower)
	flags: these may be needed on some environments, can't do any harm anyway

../configure --disable-shared --with-gtk --enable-monolithic CFLAGS="-fPIC" CXXFLAGS="-fPIC"


