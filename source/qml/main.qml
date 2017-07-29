import QtQuick 2.1
import QtQuick.Controls 1.2
import harbour.shmoose 1.0

ApplicationWindow {
    visible: true
    width: 800
    height: 400

    Loader {
        id: mainLoader
        anchors.fill: parent
        source: "LoginPage.qml"
    }

    onClosing: shmoose.mainDisconnect()
}


