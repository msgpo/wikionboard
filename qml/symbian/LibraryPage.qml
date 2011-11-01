import QtQuick 1.0
////import com.nokia.meego 1.0

import com.nokia.symbian 1.0

import "UIConstants.js" as UI

WikionboardPage {
    id: library
    signal openZimFile(string fileName);
    property string photoAuthor
    property string photoDate
    property string photoDescription
    property string photoTags
    property string photoTitle
    property int photoHeight
    property int photoWidth

    function setPhotoData(author, date, description, tags, title, photoWidth, photoHeight) {
        details.photoAuthor = author;
        details.photoDate = date;
        details.photoDescription = description;
        details.photoTags = tags;
        details.photoTitle = title;
        details.photoHeight = photoHeight;
        details.photoWidth = photoWidth;
    }
    function addZimFile(fileName) {
        libraryModel.addZimFile(fileName);
    }

    LibraryModel {
        id: libraryModel
    }


    Component {
        id: listHeading
        ListHeading {
            width: parent.width
            ListItemText {
                anchors.fill: parent.paddingItem
                role: "Heading"
                text: "eBook Library"
            }
        }
    }
    Component {
        id: listDelegate



        //
        ListItem {
            id: listItem
            Column {
                anchors.fill: listItem.paddingItem

                ListItemText {
                    function getTitle() {
                        return title
                    }
                    mode: listItem.mode
                    role: "Title"
                    text: getTitle()
                }
                ListItemText {
                    function getSubTitle() {
                        return date+" "+language+" "+description
                    }

                    mode: listItem.mode
                    role: "SubTitle"
                    text: getSubTitle()
                }
            }
            onClicked: library.openZimFile(fileName)
            onPressAndHold: {
                console.log("in pressAndHold: index "+index)
                libraryItemMenu.openMenu(index)
            }
        }
    }

    ContextMenu {
           id: libraryItemMenu
           property int itemIndex
           itemIndex: -1
           MenuLayout {
               MenuItem {
                   text: "Remove"
                   onClicked: {
                       console.log(" Remove item "+libraryItemMenu.itemIndex)
                       libraryModel.remove(libraryItemMenu.itemIndex)
                    }
                }
           }

           function openMenu(index) {
                   itemIndex = index
                   console.log("in openMenu: index "+itemIndex);
                   open();
            }
    }

    ListView {
        id: libraryListView
        //anchors.horizontalCenter: parent.horizontalCenter
        anchors { fill: parent; bottomMargin: parent.height*1/5}
        clip: true
        delegate: listDelegate
        model: libraryModel
        header: listHeading
    }

    Column {
        anchors.top: libraryListView.bottom
        anchors.topMargin: UI.MARGIN_XLARGE
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right:  parent.right

        Button {
            id: addEBookButton
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Find eBook on Phone")
        }
        Button {
            id: downloadEBookButton
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Download eBook")
        }
    }
}
