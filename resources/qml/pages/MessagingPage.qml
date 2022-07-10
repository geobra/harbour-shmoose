import QtQuick 2.6
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
    property string imagePath : shmoose.rosterController.getAvatarImagePathForJid(conversationId);
    property bool refreshDate : false;

    Timer {
        interval: 60000; running: true; repeat: true
        onTriggered: {
            refreshDate = !refreshDate;
        }
    }

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
        focus: true
        spacing: Theme.paddingMedium;
        cacheBuffer: Screen.width // do avoid flickering when image width is changed

        model: shmoose.persistence.messageController

        delegate: ListItem {
            id: item;

            property string file : shmoose.getLocalFileForUrl(message)

            contentHeight: shadow.height;

            anchors {
                left: parent.left;
                right: parent.right;
                margins: view.spacing;
            }

            readonly property bool alignRight      : (direction == 1);
            readonly property int  maxContentWidth : (width * 0.85);
            readonly property int mediaWidth : maxContentWidth * 0.75
            readonly property int mediaHeight : (mediaWidth * 2) / 3
            readonly property bool isVideo : startsWith(type, "video")
            readonly property bool isImage : startsWith(type, "image")
            readonly property string iconFileSource : getFileIcon(type)

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
                    width: item.maxContentWidth;
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere;
                    visible: type === "txt"

                    font {
                        family: Theme.fontFamilyHeading;
                        pixelSize: Theme.fontSizeMedium;
                    }

                    onLinkActivated: Qt.openUrlExternally(link)
                }

                BackgroundItem {

                    id: bkgnd
                    width: thumb.width
                    height: isVideo || isImage ? thumb.height : iconFile.height 
                    visible: type != "txt"

                    Thumbnail {
                        id: thumb

                        source: isVideo || isImage ? item.file : ""
                        mimeType: type

                        sourceSize.width: item.mediaWidth
                        sourceSize.height: item.mediaHeight

                        width: implicitWidth > 0 ? implicitWidth : item.mediaWidth
                        height: item.mediaHeight

                        fillMode: Thumbnail.PreserveAspectFit;
                        priority: Thumbnail.NormalPriority

                        Icon {
                            visible: isVideo
                            source: "image://theme/icon-m-file-video"
                            anchors.centerIn : parent
                        }
                    }
                    Row {
                        id: placeholder
                        visible: thumb.status != Thumbnail.Ready
                        Icon {
                            id: iconFile
                            source: iconFileSource
                        }
                        Button {
                            visible: item.file == ""
                            anchors.verticalCenter: iconFile.verticalCenter
                            id: bnDownloadAttachment
                            text: qsTr("Download")
                            onClicked: {
                                shmoose.downloadFile(message, id);
                            }
                        }
                        Label {
                            visible: thumb.status == Thumbnail.Error
                            anchors.verticalCenter: iconFile.verticalCenter
                            text: qsTr("Impossible to load")
                            color: Theme.primaryColor;
                            width: parent.width;
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere;
                            font {
                                family: Theme.fontFamilyHeading;
                                pixelSize: Theme.fontSizeMedium;
                            }
                        }
                    }
                }
                Label {
                    color: Theme.secondaryColor;
                    visible: false; // type !== "txt"
                    width: item.maxContentWidth;
                    wrapMode: Text.WrapAnywhere;
                    font {
                        family: Theme.fontFamilyHeading;
                        pixelSize: Theme.fontSizeTiny;
                    }
                    text: qsTr("Message: ")+message;
                }
                Label {
                    color: Theme.secondaryColor;
                    visible: false; // type !== "txt"
                    width: item.maxContentWidth;
                    wrapMode: Text.WrapAnywhere;
                    font {
                        family: Theme.fontFamilyHeading;
                        pixelSize: Theme.fontSizeTiny;
                    }
                    text: qsTr("Local File: ")+item.file;
                }
                Label {
                    visible: false;
                    color: Theme.secondaryColor;
                    font {
                        family: Theme.fontFamilyHeading;
                        pixelSize: Theme.fontSizeTiny;
                    }
                    text: qsTr("Id: ")+id;
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
                        id: upload
                        visible: msgstate == 4
                        text: qsTr("uploading")
                        color: Theme.secondaryColor;
                        font {
                            family: Theme.fontFamilyHeading;
                            pixelSize: Theme.fontSizeTiny;
                        }

                        Connections {
                            target: shmoose
                            onSignalShowStatus: {
                                if(qsTr(headline) == qsTr("File Upload") && msgstate == 4)
                                    upload.text = qsTr("uploading ")+body;
                            }
                            onHttpDownloadFinished: {
                                if(attachmentMsgId == id)
                                {
                                    item.file =  shmoose.getLocalFileForUrl(message);
                                }
                            }
                        }
                    }
                    Label {
                        text: qsTr("send failed")
                        visible: msgstate == 5
                        color: Theme.secondaryColor;
                        font {
                            family: Theme.fontFamilyHeading;
                            pixelSize: Theme.fontSizeTiny;
                        }
                    }
                    Label {
                        text: refreshDate, getDateDiffFormated(new Date (timestamp * 1000));
                        visible: msgstate != 4
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
            MouseArea {
                anchors.fill: parent
                enabled: thumb.status != Thumbnail.Error && item.file != ""

                onClicked: {
                    if (startsWith(type, "image"))
                        pageStack.push(Qt.resolvedUrl("ImagePage.qml"),{ 'imgUrl': item.file })
                    else if (startsWith(type, "video"))
                        pageStack.push(Qt.resolvedUrl("VideoPage.qml"),{ 'path': item.file })
                    else if (startsWith(type, "audio"))
                        pageStack.push(Qt.resolvedUrl("VideoPage.qml"),{ 'path': item.file });
                }

                onPressAndHold: {
                    item.openMenu();
                }
            }
            menu: ContextMenu {
                MenuItem {
                    text: qsTr("Copy")
                    visible: type == "txt"
                    onClicked: Clipboard.text = message
                }
                MenuItem {
                    text: qsTr("Copy URL")
                    visible: type != "txt"
                    onClicked: Clipboard.text = message
                }
                MenuItem {
                    text: qsTr("Send again")
                    visible: (msgstate == 5 && shmoose.canSendFile())
                    onClicked: {
                        shmoose.sendFile(conversationId, message);
                     }
                }
                MenuItem {
                    text: qsTr("Save")
                    visible:  (type != "txt") && (direction == 1)
                    onClicked: {
                        shmoose.saveAttachment(item.file);
                     }
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
            leftMargin: Theme.paddingMedium;
            right: sendButton.left;
            bottom: parent.bottom;
        }

        height: (previewAttachment.visible ? 
                 Math.max(editbox.height, previewAttachment.height)+Theme.paddingMedium : 
                 editbox.height+Theme.paddingMedium
                )

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
           width: parent.width
        }
        Thumbnail {
            id: previewAttachment
            visible: sendmsgview.attachmentPath.length > 0
            width: Math.min(page.width, page.height) / 4    
            height: Math.min(page.width, page.height) / 4
            sourceSize.width: Math.min(page.width, page.height) / 4
            sourceSize.height: Math.min(page.width, page.height) / 4
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
                    pageStack.push(shmoose.settings.SendOnlyImages ? imagePickerPage: filePickerPage)
                } else {
                    //console.log(sendmsgview.attachmentPath)
                    var msgToSend = editbox.text;

                    if (sendmsgview.attachmentPath.length > 0) {
                        shmoose.sendFile(conversationId, sendmsgview.attachmentPath);
                        sendmsgview.attachmentPath = ""
                    }

                    if (msgToSend.length > 0) {
                        shmoose.sendMessage(conversationId, msgToSend, "txt");
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

        Component {
            id: imagePickerPage
            ImagePickerPage {
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
            onCanSendFile: {
                //console.log("HTTP uploads enabled");
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
    function getFileIcon(type){
        if(startsWith(type, "image")) return "image://theme/icon-l-image";
        if(startsWith(type, "video")) return "image://theme/icon-l-play";
        if(startsWith(type, "audio")) return "image://theme/icon-l-music";
        return "image://theme/icon-l-other";
    }
    function startsWith(s,start) {
        return (s.substring(0, start.length) == start); 
    }
    function getDateDiffFormated(d) {
        var n = new Date();
        var diff = (n.getTime() - d.getTime()) / 1000;
        var locale = Qt.locale();

        if(diff < 0)
            return "?"
        else if(diff < 60)
            return qsTr("now")
        else if(diff < 60*2)
            return qsTr("1 mn ago")
        else if(diff < 60*30)
            return qsTr ("") + Math.round(diff/60, 0)+ qsTr(" mns ago");

        var s = d.toLocaleTimeString(locale, "hh:mm");

        if(d.getFullYear() != n.getFullYear())
        {
            s = d.toLocaleDateString(locale, "d MMM yyyy") + " " + s;
        }
        else if (d.getMonth() != n.getMonth() || d.getDate() != n.getDate())
        {
            s = d.toLocaleDateString(locale, "d MMM") + " " +s;
        }

        return s;
    }
}
