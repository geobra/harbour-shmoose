import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    SilicaListView {
        id: searchPathsList

        PullDownMenu {
            MenuItem {
                text: qsTr("Add Path")
                onClicked: pageStack.push(Qt.resolvedUrl("PathSelectorPage.qml"), { "currentPath" : StandardPaths.pictures })
            }
        }

        VerticalScrollDecorator {
        }

        anchors.fill: parent

        header: PageHeader {
            title: qsTr("Attachment Search Paths")
        }

        model: shmoose.settings.ImagePaths

        delegate: ListItem {
            width: parent.width

            Label { text: modelData }

            menu: ContextMenu {
                MenuItem {
                    text: "Remove"
                    onClicked: {
                        // TODO
                    }
                }
            }
        }
    }
}
