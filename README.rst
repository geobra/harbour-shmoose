===============================================================================
Shmoose - XMPP Client for Sailfish OS
===============================================================================

Shmoose builds on and includes code from the following projects:

* XMPP library `Swiften <https://swift.im/swiften.html>`_ from Isode
* Image picker is from `hangish <https://github.com/rogora/hangish>`_ written by Daniele Rogora
* Some files from `libpurple <https://developer.pidgin.im/>`_ to implement the plugin mock
* Omemo by libpurple plugin `lurch <https://github.com/gkdr/lurch>`_ to have state-of-the-art encryption

-------------------------------------------------------------------------------
Feature Stack until version 1.0
-------------------------------------------------------------------------------

* Login to Jabber server [done]
* Get roster [done]
* Send and receive messages [done]
* Notification on new messages [done]
* XEP-0184: Message Delivery Receipts [done]
* XEP-0199: XMPP Ping [done]
* XEP-0363: HTTP File Upload [done]
* XEP-0333: Chat Markers [done]
* XEP-0045: Multi-User Chat [partial]
* XEP-0198: Stream Management  [partial]
* OMEMO Multi-End Message and Object Encryption [WIP]
* Add, edit and delete roster items [partial]
* Finalize database [WIP]

-------------------------------------------------------------------------------
Ready to use binaries can be found on openrepos
-------------------------------------------------------------------------------
`Shmoose on openrepos <https://openrepos.net/content/schorsch/shmoose>`_

-------------------------------------------------------------------------------
Installation on debian stable
-------------------------------------------------------------------------------

On Linux do the following:

Create a working directory::

 * mkdir src
 * cd src

Prepare swift source::

 * wget https://swift.im/downloads/releases/swift-3.0/swift-3.0.tar.gz
 * tar -xzvf swift-3.0.tar.gz
 * mv swift-3.0 swift-3.0-host
 * cd swift-3.0-host

Install all dependencies and build swiften::

 * ./BuildTools/InstallSwiftDependencies.sh
 * ./scons Swiften -j<Number of threads>

Install dependencies to build Shmoose (example for Debian)::

 * sudo apt-get install zlib1g-dev libssl-dev libxml2-dev libstdc++-6-dev libqt5quick5 libqt5quickparticles5 libqt5quickwidgets5 libqt5qml5 libqt5network5 libqt5gui5 libqt5core5a qt5-default libglib2.0-dev libpthread-stubs0-dev libsqlite3-dev gcc g++ make libgcrypt20-dev libmxml-dev cmake

Get Shmoose source code::

 * cd ..
 * git clone https://github.com/geobra/harbour-shmoose

Switch to omemo branch::

 * cd harbour-shmoose/
 * git checkout xep-0384

Fetch and compile libraries for omemo::

 * git submodule update --init --recursive
 * cd lib/axc
 * make
 * cd lib/libsignal-protocol-c
 * mkdir build
 * cd build
 * cmake ..
 * make
 * cd ../../../../libomemo/
 * make
 * cd ../..

Either::

 * open pro file within qtreator

or use command line::

 * mkdir build
 * cd build
 * qmake ..
 * make -j<Number of threads>

-------------------------------------------------------------------------------
Installation on SFOS
-------------------------------------------------------------------------------

To cross compile for Sailfish OS, do the following::

 * Get and install Sailfish OS mersdk (tested with version 1608)
 * Ssh into mersdk and do the following

Fetch and compile packages which does not exists for SFOS or are too old::

 * libgpg-error-1.13
 * libgcrypt-1.7.6 (--enable-static --with-pic)
 * mxml-2.10

Build with this process::

 * tar -xvf ...tar.gz
 * sb2 -t SailfishOS-armv7hl ./configure
 * sb2 -t SailfishOS-armv7hl make

 * sb2 -R -t SailfishOS-armv7hl make install (only for libgpg-error!)
 * cp libgcrypt.a libmxml.a  /srv/mer/targets/SailfishOS-armv7hl/usr/local/lib/ 

Fetch swift source::

 * wget https://swift.im/downloads/releases/swift-3.0/swift-3.0.tar.gz
 * tar -xzvf swift-3.0.tar.gz
 * cd swift-3.0/

Install all dependencies to build swiften::

 * sb2 -t SailfishOS-armv7hl -m sdk-install -R zypper in openssl-devel libiphb-devel glib2-devel sqlite-devel cmake 

Patch SConstruct file to do a PIC build of the library archive

Add::

 * env.Append(CCFLAGS='-fPIC')

under the line 'env.SConscript = SConscript' on line 14

Build Swiften Library::

 * sb2 -t SailfishOS-armv7hl /bin/bash ./scons Swiften

Follow 'Get shmoose soure code' as on host guide but::

 * move all omemo build dirs to build-host
 * prepend the build commands with 'sb2 -t SailfishOS-armv7hl'
 * move all build dirs to build-arm
 * patch libsignal-protocol-c to make a PIC build (CMakeLists.txt:23: add -fPIC to CMAKE_C_FLAGS)


