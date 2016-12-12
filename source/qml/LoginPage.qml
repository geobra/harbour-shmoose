import QtQuick 2.2
import QtQuick.Controls 1.2
import harbour.shmoose 1.0

Item {
    Column {
        Row {
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
            mainLoader.source = "RosterPage.qml"
        }
        function enableConnectButton() {
            connectButton.enabled = true
        }
        shmoose.connectionStateConnected.connect(goToRoster)
        shmoose.connectionStateDisconnected.connect(enableConnectButton)
    }
}



