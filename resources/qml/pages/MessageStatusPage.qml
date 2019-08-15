import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.shmoose 1.0

Page {
    id: page;

    SilicaListView {
        id: view;
        header: Column {
            spacing: Theme.paddingMedium;
            anchors {
                left: parent.left;
                right: parent.right;
            }

            PageHeader {
                title: qsTr("Message Status");
            }
        }
        model: shmoose.persistence.gcmController
        delegate: ListItem {
            id: item;
            contentHeight: Theme.itemSizeMedium;

            Column {
                anchors {
                    left: parent.left;
                    right: parent.right;
                    margins: Theme.paddingMedium;
                }

                Label {
                    text: chatmembername;
                    color: Theme.secondaryColor;
                    font.pixelSize: Theme.fontSizeMedium;
                }
                Row {
                    anchors {
                        left: parent.left;
                        right: parent.right;
                        margins: Theme.paddingMedium;
                    }

                    Label {
                        text: Qt.formatDateTime (new Date (timestamp * 1000), "yyyy-MM-dd hh:mm:ss");
                        color: Theme.secondaryColor;
                        font.pixelSize: Theme.fontSizeTiny;
                    }
                    Image {
                        source: {
                            if (msgstate == 2) {
                                return "../img/read_until_green.png"
                            }
                            if (msgstate == 1) {
                                return "../img/2check.png"
                            }
                            return ""
                        }
//		                    anchors.right: parent.right
                    }
                }
            }
        }
        anchors.fill: parent;
    }
}


