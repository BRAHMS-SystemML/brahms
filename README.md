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
makefiles in the original, because at present we are using "set in 
aspic" binary builds of BRAHMS and we would like to be able to easily 
modify and add features as our requirements of the software change. All 
of the old, custom-written makefiles are present with an "__" prefix. 
These will be removed at some point, but I'm keeping them for reference 
at present.

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

OLD Build Notes for reference
-----------------------------

To build each part, enter each project directory and
run "make". On Windows, you will have to have GnuWin32
installed. To build all parts, run "make" in this folder,
and review the options.

The only pre-requisite (other than compiler, libraries,
and all required INCLUDE and LIB paths set) is that you
should have set the env var "SYSTEMML_INSTALL_PATH" to
the folder you want to install to, or to the folder
where your SystemML/BRAHMS installation currently is. If you
installed BRAHMS using an installer, this may have been
done for you. If you are building from scratch without a
current install, this probably needs doing manually.

As well as the base development libraries on Linux the
dev versions of libXv and libXaw are required.

You will need subversion installed, because the makefiles
use it to get the revision number, even if you're building
from an exported branch. We'll fix this one day.

On Linux, you will need the env var SYSTEMML_MATLAB_PATH
set to your Matlab root directory. On our system, it's
"/usr/local/encap/matlab-R2007B". On Windows, you will
need to make sure the Matlab include and bin folders are
in your INCLUDE and LIB env vars, respectively.



Compilers
---------

We use G++ 4.2 on linux, or cl v14 on windows. Other
compilers should work fine, but your binaries will be
incompatible with ours. That won't usually matter, since
you'll build all the binaries yourself.

--
Seb James, Feb 2015.
