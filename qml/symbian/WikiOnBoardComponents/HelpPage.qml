import QtQuick 1.1

//SYMBIAN_SPECIFIC. For harmattan use: import com.nokia.meego 1.0
import com.nokia.symbian 1.1

import "UIConstants.js" as UI

TextPage {
    id: helpPage
    anchors { fill: parent}
    signal findEbookClicked();
    signal openExternalLink(string url);
    //% "Change link to page with localized zim files." (e.g http://cip.github.com/WikiOnBoard/Get-eBooks-DE.html"
    property string zimDownloadUrl : qsTr("http://cip.github.com/WikiOnBoard/Get-eBooks.html");
    property string getEBookLinkCaption : qsTr("Download zimfile");
    property string findEBookUrl : "internal://findEBook";
    property string findEBookCaption : qsTr("Find eBook on Phone"); //TODO: Overhead as proprties?

    text: qsTr("[TRANSLATOR] No zimfile selected. getEBook link  %1 opens url %3 with info where to get eBooks. Click %2 to open zimfile on mobile").replace(
              "%1",getHtmlLink(getEBookLinkCaption, zimDownloadUrl)).replace(
              "%2",getHtmlLink(findEBookCaption, findEBookUrl))
    onLinkActivated: {
        console.log("helpPage.linkActivated: "+link)
        //TODO internal links to text page?
        if (link == findEBookUrl){
            findEbookClicked()
        } else {
            openExternalLink(link)
        }
    }
    tools: backOnlyTools


    onFindEbookClicked: {
        //pageStack.pop(); FIXME: works as expected regarding page, but toolbar is cleared :(
        pageStack.push(Qt.resolvedUrl("ZimFileSelectPage.qml"))
    }
    onOpenExternalLink: {
        openExternalLinkQueryDialog.askAndOpenUrlExternally(url, false);
    }



}

