import QtQuick 2.0
import Sailfish.Silica 1.0
import Qt.labs.folderlistmodel 2.1

Dialog {
    id: page;

    property url currentPath

    onAccepted: {
        shmoose.settings.addImagePath(currentPath)
    }

    acceptDestinationAction: PageStackAction.Pop

    DialogHeader {
        id: header
        width: parent.width
    }


    Button {
        id: oneUpButton
        text: "⇧"
        width: Theme.itemSizeExtraSmall
        x: Theme.horizontalPageMargin

        anchors.top: header.bottom

        enabled: currentPath != "file:///"
        onClicked: currentPath = fsModel.parentFolder
    }

    Label {
            id: currentPathLabel
            text: currentPath.toString().replace(/file:\/\//, "")

            anchors {
                top: header.bottom
                left: oneUpButton.right
                right: parent.right
                verticalCenter: oneUpButton.verticalCenter
            }

            padding: Theme.horizontalPageMargin

            horizontalAlignment: Text.AlignRight
            font.pixelSize: Theme.fontSizeSmall
            color: Theme.highlightColor
            height: Theme.itemSizeExtraSmall
            truncationMode: TruncationMode.Fade
    }

    SilicaListView {
        id: folderContents
        anchors.top: oneUpButton.bottom
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

            onClicked: currentPath = decodeURIComponent(currentPath.toString().replace(/(\/)?$/, "/" + fileName))
        }
    }

}
