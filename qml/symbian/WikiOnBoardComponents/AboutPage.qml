import QtQuick 1.1

//SYMBIAN_SPECIFIC. For harmattan use: import com.nokia.meego 1.0
import com.nokia.android 1.1

import "UIConstants.js" as UI

TextPage {
    id: aboutPage
    anchors { fill: parent}

    property string wikionboardUrl : qsTr("http://cip.github.com/WikiOnBoard/")
    property string openDonatePageCaption : qsTr("Donate");
    property string openDonatePageUrl : qsTr("internal://donatePage");

    text: qsTr("WikiOnBoard %1<br>\
%5<br>\
Build date: %3<br>\
Author: %2<p>\
Uses zimlib (openzim.org) and liblzma.<p>\
%4\
If you like WikiOnBoard and want to support us click %6 for information how you can donate \
easily via nokia store","Use html tags for new line/paragraphs").replace(
              "%1",appInfo.version).replace(
              "%2",qsTr("Christian Puehringer")).replace(
              "%3",appInfo.buildDate).replace(
              "%4",appInfo.isSelfSigned?qsTr("application is self-signed", "only displayed if application is self-signed. Add <p> at end"):"").replace(
              "%5",getHtmlLink(wikionboardUrl, wikionboardUrl)).replace(
              "%6",getHtmlLink(openDonatePageCaption, openDonatePageUrl))
    tools: ToolBarLayout {
        ToolButton {
            iconSource: "toolbar-back"
            onClicked: pageStack.pop()
        }
    }
    onLinkActivated: {
        if (link == openDonatePageUrl) {
            pageStack.push(Qt.resolvedUrl("DonatePage.qml"))
        } else {
            openExternalLinkQueryDialog.askAndOpenUrlExternally(link, false);
        }
    }
}




