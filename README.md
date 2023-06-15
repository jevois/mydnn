# mydnn
Sample custom C++ module for DNN processing on JeVois-Pro


INSTALLATION INSTRUCTIONS
=========================

First follow the instructions at http://www.jevois.org/doc/ProgrammerSource.html and https://jevois.usc.edu to install
the jevoispro-sdk-dev DEB package.

Then:

cd mydnn

JeVois-Pro: To compile and install for host:
--------------------------------------------

./rebuild-pro-host.sh


JeVois-Pro: To cross-compile for platform hardware and pack into a .deb package:
--------------------------------------------------------------------------------

./rebuild-pro-platform-pdeb.sh

this will create a .deb file for arm64 architecture (the processor of JeVois-Pro) that you can then copy to the
microSD of your JeVois-Pro and install on your running camera using the following command in the JeVois-Pro console:

!dpkg -i /location/of/debfile.deb
