import QtQuick.Layouts 1.2
import QtQuick 2.3
import QtQuick.Controls 2.4


Rectangle {
    RowLayout {
        id: layout
        anchors.fill: parent
        spacing: 6
        Rectangle {
            id: textBoxRect
            Layout.fillWidth: true
            Layout.minimumHeight: 150
            color: 'teal'

            ScrollView {
                id: scroll
                clip: true
                Layout.fillWidth: true
                anchors.fill: parent
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                function scrollToBottom() {
                    contentItem.contentY = edit.height - contentItem.height
                }

                TextEdit {
                    id: edit
                    width: textBoxRect.width

                    readOnly: false
                    wrapMode: TextEdit.Wrap
                    text: "a message"

                    //onCursorRectangleChanged: scroll.scrollToBottom()
                }
            }
        }
        Button {
            text: "send"

            onClicked: {
                console.log("send to: " + shmoose.getCurrentChatPartner())
                shmoose.sendMessage(edit.text, "txt")
                edit.text = ""
            }
        }
    }
}

