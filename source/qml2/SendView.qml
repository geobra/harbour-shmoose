import QtQuick.Layouts 1.2
import QtQuick 2.3
import QtQuick.Controls 2.4


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
                shmoose.sendMessage(edit.text, "txt")
                edit.text = ""
            }
        }
    }
}

