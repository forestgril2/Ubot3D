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
    minimumHeight: paramGroupsTabs.height + stackLayout.height + buttons.height
    height: minimumHeight
	title: "Input slicer parameters"

    TabBar {
        id: paramGroupsTabs
         width: parent.width
         Repeater {
             id: tabBarRepeater
             model: getVisibleParamGroupNames(root.paramGroups)
             TabButton {
                 text: /*groupName*/ modelData
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
            model: getVisibleParamGroupNames(root.paramGroups)

            Column {
                id: paramColumn

                Repeater {
                    id: paramRowRepeater
                    model: getVisibleParamsInGroup(getParamGroupWithName(root.paramGroups, /*groupName*/ modelData))

                    ParameterRow {
                        id: parameterRow
                        param: /*parameter*/ modelData
                    }

                }

            }
        }
    }

    Row {
        id: buttons

        anchors {
            bottom: parent.bottom
            right: parent.right
        }

        padding: 4
        spacing: 17

        Button {
            text: "Load"
            onPressed: {
                fileDialog.openImportJsonFileDialog()
            }
        }
        Button {
            text: "Save"
        }
        Button {
            text: "Ok"
        }
        Button {
            text: "Apply"
        }
        Button {
            text: "Cancel"
        }
    }

    function getVisibleParamGroupNames(paramGroups) {
        if (!paramGroups)
            return []

        var names = []
        for (var i=0; i<paramGroups.length; i++) {
            if (!paramGroups[i].isVisible)
                continue
            names.push(paramGroups[i].groupName)
        }
        return names
    }

    function getParamGroupWithName(paramGroups, name) {
        if (!paramGroups)
            return {}

        for (var i=0; i<paramGroups.length; i++) {
            if (name === paramGroups[i].groupName)
                return paramGroups[i]
        }
        return {}
    }

    function getVisibleParamsInGroup(paramGroup) {
        if (!paramGroups)
            return []

        var visibleParams = []
        for (var i=0; i<paramGroup.params.length; i++) {
            if (!paramGroup.params[i].isVisible)
                continue
            visibleParams.push(paramGroup.params[i])
        }

        return visibleParams
    }
}
