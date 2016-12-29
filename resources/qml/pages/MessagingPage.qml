import QtQuick 2.0
import QtQuick.Window 2.0;
import Sailfish.Silica 1.0

Page {
    id: page;

    property string conversationId : "";

    Image {
        //source: "image://glass/qrc:///qml/img/photo.png";
        opacity: 0.85;
        sourceSize: Qt.size (Screen.width, Screen.height);
        asynchronous: false
        anchors.centerIn: parent;
    }
    Item {
        id: banner;
        height: Theme.itemSizeLarge;
        anchors {
            top: parent.top;
            left: parent.left;
            right: parent.right;
        }

        Rectangle {
            z: -1;
            color: "black";
            opacity: 0.15;
            anchors.fill: parent;
        }
        Image {
            id: avatar;
            width: Theme.iconSizeMedium;
            height: width;
            smooth: true;
            source: "qrc:///qml/img/avatar.png";
            fillMode: Image.PreserveAspectCrop;
            antialiasing: true;
            anchors {
                right: parent.right;
                margins: Theme.paddingMedium;
                verticalCenter: parent.verticalCenter;
            }

            Rectangle {
                z: -1;
                color: "black";
                opacity: 0.35;
                anchors.fill: parent;
            }
        }
        Column {
            anchors {
                right: avatar.left;
                margins: Theme.paddingMedium;
                verticalCenter: parent.verticalCenter;
            }

            Label {
                text:  conversationId;
                color: Theme.highlightColor;
                font {
                    family: Theme.fontFamilyHeading;
                    pixelSize: Theme.fontSizeLarge;
                }
                anchors.right: parent.right;
            }
            Label {
                text: qsTr ("last seen yesterday, 12:30 PM");
                color: Theme.secondaryColor;
                font {
                    family: Theme.fontFamilyHeading;
                    pixelSize: Theme.fontSizeTiny;
                }
                anchors.right: parent.right;
            }
        }
    }
    SilicaListView {
        id: view;
        clip: true;

        model: shmoose.persistence.messageController

        header: Item {
            height: view.spacing;
            anchors {
                left: parent.left;
                right: parent.right;
            }
        }
        footer: Item {
            height: view.spacing;
            anchors {
                left: parent.left;
                right: parent.right;
            }
        }
        spacing: Theme.paddingMedium;
        delegate: Item {
            id: item;
            height: shadow.height;
            anchors {
                left: parent.left;
                right: parent.right;
                margins: view.spacing;
            }

            readonly property bool alignRight      : (direction == 1);
            readonly property int  maxContentWidth : (width * 0.85);

            Rectangle {
                id: shadow;
                color: "white";
                radius: 3;
                opacity: (item.alignRight ? 0.05 : 0.15);
                antialiasing: true;
                anchors {
                    fill: layout;
                    margins: -Theme.paddingSmall;
                }
            }
            Column {
                id: layout;
                anchors {
                    left: (item.alignRight ? parent.left : undefined);
                    right: (!item.alignRight ? parent.right : undefined);
                    margins: -shadow.anchors.margins;
                    verticalCenter: parent.verticalCenter;
                }

                Text {
                    text: message;
                    color: Theme.primaryColor;
                    width: Math.min (item.maxContentWidth, contentWidth);
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere;
                    visible: true;
                    font {
                        family: Theme.fontFamilyHeading;
                        pixelSize: Theme.fontSizeMedium;
                    }
                    anchors {
                        left: (item.alignRight ? parent.left : undefined);
                        right: (!item.alignRight ? parent.right : undefined);
                    }
                }
                Image {
                    source: (visible ? modelData ["content"] || "" : "");
                    width: Math.min (item.maxContentWidth, sourceSize.width);
                    fillMode: Image.PreserveAspectFit;
                    visible: (modelData ["type"] === "img");
                    anchors {
                        left: (item.alignRight ? parent.left : undefined);
                        right: (!item.alignRight ? parent.right : undefined);
                    }
                }
                Label {
                    text: Qt.formatDateTime (new Date (timestamp * 1000), "yyyy-MM-dd hh:mm:ss");
                    color: Theme.secondaryColor;
                    font {
                        family: Theme.fontFamilyHeading;
                        pixelSize: Theme.fontSizeTiny;
                    }
                    anchors {
                        left: (item.alignRight ? parent.left : undefined);
                        right: (!item.alignRight ? parent.right : undefined);
                    }
                }
                Image {
                    source: {
                        if (isreceived) {
                            return "../img/check.png"
                        }
                        return ""
                    }
                    anchors.right: parent.right
                }
            }
        }
        anchors {
            top: banner.bottom;
            left: parent.left;
            right: parent.right;
            bottom: sendmsgview.top;
        }
    }

    Row {
        id: sendmsgview

        property var attachmentPath: ""

        anchors {
            left: parent.left;
            right: parent.right;
            bottom: parent.bottom;
        }

        TextArea {
            id: editbox;
            placeholderText: qsTr ("Enter message...");
            width: parent.width - 100

            onTextChanged: {
                sendButton.icon.source = getSendButtonImage()
            }
        }
        IconButton {
            id: sendButton
            icon.source: getSendButtonImage()
            width: 100
            onClicked: {
                if (editbox.text.length === 0 && sendmsgview.attachmentPath.length === 0) {
                    sendmsgview.attachmentPath = ""
                    fileModel.searchPath = "/home/nemo/Pictures"
                    pageStack.push(pageImagePicker)
                    pageImagePicker.selected.connect(setAttachmentPath)
                } else {
                    //console.log(sendmsgview.attachmentPath)
                    var msgToSend = editbox.text;
                    editbox.text = " ";
                    if (sendmsgview.attachmentPath.length > 0) {
                        shmoose.sendFile(conversationId, sendmsgview.attachmentPath);
                    }
                    shmoose.sendMessage(conversationId, msgToSend);
                }
                sendmsgview.attachmentPath = ""
            }
            function setAttachmentPath(path) {
                //console.log(path)
                sendmsgview.attachmentPath = path
            }
        }
    }
    function getSendButtonImage() {
        if (editbox.text.length === 0 && sendmsgview.attachmentPath.length === 0) {
            return "image://theme/icon-m-attach"
        } else {
            if (sendmsgview.attachmentPath.length > 0) {
                return "image://theme/icon-m-media"
            } else {
                return "image://theme/icon-m-enter-accept"
            }
        }
    }
}
