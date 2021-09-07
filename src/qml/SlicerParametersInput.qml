import QtQml 2.3
import Qt.labs.platform 1.1
import QtQuick3D.Helpers 1.15
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick3D 1.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import customgeometry 1.0
import "HelperFunctions.js" as QmlHelpers

Window {
    id: root
    property var isUsingTwoExtruders: false
    property var paramGroups: []

    minimumWidth: 300
	title: "Input slicer parameters"

    TabBar {
        id: paramGroupsTabs
         width: parent.width
         Repeater {
             id: tabBarRepeater
             model: getParamGroupNames(root.paramGroups)
             TabButton {
                 text: modelData
             }
         }
    }

    StackLayout {
        id: stackLayout
        width: parent.width
        anchors.top: paramGroupsTabs.bottom
        currentIndex: paramGroupsTabs.currentIndex

        Repeater {
            id: paramGroupRepeater
            model: getParamsInGroup(paramGroups[stackLayout.currentIndex])
            Item {
                Switch {
                    id: twoExtrudersSwitch
                    property bool isUsingTwoExtruders: (position == 1.0)
                    text: "Use two extruders"
                }

            }
        }
    }

    function getParamGroupNames(paramGroups) {
        if (!paramGroups)
            return []

        var names = []
        for (var i=0; i<paramGroups.length; i++) {
            names.push(paramGroups[i].groupName)
        }
        return names
    }

    function getParamsInGroup(paramGroup) {
        return paramGroup.params
    }
}
