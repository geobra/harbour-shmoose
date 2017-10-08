import QtQuick 2.3
import QtQuick.Window 2.2
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1

import harbour.shmoose 1.0


GridLayout {
    anchors.fill: parent
    columns: 2

    readonly property int margin: 10;

    ListView {
        id: roster

        Layout.fillHeight: true
        Layout.fillWidth: true

        //model: rosterList
        model: shmoose.rosterController.rosterList
        //model: shmoose.persistence.sessionController

        delegate: Rectangle {
            height: 25
            width: parent.width
            color: "green"
            Text { text: jid}

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    roster.currentIndex = index

                    //console.log( "clicked: " + roster.currentIndex)
                    console.log( "jid: " + shmoose.rosterController.rosterList[roster.currentIndex].jid)

                    shmoose.setCurrentChatPartner(shmoose.rosterController.rosterList[roster.currentIndex].jid)
                }
            }
        }
    }

    Rectangle {
        id: chat

        anchors.left: roster.right

        Layout.fillHeight: true
        Layout.fillWidth: true

        property string jid: "foo"

        Text {
            anchors.centerIn: parent
            text: jid
        }
        ListView {
            id: messages

            height: parent.height
            width: parent.width

            model: shmoose.persistence.messageController
            //model: messageController

            delegate: Item {
                    id: item

                    width: parent.width
                    height: 20

                    readonly property bool alignRight: (direction == 1);

                    Column {

                        width: messages.width

                        Rectangle {

                            width: parent.width - 50
                            height: messageText.height
                            radius: margin

                            anchors {
                                left: (item.alignRight ? parent.left : undefined);
                                right: (!item.alignRight ? parent.right : undefined);
                                margins: margin
                            }

                            color: (item.alignRight ? "yellow" : "lightgreen")

                            Text {
                                id: messageText
                                horizontalAlignment: (item.alignRight ? Text.AlignRight : Text.AlignLeft)
                                text: message

                            }

                        }

                    }
                }
        }

        Row {
            anchors.bottom: chat.bottom
            height: 30
            width: parent.width

            TextField {
                id: texttosend

                width: parent.width - sendbutton.width
                height: parent.height

                placeholderText: qsTr("Enter message")
            }
            Button {
                id: sendbutton

                width: 50
                height: parent.height

                text: "send"
                onClicked: {
                    console.log("send to: " + shmoose.rosterController.rosterList[roster.currentIndex].jid)
                    shmoose.sendMessage(shmoose.rosterController.rosterList[roster.currentIndex].jid, texttosend.text, "txt")
                }
            }
        }
    }
}
