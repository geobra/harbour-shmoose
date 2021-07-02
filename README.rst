.. image:: https://github.com/geobra/harbour-shmoose/workflows/build_shmoose/badge.svg
    :target: https://github.com/geobra/harbour-shmoose/actions

.. image:: https://codecov.io/gh/geobra/harbour-shmoose/branch/master/graph/badge.svg
  :target: https://codecov.io/gh/geobra/harbour-shmoose

.. image:: https://img.shields.io/lgtm/grade/cpp/g/geobra/harbour-shmoose.svg?logo=lgtm&logoWidth=18 
  :target: https://lgtm.com/projects/g/geobra/harbour-shmoose/context:cpp

.. image:: https://hosted.weblate.org/widgets/shmoose/-/svg-badge.svg
  :target: https://hosted.weblate.org/engage/shmoose

.. image:: https://gitpod.io/button/open-in-gitpod.svg
  :target: https://gitpod.io/#https://github.com/geobra/harbour-shmoose

===============================================================================
Shmoose - XMPP Client for Sailfish OS
===============================================================================

Shmoose builds on and includes code from the following projects:

* XMPP library `Swiften <https://swift.im/swiften.html>`_ from Isode
* Omemo library `libomemo <https://github.com/gkdr/libomemo>`_ written by Richard Bayerle
* `Axc <https://github.com/gkdr/axc>`_ library written by Richard Bayerle
* `Lurch <https://github.com/gkdr/lurch>`_ plugin written by Richard Bayerle
* Image picker is from `hangish <https://github.com/rogora/hangish>`_ written by Daniele Rogora
* Image zoom and pitch page is from `harbour-one <https://github.com/0312birdzhang/harbour-one>`_ written by 0312birdzhang

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
* XEP-0313: Message Archive Management [partial]
* XEP-0359: Unique and Stable Stanza IDs [WIP]
* XEP-0045: Multi-User Chat [WIP]
* XEP-0280: Message Carbons [WIP]
* XEP-0198: Stream Management  [done]
* XEP-0384: OMEMO Encryption [WIP]

-------------------------------------------------------------------------------
Documentation of the Software Architecture
-------------------------------------------------------------------------------
`Software Architecture description <https://geobra.github.io/harbour-shmoose/>`_

-------------------------------------------------------------------------------
Ready-to-use binaries can be found on OpenRepos
-------------------------------------------------------------------------------
`Shmoose on OpenRepos <https://openrepos.net/content/schorsch/shmoose>`_

Note! starting with Sailfish OS version 3.3.0.16 you need to adjust the suspend settings of your device::

 * pkcon refresh
 * pkcon install mce-tools
 * mcetool -searly

This disables late suspend of the device. Battery drain is only slightly more at my device with this setting. This setting is persistent over reboots and has to be done only once. Without this, the longterm TCP connection will drop after a short amount of time and you won't get notifications on new messages.

-------------------------------------------------------------------------------
Jump direct into development on gitpod
-------------------------------------------------------------------------------
`Ready to use development environment, running in your browser <https://gitpod.io/#https://github.com/geobra/harbour-shmoose>`_

-------------------------------------------------------------------------------
Build a x86 binary locally for development purposes
-------------------------------------------------------------------------------

On Linux|GNU, do the following:

Create a working directory::

 * mkdir src
 * cd src

Fetch the Swift source::

 * wget https://swift.im/downloads/releases/swift-4.0.2/swift-4.0.2.tar.gz
 * tar -xzvf swift-4.0.2.tar.gz
 * cd swift-4.0.2/

Install all dependencies to build Swiften::

 * ./BuildTools/InstallSwiftDependencies.sh
 * ./scons Swiften -j<Number of threads>

Install dependencies to build Shmoose (example for Debian)::

 * sudo apt-get install zlib1g-dev libssl-dev libxml2-dev libstdc++-5-dev libqt5quick5 libqt5quickparticles5 libqt5quickwidgets5 libqt5qml5 libqt5network5 libqt5gui5 libqt5core5a qt5-default libglib2.0-dev libpthread-stubs0-dev libmxml-dev libgcrypt20-dev libglib2.0-dev libsqlite3-dev

Fetch the Shmoose source code::

 * cd ..
 * git clone https://github.com/geobra/harbour-shmoose

Either::

 * Open the project file (.pro) within Qt Creator

or use command-line::

 * cd harbour-shmoose
 * qmake
 * make -j<Number of threads>

-------------------------------------------------------------------------------
To cross compile for Sailfish OS, do the following:
-------------------------------------------------------------------------------

 * Get and install Sailfish OS mersdk (tested with version 1608)
 * SSH into mersdk and do the following

Hint::

Use 'sb2-config -l' to show available targets for sb2.

Install all dependencies to build Swiften and shmoose::

 * sb2 -t SailfishOS-3.3.0.16-armv7hl -m sdk-install -R zypper in openssl-devel libiphb-devel libxml2-devel libgpg-error-devel libgcrypt-devel sqlite-devel cmake python

Fetch the Swift source source::

 * wget https://swift.im/downloads/releases/swift-4.0.2/swift-4.0.2.tar.gz
 * mkdir swift-4.0.2-arm
 * cd swift-4.0.2-arm
 * tar --strip-components=1 -xzvf ../swift-4.0.2.tar.gz

Patch the SConstruct file to do a PIC build of the library archive

Add::

 * env.Append(CCFLAGS='-fPIC')

under the line 'env.SConscript = SConscript' on line 14

Build the Swiften Library::

 * sb2 -t SailfishOS-armv7hl /bin/bash ./scons Swiften
 * cd ..

Install mxml::

 * curl -L -O https://github.com/michaelrsweet/mxml/releases/download/v3.2/mxml-3.2.tar.gz
 * tar -xvf mxml-3.2.tar.gz && cd mxml-3.2
 * sb2 -t SailfishOS-3.3.0.16-armv7hl ./configure
 * sb2 -t SailfishOS-3.3.0.16-armv7hl make
 * cp libmxml.a /srv/mer/targets/SailfishOS-3.3.0.16-armv7hl/usr/local/lib/
 * cp mxml.h /srv/mer/targets/SailfishOS-3.3.0.16-armv7hl/usr/local/include/
 * PKG_CONFIG_PATH=$(pwd)
 * cd ..

Fetch the Shmoose source code::

 * git clone https://github.com/geobra/harbour-shmoose
 * cd harbour-shmoose

Install libomemo::

 * git clone https://github.com/gkdr/libomemo && cd libomemo
 * git checkout tags/v0.7.0
 * sb2 -t SailfishOS-3.3.0.16-armv7hl make
 * cd ..

Install axc and libsignal-protocol-c::

 * git clone https://github.com/gkdr/axc && cd axc
 * git checkout tags/v0.3.3
 * git submodule update --init
 * sb2 -t SailfishOS-3.3.0.16-armv7hl make
 * cd  lib/libsignal-protocol-c/
 * add 'set(CMAKE_POSITION_INDEPENDENT_CODE ON)' to CMakeLists.txt
 * mkdir build && cd build
 * sb2 -t SailfishOS-3.3.0.16-armv7hl cmake ..
 * sb2 -t SailfishOS-3.3.0.16-armv7hl make
 * cd ../../../..

Finally, build Shmoose::

 * mb2 -t SailfishOS-3.3.0.16-armv7hl build

