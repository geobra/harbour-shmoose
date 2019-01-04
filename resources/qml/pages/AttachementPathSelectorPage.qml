import QtQuick 2.0
import Sailfish.Silica 1.0

Dialog {
    id: page;

    property string currentPath

//    Dialog {
//        anchors.fill: parent
//        acceptDestinationAction: PageStackAction.Pop

    Column {
        anchors.fill: parent

        DialogHeader {
            id: header
        }

        SectionHeader {
                id: currentPathLabel
                text: currentPath
                //text: qsTr("blah")
        }

        SilicaListView {
            id: folderContents
            anchors.top: currentPathLabel.bottom
            anchors.bottom: parent.bottom

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

            delegate: ListItem {
                width: folderContents.width
                height: Theme.itemSizeSmall

                Label { text: fruit }
            }
        }
    }

}
