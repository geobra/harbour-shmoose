import QtQuick 2.0
import Sailfish.Silica 1.0

CoverBackground {
    Column {
        id: cover
        anchors.top: parent.top
        width: parent.width

        property bool xmppConnected: false

        Label {
            width: parent.width
            anchors.horizontalCenter: parent.horizontalCenter
            horizontalAlignment: Text.AlignHCenter
            font.family: Theme.fontFamilyHeading
            color: Theme.primaryColor
            text: "Shmoose"
        }
        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            color: Theme.primaryColor
            text: "---"
        }
        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            font.family: Theme.fontFamily
            color: mainWindow.hasInetConnection ? "green" : "red"
            text: mainWindow.hasInetConnection ? "online" : "offline"
        }
        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            font.family: Theme.fontFamily
            color: cover.xmppConnected ? "green" : "red"
            text: cover.xmppConnected ? "connected" : "disconnected"

            Connections {
                target: shmoose
                onConnectionStateConnected: {
                    cover.xmppConnected = true;
                }
                onConnectionStateDisconnected: {
                    cover.xmppConnected = false;
                }
            }
        }
        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "---"
        }
        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            color: "blue"
            font.pixelSize: Theme.fontSizeExtraLarge
            text: "0" // FIXME show unread messages
        }
    }
}
