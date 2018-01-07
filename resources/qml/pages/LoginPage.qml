import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.shmoose 1.0

Page {
    id: loginPage
    SilicaFlickable {
        anchors.fill: parent;
        contentHeight: loginPageContent.height

        Column {
            id: loginPageContent
            anchors {
                fill: parent;
                margins: Theme.paddingMedium;
            }

            width: loginPage.width
            spacing: Theme.paddingSmall

            PageHeader {
                title: qsTr("Welcome to Shmoose")
            }

            Label {
                x: Theme.paddingLarge
                width: parent.width
                wrapMode: Text.Wrap
                text: qsTr("Login to Jabber Server")
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeExtraLarge
            }

            TextField {
                id: jidTextField
                placeholderText: qsTr("jid@server.com")
                label: qsTr("JID")
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
                width: parent.width
                Switch {
                    id: saveCredentials
                    checked: shmoose.checkSaveCredentials()
                    onClicked: {
                        shmoose.saveCredentials(saveCredentials.checked);
                    }
                }
                Label {
                    wrapMode: Text.Wrap
                    width: credentialsRow.width - saveCredentials.width
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
                    connectButton.text = qsTr("Connecting...");
                    shmoose.mainConnect(jidTextField.text, passTextField.text);
                }

            }
        }
    }


    Connections {
        target: shmoose
        onConnectionStateChanged: {
            if (shmoose.connectionState == true) {
                pageStack.replace(pageMenu)
            }
            else {
                connectButton.enabled = true;
                connectButton.text = qsTr("Connect");
            }
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
