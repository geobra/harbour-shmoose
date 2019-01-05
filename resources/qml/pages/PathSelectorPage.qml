import QtQuick 2.0
import Sailfish.Silica 1.0

Dialog {
    id: page;

    property string currentPath
    DialogHeader {
        id: header
        width: parent.width
    }

    SectionHeader {
            id: currentPathLabel
            text: currentPath

            anchors.top: header.bottom
    }

    SilicaListView {
        id: folderContents
        anchors.top: currentPathLabel.bottom
        anchors.bottom: parent.bottom
        width: parent.width

        clip: true

        model: ListModel {
            ListElement { fruit: "jackfruit" }
            ListElement { fruit: "orange" }
            ListElement { fruit: "lemon" }
            ListElement { fruit: "lychee" }
            ListElement { fruit: "apricots" }
            ListElement { fruit: "apple" }
            ListElement { fruit: "clementine" }
            ListElement { fruit: "lime" }
            ListElement { fruit: "melon" }
            ListElement { fruit: "cherry" }
            ListElement { fruit: "pear" }
            ListElement { fruit: "banana" }
            ListElement { fruit: "blueberry" }
            ListElement { fruit: "raspberry" }
            ListElement { fruit: "blackberry" }
        }

        VerticalScrollDecorator {}

        delegate: ListItem {
            width: folderContents.width

            Label { text: fruit }
        }
    }

}
