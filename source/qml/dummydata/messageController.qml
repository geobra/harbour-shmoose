import QtQuick 2.3

ListModel {
	ListElement { message: "Hi"; direction: 1; msgstate: 1 }
	ListElement { message: "Hello"; direction: 0; msgstate: 2 }
	ListElement { message: "How are you?"; direction: 1; msgstate: 2 }
	ListElement { message: "This is a very long text This is a very long text This is a very long text This is a very long text This is a very long text This is a very long text This is a very long text This is a very long text This is a very long text This is a very long text This is a very long text This is a very long text This is a very long text This is a very long text "; direction: 0; msgstate: 3 }
	ListElement { message: "<b>bold</b> <i>italic</i>"; direction: 1; msgstate: 2 }
	ListElement { message: "<a href=\"http://www.heise.de\">heise.de</a>"; direction: 0; msgstate: 2 }
	ListElement { message: "http://www.heise.de"; direction: 0; msgstate: 2 }
}

