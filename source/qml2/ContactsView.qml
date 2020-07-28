import QtQuick.Layouts 1.2
import QtQuick 2.3
import QtQuick.Controls 2.4

Rectangle {

    ListView {
        id: rosterListView
        width: parent.width
        height: parent.height + 20

        model: shmoose.rosterController.rosterList
        delegate: Rectangle {
            height: Math.max(img.height, nameText.height + jidText.height) + 10
            width: parent.width

            Rectangle {
                id: contactsItem
                height: Math.max(img.height, nameText.height + jidText.height) + 5
                width: parent.width
                color: "linen"

                radius: 10


                Image {
                    id: img;
                    width: Math.min (50, sourceSize.width);
                    height: Math.min (50, sourceSize.height);
                    source: imagePath != "" ? imagePath : getImage(jid)
                }

                Column {
                    anchors.left: img.right
                    Label {
                        id: nameText

                        wrapMode: Text.Wrap
                        font.pixelSize: 14
                        width: contactsItem.width

                        text: name
                    }
                    Label {
                        id: jidText

                        wrapMode: Text.Wrap
                        font.pixelSize: 8
                        width: contactsItem.width

                        text: jid
                    }

                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        rosterListView.currentIndex = index

                        shmoose.setCurrentChatPartner(shmoose.rosterController.rosterList[rosterListView.currentIndex].jid)
                    }
                }
            }
        }

        highlight: Rectangle {
            z: 2
            color: "grey"
            radius: 10
            opacity: 0.3
            focus: true
        }
    }

    function getImage(jid) {
        if (shmoose.rosterController.isGroup(jid)) {
            return "img/baseline_group_black_24dp.png";
        } else {
            return "img/baseline_person_black_24dp.png"
        }
    }
}

