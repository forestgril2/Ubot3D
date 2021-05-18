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

    signal clearSceneRequested()

	menus: [
		Menu {
			title: qsTr("File")

			MenuItem {
                text: qsTr("Import model(s) as STL")
                onTriggered: fileDialog.openImportStlFileDialog()
			}
            MenuItem {
                text: qsTr("Export model(s) to STL")

                onTriggered: fileDialog.openExportStlFileDialog()
            }
			MenuItem{
                text: qsTr("Import GCode")
                onTriggered: fileDialog.openImportGcodeFileDialog()
			}
			MenuItem {
				text: qsTr("Recently Opened...")
			}
			MenuItem{
                text: qsTr("Clear scene")
                onTriggered: {
                    mainMenu.clearSceneRequested()
                }
			}
			MenuSeparator {}
			MenuItem{
				text: qsTr("Exit")
                onTriggered: {
                    window.close()
                }
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
                text: qsTr("Parameters")
                onTriggered: slicerParameters.show()
			}
			MenuItem{
                text: qsTr("Slice selected model")
                onTriggered: slicerProcessLauncher.sliceSelectedModel()
			}
			MenuItem{
				text: qsTr("???")
			}
		},
		Menu {
			title: qsTr("Options")
			
			MenuItem {
				text: qsTr("Support options")
                onTriggered: supportOptions.show()
			}
			MenuItem{
                text: qsTr("Raft options")
                onTriggered: raftOptions.show()
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
