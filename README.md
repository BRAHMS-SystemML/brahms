# BRAHMS

BRAHMS is a simulation execution engine which was written by Ben
Michinson.  It's often used alongside SpineML/SpineML_2_BRAHMS to
provide a simulation back-end for SpineML/SpineCreator. The original
author is not currently developing BRAHMS, although it is in current
use within the Adaptive Behaviour Research Group at the Department of
Psychology of the University of Sheffield. As we expect to continue
its use, I have created this fork of the software on github.

This fork of the Brahms simulator starts from brahms version 0.7.3. The 
key improvement is a standard build system to replace the custom 
makefiles in the original, because recently we have been using "set in 
aspic" binary builds of BRAHMS and we would like to be able to easily 
modify and add features as our requirements of the software change.

You can read all about the original at:

http://brahms.sourceforge.net/docs/What%20is%20BRAHMS.html

The original code is also available at that site.

## What changed in this version?

The major change between this version and BRAHMS 0.7.3 is that the
requirement to set `SYSTEMML_INSTALL_PATH` has been removed. Instead, when
the software is compiled, a directory for the "installed SystemML Namespace"
is set - that might be `/usr/local/var/SystemML/Namespace`. That becomes
the default BRAHMS Namespace. It's in a `var` directory as you may decide
to install your own components in there.

You can also run BRAHMS without the progress window with the --nogui option.
This can be useful if you need to run hundreds of concurrent BRAHMS instances
on an HPC system and you forgot to log in with a non-X windows connection.

## What may change in future?

I may re-instate `SYSTEMML_INSTALL_PATH` such that you can have a second
installed Namespace, perhaps at `$HOME/SystemML/Namespace` which is referred
to with `SYSTEMML_INSTALL_PATH`.

I may implement a scheme for saving the delayed values stored in connections
which have some specified lag.

# Building BRAHMS

This project uses cmake for a cross-platform friendly build process. cmake
is a good choice of build system, because it is familiar to many developers.

You'll need to obtain the correct dependencies to build BRAHMS. On Linux that means a
compiler, libXv, libXaw and WX windows. For example, on (current 2015)
Ubuntu or Debian systems you'll want to do:

~~~ {.bash}
sudo apt-get install build-essential libxaw7-dev libxv-dev
~~~

These libraries are used to draw the progress box which shows while
BRAHMS is running.

For Windows, you'll reportedly need GnuWin32, but this build has not
yet been tested.

For optional features (MPI channel, Python and MATLAB bindings and
one WXWidgets-dependent component) you'll
need mpich2, python development libraries, WXwidgets and a MATLAB
installation. The first three can be obtained via apt-get:

~~~ {.bash}
sudo apt-get install mpich2 python-dev libwxgtk2.8-dev
~~~

The recommended method for building is to create a separate build
folder as follows:

~~~ {.bash}
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr/local .. # or cmake-gui ..
make -j4 # or however many processor cores you have
sudo make install
~~~

## Options for cmake

Here are the options you can set. Shown here are the defaults (at time of writing):

Adding/removing bindings:
~~~ {.bash}
cmake -DCOMPILE_MATLAB_BINDING=OFF -DCOMPILE_PYTHON_BINDING=ON ..
~~~

MPICH2:
~~~ {.bash}
cmake -DCOMPILE_WITH_MPICH2=ON ..
~~~

Compiling a component which requires WX windows:
~~~ {.bash}
cmake -DCOMPILE_WX_COMPONENT=OFF ..
~~~

Compiling with a specific installation prefix:
~~~ {.bash}
cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
~~~

--
Seb James, Feb 2015.
