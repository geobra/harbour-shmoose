import QtQuick 2.0
import Sailfish.Silica 1.0

CoverBackground {
    Column {
        id: cover


        anchors.top: parent.top
        width: parent.width

        property bool xmppConnected: false
        property int unreadMessages: 0

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
            text: mainWindow.hasInetConnection ? qsTr("online") : qsTr("offline")
        }
        Label {
            id: appConnectedId
            anchors.horizontalCenter: parent.horizontalCenter
            font.family: Theme.fontFamily
            color: "red"
            text: "disconnected"

            Connections {
                target: shmoose
                onSignalAppIsOnline: {
                    if (connected === true) {
                        appConnectedId.color = "green";
                        appConnectedId.text = qsTr("connected");
                    }
                    else {
                        appConnectedId.color = "red";
                        appConnectedId.text = qsTr("disconnected");
                    }
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
            text: cover.unreadMessages
        }

        Connections {
            target: shmoose.persistence.messageController
            onSignalMessageReceived: {
                if (applicationActive == false) {
                    cover.unreadMessages++
                }
            }
        }

        Connections {
            target: mainWindow
            onAppGetsActive: {
                cover.resetUnreadMessages()
            }
        }

        function resetUnreadMessages() {
            cover.unreadMessages = 0
        }

    }
}
