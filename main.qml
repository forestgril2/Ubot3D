import QtQuick 2.12
import QtQuick.Window 2.12
import Qt.labs.platform 1.1
import QtQuick.Controls 2.12

Window {
    visible: true
    width: 640
    height: 480
    title: "Krice3D"

    MenuBar {
        menus: [
            Menu {
                title: qsTr("File")

                MenuItem {
                    text: qsTr("Open STL")
                }
                MenuItem{
                    text: qsTr("Open GCode")
                }
                MenuItem{
                    text: qsTr("Close file")
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
}
