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
                            console.log( "jid: " + shmoose.rosterController.rosterList[rosterListView.currentIndex].jid)

                            shmoose.setCurrentChatPartner(shmoose.rosterController.rosterList[rosterListView.currentIndex].jid)
                        }
                    }
                }
            }
        }

        Rectangle {
            color: 'grey'
            Layout.fillWidth: true
            Layout.fillHeight: true
            ListView {
                height: parent.height
                width: parent.width

                model: shmoose.persistence.messageController
                delegate: Item {
                        id: item

                        width: parent.width
                        //height: parent.height
                        height: messageText.height// + msgStatus.height
                        //width: messageText.width



                        readonly property bool alignRight: (direction == 1);

                        Column {

                            //width: parent.width - 50
                            //spacing: 20

                            Rectangle {

                                width: item.width
                                height: item.height
                                //height: messageText.height + msgStatus.height
                                //radius: margin

                                color: (alignRight ? "green" : "lightblue")

                                /*
                                anchors {
                                    left: (direction == 0 ? item.right : undefined);
                                    right: (direction == 1 ? item.left : undefined);
                                    //margins: 10
                                }
                                */



                                Text {
                                    id: messageText
                                    //horizontalAlignment: (item.alignRight ? Text.AlignLeft : Text.AlignRight)
                                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere;
                                    //width: parent.width


                                    text: message

                                    /*
                                    // make a hand over the link
                                    MouseArea {
                                        anchors.fill: parent
                                        //acceptedButtons: Qt.NoButton // we don't want to eat clicks on the Text
                                        //cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
                                        onClicked:  {
                                            shmoose.persistence.gcmController.setFilterOnMsg(id);
                                            console.log("seen by: " + shmoose.persistence.getResourcesOfNewestDisplayedMsgforJid(shmoose.rosterController.rosterList[roster.currentIndex].jid));
                                        }
                                    }
                                    */

                                    //onLinkActivated: Qt.openUrlExternally(link)
                                }

                                /*
                                Image {
                                    id: msgStatus
                                    //source: "img/check.png"

                                    anchors.right: parent.right
                                    anchors.bottom: parent.bottom

                                    source: {
                                        if (msgstate == 3) {
                                            return "img/read_until_green.png"
                                        }
                                        if (msgstate == 2) {
                                            return "img/2check.png"
                                        }
                                        if (msgstate == 1) {
                                            return "img/check.png"
                                        }
                                        return ""
                                    }

                                }
                                */
                            }
                        }
                }

            }
        }

    }

}


