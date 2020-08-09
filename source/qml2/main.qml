import QtQuick.Layouts 1.2
import QtQuick 2.3
import QtQuick.Controls 2.4

ApplicationWindow {
    visible: true

    minimumWidth: 800
    minimumHeight: 600

    menuBar: MenuBar {
        Menu {
            title: qsTr("&File")
            Action { text: qsTr("&New...") }
            Action { text: qsTr("&Quit") }
        }
        Menu {
            title: qsTr("&Edit")
            Action { text: qsTr("&Copy") }
            Action { text: qsTr("&Paste") }
        }
        Menu {
            title: qsTr("&Help")
            Action { text: qsTr("&About") }
        }
    }

    header: ToolBar {
        RowLayout {
            anchors.fill: parent

            TextField {
                id: jidField
                Layout.fillWidth: true
                text: shmoose.settings.Jid
                placeholderText: "JID"
            }
            TextField {
                id: passField
                Layout.fillWidth: true
                text: shmoose.settings.Password
                placeholderText: "Password"
                echoMode: TextInput.Password
            }

            Button {
                id: connectButton
                text: "Connect"
                onClicked: {
                    shmoose.settings.SaveCredentials = true;
                    //connectButton.enabled = false;
                    shmoose.mainConnect(jidField.text, passField.text);
                }
            }
        }
    }

    footer: TextEdit {
        id: footer
        width: 240
        text: "Hello World!"
        font.family: "Helvetica"
        font.pointSize: 10
    }

    RowLayout {
        id: layout
        anchors.fill: parent
        spacing: 6

        ContactsView {
            Layout.minimumWidth: 100
            Layout.preferredWidth: 200
            Layout.fillHeight: true
        }

        ColumnLayout {
            MessageView {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            SendView {
                Layout.fillWidth: true
                Layout.preferredHeight: 50
            }
        }
    }

    Connections {
        target: shmoose
        onConnectionStateChanged: {
            if (shmoose.connectionState == true) {
                footer.text = "Connected";
                connectButton.enabled = false;

                shmoose.setAppIsActive(true)
                shmoose.setCurrentChatPartner("")
            }
            else {
                footer.text = "DISconnected"

                shmoose.setAppIsActive(false)
                connectButton.enabled = true;
            }
        }
    }

    Connections {
        target: shmoose
        onSignalShowStatus: {
            footer.text = headline + " " + body
        }
    }
}


