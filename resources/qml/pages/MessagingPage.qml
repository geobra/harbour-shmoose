import QtQuick 2.0
import QtQuick.Window 2.0;
import QtMultimedia 5.6
import Sailfish.Silica 1.0
import harbour.shmoose 1.0
import Sailfish.Pickers 1.0
import Nemo.Thumbnailer 1.0

Page {
    id: page;
    allowedOrientations: Orientation.All;

    onStatusChanged: {
        if (status === PageStatus.Active) {
            pageStack.pushAttached(Qt.resolvedUrl("MessagingSettingsPage.qml"),{ 'conversationId': conversationId })
        }
        if (status == PageStatus.Deactivating) {
            if (_navigation == PageNavigation.Back) {
                shmoose.setCurrentChatPartner("")
            }
        }
    }

    property string conversationId : "";
    property bool isGroup : shmoose.rosterController.isGroup(conversationId);
    property string attachmentPath: shmoose.getAttachmentPath();
    property string imagePath : shmoose.rosterController.getAvatarImagePathForJid(conversationId);

    Image {
        //source: "image://glass/qrc:///qml/img/photo.png";
        opacity: 0.85;
        sourceSize: Qt.size (Screen.width, Screen.height);
        asynchronous: false
        anchors.centerIn: parent;
    }

    PageHeader {
        id: banner;
        title: trimStr(shmoose.rosterController.getNameForJid(conversationId));
        Image {
            id: avatar;
            parent: banner.extraContent;
            width: Theme.iconSizeMedium;
            height: width;
            smooth: true;
            source: imagePath != "" ? imagePath : getImage(conversationId);
            fillMode: Image.PreserveAspectCrop;
            antialiasing: true;
            anchors {
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
    }
    SilicaListView {
        id: view;

        verticalLayoutDirection: ListView.BottomToTop;
        clip: true;
        spacing: Theme.paddingMedium;

        model: shmoose.persistence.messageController

        delegate: ListItem {
            id: item;

            contentHeight: shadow.height;

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


                Label {
                    text: message;
                    color: Theme.primaryColor;
                    //width: Math.min (item.maxContentWidth, contentWidth);
                    width: item.maxContentWidth;
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere;
                    visible: type === "txt"

                    font {
                        family: Theme.fontFamilyHeading;
                        pixelSize: Theme.fontSizeMedium;
                    }
                    anchors {
                        left: (item.alignRight ? parent.left : undefined);
                        right: (!item.alignRight ? parent.right : undefined);
                    }

                    onLinkActivated: Qt.openUrlExternally(link)
                }
                BackgroundItem {
                    width: Math.max(thumb.width, icon.width)
                    height: Math.max(thumb.height, icon.height)

                    visible: type !== "txt"
                    anchors {
                        left: (item.alignRight ? parent.left : undefined);
                        right: (!item.alignRight ? parent.right : undefined);
                    }
                    Thumbnail {
                        id: thumb

                        property string file: getFilePath(message)
                        anchors.right: (!item.alignRight ? parent.right : undefined)
                        source: file
                        mimeType: type
                        visible: hasThumbnail(type)

                        sourceSize.width: item.maxContentWidth*.75
                        sourceSize.height: item.maxContentWidth*.75

                        fillMode: Thumbnail.PreserveAspectFit;
                        priority: Thumbnail.NormalPriority

                        Icon {
                            visible: startsWith(type, "video")
                            source: "image://theme/icon-m-file-video"
                            anchors.centerIn : parent
                        }
                    }
                    Icon {
                        id: icon
                        visible: (type !== "txt") && (!hasThumbnail(type))
                        source: getFileIcon(type)

                        anchors.right: (!item.alignRight ? parent.right : undefined)
                    }
                    onClicked: {
                        if (startsWith(type, "image"))
                            pageStack.push(Qt.resolvedUrl("ImagePage.qml"),{ 'imgUrl': thumb.file })
                        else if (startsWith(type, "video"))
                            pageStack.push(Qt.resolvedUrl("VideoPage.qml"),{ 'path': thumb.file })
                         else if (startsWith(type, "audio"))
                            pageStack.push(Qt.resolvedUrl("VideoPage.qml"),{ 'path': thumb.file });
                    }
                }
                Label {
                    visible: isGroup;
                    color: Theme.secondaryColor;
                    font {
                        family: Theme.fontFamilyHeading;
                        pixelSize: Theme.fontSizeTiny;
                    }
                    text: resource;
                }
                Row {
                    spacing: 5
                    anchors.right: (!item.alignRight ? parent.right : undefined)
                    Label {
                        text: Qt.formatDateTime (new Date (timestamp * 1000), "yyyy-MM-dd hh:mm:ss");
                        color: Theme.secondaryColor;
                        font {
                            family: Theme.fontFamilyHeading;
                            pixelSize: Theme.fontSizeTiny;
                        }
                    }

                    Image {
                        id: chatmarker
                        source: {
                            if (msgstate == 3) {
                                return "../img/read_green.png"
                            }
                            if (msgstate == 2) {
                                return "../img/2check.png"
                            }
                            if (msgstate == 1) {
                                return "../img/check.png"
                            }
                            return ""
                        }
                    }

                    Image {
                        source: {
                            if (security == 1) { // omemo
                                return "image://theme/icon-s-outline-secure"
                            }
                            return ""
                        }
                    }
                }

            }

            menu: ContextMenu {
                MenuItem {
                    text: qsTr("Copy")
                    onClicked: Clipboard.text = message
                }
                MenuItem {
                    visible: isGroup;
                    text: qsTr("Status")
                    onClicked:  {
                        shmoose.persistence.gcmController.setFilterOnMsg(id);
                        pageStack.push(pageMsgStatus);
                    }
                }
            }

        }
        anchors {
            top: banner.bottom;
            left: parent.left;
            right: parent.right;
            bottom: displaymsgview.top;
        }

    }
    Row {
        id: displaymsgview

        spacing: 5

        anchors {
            left: parent.left;
            right: parent.right;
            bottom:sendmsgview.top;
            margins: view.spacing;
        }
        Label {
            id: displaymsgviewlabel
            text:  shmoose.persistence.getResourcesOfNewestDisplayedMsgforJid(conversationId);
            enabled: isGroup;
            color: Theme.highlightColor;
            font {
                family: Theme.fontFamily;
                pixelSize: Theme.fontSizeTiny;
            }
        }
        Image {
            source: "../img/read_until_green.png";
            visible: displaymsgviewlabel.text.length > 0 ? true: false;
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

        height: Math.max(editbox.height, previewAttachment.height)

        TextArea {
            id: editbox;
            visible: sendmsgview.attachmentPath.length === 0

            property var userHasOmemo: shmoose.isOmemoUser(conversationId);
            property var useOmemo: (shmoose.settings.SendPlainText.indexOf(conversationId) < 0) ? true : false;

            property var enterMsg: qsTr("Enter message...");
            property var phT: (userHasOmemo && useOmemo) ? "Omemo: " + enterMsg : enterMsg;

            placeholderText: phT;

            font {
                family: Theme.fontFamily
                pixelSize: Theme.fontSizeMedium
            }

            onTextChanged: {
                sendButton.icon.source = getSendButtonImage()
            }

           anchors.bottom: parent.bottom 
           width: parent.width - sendButton.width - Theme.horizontalPageMargin
        }
        Thumbnail {
            id: previewAttachment
            visible: sendmsgview.attachmentPath.length > 0
            width: parent.width /4     
            height: parent.width /4
            sourceSize.width: parent.width / 4
            sourceSize.height: parent.width / 4
            anchors {                                                                                                                               
                bottom: parent.bottom; bottomMargin: Theme.paddingMedium                           
            }
            Icon {
                visible: startsWith(previewAttachment.mimeType, "video") && previewAttachment.status != Thumbnail.Error
                source: "image://theme/icon-m-file-video"
                anchors.centerIn: parent
            }
            Icon {
                visible: previewAttachment.status == Thumbnail.Error
                source: getFileIcon(previewAttachment.mimeType)
                anchors.centerIn: parent
            }
            IconButton {
                id: removeAttachment
                anchors.right: parent.right
                anchors.top: parent.top
                icon.source: "image://theme/icon-splus-cancel" 

                onClicked: {
                    sendmsgview.attachmentPath = "";
                    sendButton.icon.source = getSendButtonImage();
                }           
            }
        }
    }
        IconButton {
            id: sendButton
            enabled: {
                if (shmoose.connectionState && mainWindow.hasInetConnection) {
                    return true
                }
                else {
                    return false
                }
            }
            icon.asynchronous: true
            icon.source: getSendButtonImage()
            icon.width: Theme.iconSizeMedium + 2*Theme.paddingSmall                                
            icon.height: width

            anchors {                                                                              
                // icon-m-send has own padding                                                     
                right: parent.right; rightMargin: Theme.horizontalPageMargin-Theme.paddingMedium   
                bottom: parent.bottom; bottomMargin: Theme.paddingMedium                           
            } 
            onClicked: {
                if (editbox.text.length === 0 && sendmsgview.attachmentPath.length === 0 && shmoose.canSendFile()) {
                    sendmsgview.attachmentPath = ""
                    fileModel.searchPath = shmoose.settings.ImagePaths
                    pageStack.push(filePickerPage)
                } else {
                    //console.log(sendmsgview.attachmentPath)
                    var msgToSend = editbox.text;

                    if (sendmsgview.attachmentPath.length > 0) {
                        shmoose.sendFile(conversationId, sendmsgview.attachmentPath);
                        sendmsgview.attachmentPath = ""
                    }

                    if (msgToSend.length > 0) {
                        shmoose.sendMessage(conversationId, msgToSend, "txt", "");
                        editbox.text = " ";
                        editbox.text = "";
                    }

                    displaymsgviewlabel.text = "";
                }
            }
        } 
        
        Component {
            id: filePickerPage
            ContentPickerPage {
            onSelectedContentPropertiesChanged: {
                sendmsgview.attachmentPath = selectedContentProperties.filePath
                sendButton.icon.source = getSendButtonImage()
                previewAttachment.source = sendmsgview.attachmentPath
                previewAttachment.mimeType = selectedContentProperties.mimeType
                }
            }
        }

        Connections {
            target: shmoose
            onSignalCanSendFile: {
                console.log("HTTP uploads enabled");
                sendButton.icon.source = getSendButtonImage();
            }
        }

    function getSendButtonImage() {
        if (editbox.text.length === 0 && sendmsgview.attachmentPath.length === 0) {
            if (shmoose.canSendFile()) {
                return "image://theme/icon-m-attach"
            } else {
                return "image://theme/icon-m-send"
            }
        } else {
                return "image://theme/icon-m-send"
        }
    }

    function basename(str)
    {
        return (str.slice(str.lastIndexOf("/")+1))
    }

    function getImage(jid) {
        if (shmoose.rosterController.isGroup(jid)) {
            return "image://theme/icon-l-image";
        } else {
            return "image://theme/icon-l-people"
        }
    }
    
    function trimStr(str){
        var trimmedStr = str;
        if (str.length > 30)
        {
            trimmedStr = str.substring(1, 30);
        }

        trimmedStr = trimmedStr.replace(/(\r\n|\n|\r)/gm,"");

        return trimmedStr;
    }
    function hasThumbnail(type) {
        return startsWith(type, "image") || startsWith(type, "video");
    }
    function getFilePath(message) {
        return attachmentPath + "/" + shmoose.getLocalFileForUrl(message);
    }
    function getFileIcon(type){
        if(startsWith(type, "image")) return "image://theme/icon-m-file-image";
        if(startsWith(type, "video")) return "image://theme/icon-m-file-video";
        if(startsWith(type, "audio")) return "image://theme/icon-m-file-audio";
        return "image://theme/icon-m-file-document-light";
    }
    function startsWith(s,start) {
        return (s.substring(0, start.length) == start); 
    }
}
