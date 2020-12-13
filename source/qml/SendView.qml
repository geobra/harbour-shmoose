import QtQuick.Layouts 1.2
import QtQuick 2.3
import QtQuick.Controls 2.4
import QtQuick.Dialogs 1.0

Rectangle {
    RowLayout {
        id: layout

        anchors.fill: parent
        width: parent.width
        height: parent.height

        spacing: 6

        Rectangle {
            id: textBoxRect

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 50

            width: parent.width
            height: parent.height

            border.color: 'deepskyblue'
            border.width: 2

            radius: 5

            ScrollView {
                id: scroll
                clip: true
                Layout.fillWidth: true
                anchors.fill: parent

                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                function scrollToBottom(r) {
                    if (contentItem.contentX >= r.x)
                        contentItem.contentX = r.x;
                    else if (contentItem.contentX+width <= r.x+r.width)
                        contentItem.contentX = r.x+r.width-width;
                    if (contentItem.contentY >= r.y)
                        contentItem.contentY = r.y;
                    else if (contentItem.contentY+height <= r.y+r.height)
                        contentItem.contentY = r.y+r.height-height;
                }

                TextEdit {
                    id: edit
                    width: textBoxRect.width - 10
                    height: textBoxRect.height

                    readOnly: false
                    wrapMode: TextEdit.Wrap
                    x: 5

                    property string placeholderText: "Enter message"

                    Text {
                        text: edit.placeholderText
                        color: "#aaa"
                        visible: !edit.text
                    }

                    onCursorRectangleChanged: scroll.scrollToBottom(cursorRectangle)
                }
            }
        }
        Button {
            Layout.alignment: Qt.AlignVCenter

            text: "send"

            onClicked: {
                console.log("send to: " + shmoose.getCurrentChatPartner())
                if (edit.text.startsWith("file://")) {
                    shmoose.sendFile(edit.text)
                }
                else {
                    shmoose.sendMessage(edit.text, "txt")
                }
                edit.text = ""
            }
        }
        Button {
            Layout.alignment: Qt.AlignVCenter

            id: attachButton
            text: "attach"
            enabled: false

            onClicked: {
                console.log("attach")
                fileDialog.open();
            }
        }

        Connections {
            target: shmoose
            onSignalCanSendFile: {
                console.log("HTTP uploads enabled");
                attachButton.enabled = true;
            }
        }

        FileDialog {
            id: fileDialog
            title: "Please choose a picture"
            folder: shortcuts.home
            nameFilters: [ "Image files (*.jpg *.jpeg *.png *.gif)"]
            onAccepted: {
                console.log("You chose: " + fileDialog.fileUrl);
                edit.text = fileDialog.fileUrl.toString();
            }
            onRejected: {
                console.log("Canceled")
            }
            Component.onCompleted: visible = false
        }

    }
}

