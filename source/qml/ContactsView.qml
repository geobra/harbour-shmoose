import QtQuick.Layouts 1.2
import QtQuick 2.3
import QtQuick.Controls 2.4
import harbour.shmoose 1.0

Rectangle {

    ListView {
        id: rosterListView
        width: parent.width
        height: parent.height + 20

        model: shmoose.rosterController.rosterList
        delegate: Rectangle {
            height: nameText.height + jidText.height + avail.height + 10
            width: parent.width

            Rectangle {
                id: contactsItem
                height: nameText.height + jidText.height + avail.height + 5
                width: parent.width
                color: "linen"

                radius: 10


                Image {
                    id: img;
                    width: Math.min (50, sourceSize.width);
                    height: Math.min (50, sourceSize.height);
                    source: imagePath != "" ? "file:/" + imagePath : getImage(jid)
                }

                Column {
                    id: nameAndJid
                    anchors.left: img.right
                    Label {
                        id: nameText

                        wrapMode: Text.NoWrap
                        font.pixelSize: 14
                        width: contactsItem.width
                        maximumLineCount: 1

                        text: name
                    }
                    Label {
                        id: jidText

                        wrapMode: Text.Wrap
                        font.pixelSize: 8
                        width: contactsItem.width

                        text: jid
                    }
                    Row {
                        Image {
                            id: subs;
                            visible: ! shmoose.rosterController.isGroup(jid)
                            source: getSubscriptionImage(subscription);
                        }
                        Image {
                            id: avail;
                            source: getAvailabilityImage(availability)
                        }
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
            return "img/group.png";
        } else {
            return "img/person.png"
        }
    }

    function getSubscriptionImage(subs) {
        if (subs === RosterItem.SUBSCRIPTION_NONE) {
            return "img/hourglass.png"
        } else if (subs === RosterItem.SUBSCRIPTION_TO) {
            return "img/arrow_left.png"
        } else if (subs === RosterItem.SUBSCRIPTION_FROM) {
            return "img/arrow_right.png"
        } else {
            return "img/arrow_both.png"
        }
    }

    function getAvailabilityImage(avail) {
        if (avail === RosterItem.AVAILABILITY_ONLINE) {
            return "img/online.png"
        } else if (avail === RosterItem.AVAILABILITY_OFFLINE) {
            return "img/offline.png"
        } else {
            return "img/device_unknown.png"
        }
    }
}

