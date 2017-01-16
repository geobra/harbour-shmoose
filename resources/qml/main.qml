import QtQuick 2.0
import Sailfish.Silica 1.0
import MeeGo.Connman 0.2
import org.nemomobile.notifications 1.0

import "pages"
import "cover"

ApplicationWindow {
    id: mainWindow

    onApplicationActiveChanged: {
        if (applicationActive == true) {
            shmoose.setAppIsActive(true)
        }
        else {
            shmoose.setAppIsActive(false)
        }
    }

    cover: pageCover;
    initialPage: pageLogin;

    property var networkType: 0
    property bool hasInetConnection: false

    Component { id: pageLogin; LoginPage { } }
    Component { id: pageMenu; MenuPage { } }
    Component { id: pageCover; CoverPage { } }
    Component { id: pageContacts; ContactsPage { } }
    Component { id: pageConversations; ConversationsPage { } }
    Component { id: pagePreferences; PreferencesPage { } }
    Component { id: pageAccount; AccountPage { } }
    Component { id: pageMessaging; MessagingPage { } }
    Component { id: imagePicker; ImagePickerPage { } }

    ImagePickerPage {
        id: pageImagePicker
    }

    Component {
        id: messageNotification
        Notification {}
    }

    function newMessageNotification(id, jid, body) {
        var m = messageNotification.createObject(null)
        m.category = "harbour-shmoose-message"
        m.previewSummary = jid
        m.previewBody = body
        m.summary = jid
        m.body = body
        m.clicked.connect(function() {
            mainWindow.activate()
            mainWindow.showSession(jid)
        })
        // This is needed to call default action
        m.remoteActions = [ {
                               "name": "default",
                               "displayName": "Show Conversation",
                               "icon": "harbour-shmoose",
                               "service": "org.shmoose.session",
                               "path": "/message",
                               "iface": "org.shmoose.session",
                               "method": "showConversation",
                               "arguments": [ "jid", jid ]
                           } ]
        m.publish()
    }

    Connections {
        target: shmoose.persistence.messageController
        onSignalMessageReceived: {
            if (applicationActive == false) {
                newMessageNotification(id, jid, message);
            }
        }
    }

    function getHasInetConnection() {
        if(wifi.available && wifi.connected) {
            networkType = 1
            return true
        }
        if(cellular.available && cellular.connected) {
            networkType = 2
            return true
        }
        if(ethernet.available && ethernet.connected) {
            networkType = 3
            return true
        }

        return false
    }

    function showSession(jid) {
        //removeNotification(jid)
        shmoose.setCurrentChatPartner(jid)

        pageStack.clear()
        pageStack.push(pageMenu)
        pageStack.push(pageConversations)
        pageStack.push(pageMessaging, { "conversationId" : jid })
    }

    TechnologyModel {
        id: wifi
        name: "wifi"
        onConnectedChanged: {
            console.log("wifi changed!")
            if (wifi.connected)
            {
                console.log("wifi connected " + mainWindow.networkType)
            }
            else
            {
                console.log("wifi DISconnected " + mainWindow.networkType)
            }
            mainWindow.hasInetConnection = mainWindow.getHasInetConnection()
            shmoose.setHasInetConnection(mainWindow.hasInetConnection)
        }
    }

    TechnologyModel {
        id: cellular
        name: "cellular"
        onConnectedChanged: {
            console.log("cellular changed!")
            if (cellular.connected)
            {
                console.log("cellular connected")
            }
            else
            {
                console.log("cellular DISconnected")
            }
            mainWindow.hasInetConnection = mainWindow.getHasInetConnection()
            shmoose.setHasInetConnection(mainWindow.hasInetConnection)
        }
    }

    TechnologyModel {
        id: ethernet
        name: "ethernet"
        onConnectedChanged: {
            console.log("ethernet changed!")
            if (ethernet.connected)
            {
                console.log("ethernet connected")
            }
            else
            {
                console.log("ethernet DISconnected")
            }
            mainWindow.hasInetConnection = mainWindow.getHasInetConnection()
            shmoose.setHasInetConnection(mainWindow.hasInetConnection)
        }
    }
}


