
Build Notes
-----------

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



