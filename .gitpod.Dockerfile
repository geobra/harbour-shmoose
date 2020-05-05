FROM gitpod/workspace-full

USER root

# Install custom tools, runtime, etc.
RUN apt-get update \
    && apt-get install -y build-essential pkg-config libssl-dev libhunspell-dev qt5-default libqt5sql5-sqlite qtdeclarative5-dev qml-module-qtquick2 qml-module-qtquick-controls \
    && rm -rf /var/lib/apt/lists/*

RUN wget https://swift.im/downloads/releases/swift-4.0.2/swift-4.0.2.tar.gz && tar -xvf swift-4.0.2.tar.gz && cd swift-4.0.2 && /bin/bash scons -j 4 Swiften && find . -type f -name "*.o" -exec rm -rf {} \; && cd .. && rm -f swift-4.0.2.tar.gz

