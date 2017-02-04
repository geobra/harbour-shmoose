import QtQuick 2.0
import Sailfish.Silica 1.0

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
                    id: details;
                    text: (modelData ["status"] || "Idle");
                    color: Theme.secondaryColor;
                    font.pixelSize: Theme.fontSizeSmall;
                }
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
}


