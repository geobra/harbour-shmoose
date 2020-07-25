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
                    connectButton.enabled = false;
                    shmoose.mainConnect(jidField.text, passField.text);
                }
            }
        }
    }

    footer: TextEdit {
        width: 240
        text: "Hello World!"
        font.family: "Helvetica"
        font.pointSize: 10
    }

    RowLayout {
        id: layout
        anchors.fill: parent
        spacing: 6

        Rectangle {
            color: 'teal'
            Layout.minimumWidth: 200
            Layout.fillHeight: true
            ListView {
                id: rosterListView
                width: 200
                height: parent.height

                model: shmoose.rosterController.rosterList
                //model: rosterList
                delegate: Rectangle {
                    height: 25
                    width: parent.width
                    color: "yellow"
                    Text { text: jid}

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            rosterListView.currentIndex = index
                            console.log( "jid: " + rosterListView.currentIndex)
                            console.log( "jid: " + rosterList[rosterListView.currentIndex].jid)

                            //shmoose.setCurrentChatPartner(shmoose.rosterController.rosterList[roster.currentIndex].jid)
                        }
                    }
                }
            }
        }

        Rectangle {
            color: 'lightblue'
            Layout.fillWidth:true
            Layout.fillHeight: true
            ListView {
                height: parent.height
                //model: lottoNumbers
                //delegate: Text { text: number }
            }
        }

    }

}


