import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.shmoose 1.0

Page {
    id: page;

    SilicaListView {
        id: jidlist
        header: Column {
            spacing: Theme.paddingMedium;
            anchors {
                left: parent.left;
                right: parent.right;
            }

            PageHeader {
                title: qsTr ("Contacts");
            }
            SearchField {
                placeholderText: qsTr ("Filter");
                anchors {
                    left: parent.left;
                    right: parent.right;
                }
            }
        }
        model: shmoose.rosterController.rosterList
        delegate: ListItem {
            id: item;
            menu: contextMenu
            contentHeight: Theme.itemSizeMedium;
            onClicked: {
                shmoose.setCurrentChatPartner(jid)
                pageStack.push (pageMessaging, { "conversationId" : jid });
            }

            Image {
                id: img;
                width: height;
                source: (modelData ["image"] || "qrc:///qml/img/avatar.png");
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
            }
            Column {
                anchors {
                    left: img.right;
                    margins: Theme.paddingMedium;
                    verticalCenter: parent.verticalCenter;
                }

                Label {
                    id: lbl;
                    text: jid
                    color: (item.highlighted ? Theme.highlightColor : Theme.primaryColor);
                    font.pixelSize: Theme.fontSizeMedium;
                }
                Label {
                    id: nameId;
                    text: name;
                    color: Theme.secondaryColor;
                    font.pixelSize: Theme.fontSizeSmall;
                }
                Image {
                    id: subscriptionImage;
                    source: getSubscriptionImage(subscription);
                }
                Component {
                    id: contextMenu
                    ContextMenu {
                        MenuItem {
                            text: "Remove"
                            onClicked: {
                                console.log("remove..." + jid)
                                shmoose.rosterController.removeContact(jid)
                            }
                        }
                    }
                }
            }
        }

        anchors.fill: parent;

        PullDownMenu {
            enabled: true
            visible: true

            MenuItem {
                text: qsTr ("Add contact");
                onClicked: {
                    console.log("add contact")
                    pageStack.push(dialogCreateContact)
                }
            }

            MenuItem {
                text: qsTr ("Create Room TBD");
                onClicked: {
                    console.log("create room")
                }
            }

            MenuItem {
                text: qsTr ("Join room by id TBD");
                onClicked: {
                    console.log("join room by id")
                }
            }
        }
    }

    function getSubscriptionImage(subs) {
        if (subs === RosterItem.SUBSCRIPTION_NONE) {
            return "image://theme/icon-cover-cancel"
        } else if (subs === RosterItem.SUBSCRIPTION_TO) {
            return "image://theme/icon-cover-next"
        } else if (subs === RosterItem.SUBSCRIPTION_FROM) {
            return "image://theme/icon-cover-previous"
        } else {
            return "image://theme/icon-cover-transfers"
        }
    }

}


