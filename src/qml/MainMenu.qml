import Qt.labs.platform 1.1
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick3D 1.15
import QtQuick.Controls 2.15

MenuBar {
//	anchors {
//		left: parent.left
//		top: parent.top
//		right: parent.right
//	}
//	height: 30
	
	id: mainMenu

    signal modelCloseRequested()

	menus: [
		Menu {
			title: qsTr("File")

			MenuItem {
                text: qsTr("Import model(s) as STL")
                onTriggered: fileDialog.importStl()
			}
            MenuItem {
                text: qsTr("Export selecion to STL")

                onTriggered: fileDialog.exportStl()
            }
			MenuItem{
                text: qsTr("Import GCode")
                onTriggered: fileDialog.importGcode()
			}
			MenuItem {
				text: qsTr("Recently Opened...")
			}
			MenuItem{
				text: qsTr("Close file")
                onTriggered: {
                    mainMenu.modelCloseRequested()
                }
			}
			MenuSeparator {}
			MenuItem{
				text: qsTr("Exit")
			}
		},
		Menu {
			title: qsTr("Tools")
			
			MenuItem {
				text: qsTr("STL tools")
			}
			MenuItem{
				text: qsTr("ToolsMenuItem 2")
			}
			MenuItem{
				text: qsTr("ToolsMenuItem 3")
			}
		},
		Menu {
			title: qsTr("Slicer")
			
			MenuItem {
				text: qsTr("Edit slicer options")
			}
			MenuItem{
				text: qsTr("Generate GCode")
			}
			MenuItem{
				text: qsTr("???")
			}
		},
		Menu {
			title: qsTr("Options")
			
			MenuItem {
				text: qsTr("Support options")
			}
			MenuItem{
				text: qsTr("Options item 2")
			}
			MenuItem{
				text: qsTr("Options item 3")
			}
		},
		Menu {
			title: qsTr("Help")
			
			MenuItem {
				text: qsTr("Documentation")
			}
			MenuItem{
				text: qsTr("Licensing")
			}
			MenuSeparator {}
			MenuItem{
				text: qsTr("About")
			}
		}
	]
}
