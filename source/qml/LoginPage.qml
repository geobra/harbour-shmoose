import QtQuick 2.2
import QtQuick.Controls 1.2
import harbour.shmoose 1.0

Item {
    Column {
        Row {

            anchors
            {
                left: parent.left
                right: parent.right
            }


            TextField {
                id: jidField
                width: 150
                text: shmoose.getJid()
                placeholderText: "JID"
            }
            TextField {
                id: passField
                width: 150
                height: 27
                text: shmoose.getPassword()
                placeholderText: "Password"
                echoMode: TextInput.Password
            }
            Rectangle {
                 width: 30
                 height: 30
                 color: "#FFFF99"
                 radius: width*0.5
                 visible: true

                 Label {
                     text: "1"
                     anchors.horizontalCenter: parent.horizontalCenter
                     anchors.verticalCenter: parent.verticalCenter
                 }
            }
        }
        Button {
            id: connectButton
            text: "Connect"
            onClicked: {
                shmoose.saveCredentials(true);
                connectButton.enabled = false;
                shmoose.mainConnect(jidField.text, passField.text);
            }
        }
        Label {
            id: statusLabel
            text: "Not connected"
        }
    }
    Component.onCompleted: {
        function goToRoster() {
            statusLabel.text = "Connected";
            //we need to disconnect enableConnectButton to prevent calling it on normal disconnection
            shmoose.connectionStateDisconnected.disconnect(enableConnectButton)
            mainLoader.source = "ChatPage.qml"
        }
        function enableConnectButton() {
            connectButton.enabled = true
        }
        shmoose.connectionStateConnected.connect(goToRoster)
        shmoose.connectionStateDisconnected.connect(enableConnectButton)
    }
}



