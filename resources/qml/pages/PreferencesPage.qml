import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page;

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        VerticalScrollDecorator {}

        Column {
            id: column
            width: parent.width

            PageHeader { title: "Settings" }

            SectionHeader { text: "Attachements" }

            Button {
                text: "Attachement search paths"
                onClicked: pageStack.push(Qt.resolvedUrl("AttachementPathSelectorPage.qml"))
             }

        }

    }

}
