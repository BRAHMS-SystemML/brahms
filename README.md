# BRAHMS

BRAHMS is a simulation execution engine which was written by Ben
Michinson.  It's often used alongside SpineML/SpineML_2_BRAHMS to
provide a simulation back-end for SpineML/SpineCreator. The original
author is not currently developing BRAHMS, although it is in current
use within the Adaptive Behaviour Research Group at the Department of
Psychology of the University of Sheffield. As we expect to continue
its use, I have created this fork of the software on github.

This fork of the Brahms simulator starts from brahms version
0.7.3. The key improvement is a standard build system to replace the
custom makefiles in the original, because at present we are using "set
in aspic" binary builds of BRAHMS and we would like to be able to
easily modify and add features as our requirements of the software
change.

You can read all about the original at

http://brahms.sourceforge.net/docs/What%20is%20BRAHMS.html

Seb James, Feb 2015.
