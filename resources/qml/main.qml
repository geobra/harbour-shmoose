import QtQuick 2.0
import Sailfish.Silica 1.0
import org.nemomobile.notifications 1.0

import "pages"
import "cover"

ApplicationWindow {
    cover: pageCover;
    initialPage: pageMenu;
    Component.onCompleted: {
        pageStack.push (pageContacts, { }, PageStackAction.Immediate);
    }

    Component { id: pageMenu; MenuPage { } }
    Component { id: pageCover; CoverPage { } }
    Component { id: pageContacts; ContactsPage { } }
    Component { id: pageConversations; ConversationsPage { } }
    Component { id: pagePreferences; PreferencesPage { } }
    Component { id: pageAccount; AccountPage { } }
    Component { id: pageMessaging; MessagingPage { } }

    Component {
        id: messageNotification
        Notification {}
    }

    function newMessageNotification(id, summary, body) {
        var m = messageNotification.createObject(null)
        m.category = "harbour-shmoose-message"
        m.previewSummary = summary
        m.previewBody = body
        m.summary = summary
        m.body = body
        m.clicked.connect(function() {
            contactspage.activate()
            //contactspage.showConversation(id, PageStackAction.Immediate)
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
            "arguments": [ "id", id ]
        } ]
        m.publish()
    }

	Connections {
		target: shmoose.persistence.messageController
		onSignalMessageReceived: {
			newMessageNotification(id, jid, message);
		}
	}
}


