.. image:: https://travis-ci.com/geobra/harbour-shmoose.svg?branch=master
    :target: https://travis-ci.com/geobra/harbour-shmoose

.. image:: https://codecov.io/gh/geobra/harbour-shmoose/branch/master/graph/badge.svg
  :target: https://codecov.io/gh/geobra/harbour-shmoose

.. image:: https://img.shields.io/lgtm/grade/cpp/g/geobra/harbour-shmoose.svg?logo=lgtm&logoWidth=18 
  :target: https://lgtm.com/projects/g/geobra/harbour-shmoose/context:cpp

.. image:: https://gitpod.io/button/open-in-gitpod.svg
  :target: https://gitpod.io/#https://github.com/geobra/harbour-shmoose

===============================================================================
Shmoose - XMPP Client for Sailfish OS
===============================================================================

Shmoose builds on and includes code from the following projects:

* XMPP library `Swiften <https://swift.im/swiften.html>`_ from Isode
* Image picker is from `hangish <https://github.com/rogora/hangish>`_ written by Daniele Rogora

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
* XEP-0045: Multi-User Chat [WIP]
* XEP-0280: Message Carbons [WIP]
* XEP-0198: Stream Management  [done]
* XEP-0384: OMEMO Encryption [WIP]

-------------------------------------------------------------------------------
Documentation of the Softwre Architecture
-------------------------------------------------------------------------------
`Software Architecture description <https://geobra.github.io/harbour-shmoose/>`_

-------------------------------------------------------------------------------
Ready-to-use binaries can be found on OpenRepos
-------------------------------------------------------------------------------
`Shmoose on OpenRepos <https://openrepos.net/content/schorsch/shmoose>`_

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
 * SSH into mersdk and do the following in a newly created directory

Fetch the Swift source source::

 * wget https://swift.im/downloads/releases/swift-4.0.2/swift-4.0.2.tar.gz
 * mkdir swift-4.0.2-arm
 * cd swift-4.0.2-arm
 * tar --strip-components=1 -xzvf ../swift-4.0.2.tar.gz

Install all dependencies to build Swiften::

 * sb2 -t SailfishOS-armv7hl -m sdk-install -R zypper in openssl-devel libiphb-devel libxml2-devel

Patch the SConstruct file to do a PIC build of the library archive

Add::

 * env.Append(CCFLAGS='-fPIC')

under the line 'env.SConscript = SConscript' on line 14

Build the Swiften Library::

 * sb2 -t SailfishOS-armv7hl /bin/bash ./scons Swiften

Fetch the Shmoose source code::

 * cd ..
 * git clone https://github.com/geobra/harbour-shmoose
 * cd harbour-shmoose
 * mb2 -t SailfishOS-armv7hl build


