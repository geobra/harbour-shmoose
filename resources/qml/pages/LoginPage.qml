import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.shmoose 1.0

Page {
    id: loginPage
    Column {
        anchors {
            fill: parent;
            right: avatar.left;
            margins: Theme.paddingMedium;
            verticalCenter: parent.verticalCenter;
        }

        PageHeader {
            title: qsTr("Login")
        }

        Label {
            x: Theme.paddingLarge
            text: qsTr("Login to Jabber Server")
            color: Theme.secondaryHighlightColor
            font.pixelSize: Theme.fontSizeExtraLarge
        }

        TextField {
            id: jidTextField
            placeholderText: qsTr("jid@server.com")
            label: qsTr("Jid")
            text: shmoose.getJid()
            width: parent.width

            onTextChanged: {
                checkEnableConnectButton();
            }
        }

        TextField {
            id: passTextField
            placeholderText: qsTr("password")
            echoMode: TextInput.Password
            label: qsTr("Password")
            text: shmoose.getPassword()
            width: parent.width

            onTextChanged: {
                checkEnableConnectButton();
            }
        }
        Row {
            id: credentialsRow
            Switch {
                id: saveCredentials
                checked: shmoose.checkSaveCredentials()
                onClicked: {
                    console.log("clicked cred " + saveCredentials.checked);
                    shmoose.saveCredentials(saveCredentials.checked);
                }
            }
            Label {
                text: qsTr("Save credentials (unencrypted)")
                font.pixelSize: Theme.fontSizeSmall
                anchors {
                    verticalCenter: parent.verticalCenter;
                }
            }
        }

        Button{
            id: connectButton
            text: qsTr("Connect")
            enabled: false

            onClicked: {
                connectButton.enabled = false;
                connectButton.text = "Connecting...";
                shmoose.mainConnect(jidTextField.text, passTextField.text);
            }

        }
    }


    Connections {
        target: shmoose
        onConnectionStateConnected: {
            pageStack.replace(pageMenu)
        }
        onConnectionStateDisconnected: {
            connectButton.enabled = true;
            connectButton.text = "Connect";
        }
    }

    function checkEnableConnectButton() {
        if (jidTextField.text.length > 0 && passTextField.text.length > 0) {
            connectButton.enabled = true;
        }
        else {
            connectButton.enabled = false;
        }
    }
}
