import QtQuick 2.12
import QtQuick.Window 2.12
import Qt.labs.platform 1.1
import QtQuick.Controls 2.12

Window {
    visible: true
    width: 640
    height: 480
    title: qsTr("Krice3D")

    MenuBar {
        menus: [
            Menu {
                title: qsTr("File")

                MenuItem {}
                MenuItem {}
                MenuItem {}
            },
            Menu {
                title: qsTr("Tools")

                MenuItem {}
                MenuItem {}
                MenuItem {}
            },
            Menu {
                title: qsTr("Slicer")

                MenuItem {}
                MenuItem {}
                MenuItem {}
            },
            Menu {
                title: qsTr("Options")

                MenuItem {}
                MenuItem {}
                MenuItem {}
            },
            Menu {
                title: qsTr("Help")

                MenuItem {}
                MenuItem {}
                MenuItem {}
            }
            ]
    }
}
