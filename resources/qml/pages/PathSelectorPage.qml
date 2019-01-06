import QtQuick 2.0
import Sailfish.Silica 1.0
import Qt.labs.folderlistmodel 2.1

Dialog {
    id: page;

    property url currentPath

    onAccepted: {
        console.log("Adding " + currentPath + " to image search paths")
        shmoose.settings.addImagePath(currentPath.toString())
    }

    acceptDestinationAction: PageStackAction.Pop

    DialogHeader {
        id: header
        width: parent.width
    }

    Row {
        id: controlsRow

        anchors.top: header.bottom

        Button {
            id: oneUpButton
            text: "⇧"
            width: Theme.itemSizeExtraSmall

            enabled: currentPath != "file:///"
            onClicked: currentPath = fsModel.parentFolder
        }

        Label {
                id: currentPathLabel
                text: currentPath
                anchors.verticalCenter: oneUpButton.verticalCenter

                horizontalAlignment: Text.AlignRight
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.highlightColor
                height: Theme.itemSizeExtraSmall
        }
    }

    SilicaListView {
        id: folderContents
        anchors.top: controlsRow.bottom
        anchors.bottom: parent.bottom
        width: parent.width

        clip: true

        FolderListModel {
            id: fsModel

            folder: currentPath
            showDirs: true
            showDotAndDotDot: false
            showFiles: false
        }

        model: fsModel


        VerticalScrollDecorator {}

        delegate: ListItem {
            width: folderContents.width

            Label { text: fileName }

            onClicked: currentPath += "/" + fileName
        }
    }

}
