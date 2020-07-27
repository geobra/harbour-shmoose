import QtQuick.Layouts 1.2
import QtQuick 2.3
import QtQuick.Controls 2.4

Rectangle {
    color: 'teal'

    ListView {
        id: rosterListView
        width: parent.width
        height: parent.height + 20

        model: shmoose.rosterController.rosterList
        delegate: Rectangle {
            height: Math.max(img.height, nameText.height + jidText.height) + 10
            width: parent.width + 20
            color: "yellow"
            radius: 10

            Image {
                id: img;
                //width: height;
                width: Math.min (50, sourceSize.width);
                height: Math.min (50, sourceSize.height);
                source: imagePath != "" ? imagePath : getImage(jid)
                /*
                anchors {
                    top: parent.top;
                    left: parent.left;
                    bottom: parent.bottom;
                }

                Rectangle {
                    z: -1;
                    color: (model.index % 2 ? "black" : "white");
                    opacity: 0.15;
                    anchors.fill: parent;
                }
                */
            }
            Column {
                anchors.left: img.right
                Text {
                    id: nameText
                    text: name
                }
                Text {
                    id: jidText
                    text: jid
                }

            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    rosterListView.currentIndex = index
                    //console.log( "jid: " + rosterListView.currentIndex)
                    //console.log( "jid: " + shmoose.rosterController.rosterList[rosterListView.currentIndex].jid)

                    shmoose.setCurrentChatPartner(shmoose.rosterController.rosterList[rosterListView.currentIndex].jid)
                }
            }
        }
    }

    function getImage(jid) {
        if (shmoose.rosterController.isGroup(jid)) {
            return "image://theme/icon-l-image";
        } else {
            return "image://theme/icon-l-people"
        }
    }
}

