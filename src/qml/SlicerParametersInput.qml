import QtQml 2.3
import Qt.labs.platform 1.1
import QtQuick3D.Helpers 1.15
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick3D 1.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Window {
    id: root
    property var isUsingTwoExtruders: false
    property var paramGroups
    property var visibleParamGroups: getVisibleParamGroups(paramGroups)
    property var visibleParamGroupNames: getParamGroupNames(visibleParamGroups)

    onVisibleParamGroupsChanged: {
        console.log(" ### onVisibleParamGroupsChanged:")
    }

    minimumWidth: 300
    minimumHeight: paramGroupSelectorTabs.height + paramGroupEditFormStack.height + buttons.height
    height: minimumHeight
	title: "Input slicer parameters"


    TabBar {
        id: paramGroupSelectorTabs
         width: parent.width
         Repeater {
             id: tabBarRepeater
             model: visibleParamGroupNames
             TabButton {
                 text: /*groupName*/ modelData
             }
         }
    }

    StackLayout {
        id: paramGroupEditFormStack
        width: parent.width
        currentIndex: paramGroupSelectorTabs.currentIndex
        anchors.top: paramGroupSelectorTabs.bottom
        anchors.topMargin: 4
        anchors.left: paramGroupSelectorTabs.left
        anchors.leftMargin: 4

        Repeater {
            id: paramGroupEditForms
            model: visibleParamGroups

            ParamGroupEdit {
                id: paramGroupEditForm
                paramGroup: modelData

                onParamValueChanged: {
                    const paramGroupIndex = getParamGroupIndexWithName(paramGroups, /*groupName*/ paramInputGridElementRepeater.paramGroupName)
                    const paramIndex = getParamIndex(paramInputGridElementRepeater.paramGroup.params, paramData)
                    paramGroups[paramGroupIndex].params[paramIndex].value = paramValue
                }
            }
        }
    }

    //TODO: Change Window to Popup, where all these buttons are predefined.
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
            onPressed: {
                fileDialog.openExportJsonFileDialog()
            }
        }
        Button {
            text: "Ok"
            onPressed: {
                fileDialog.openExportJsonFileDialog()
                root.close()
            }
        }
        Button {
            text: "Apply"
            onPressed: {
                fileDialog.openExportJsonFileDialog()
            }
        }
        Button {
            text: "Cancel"
            onPressed: {
                root.close()
            }
        }
    }

    function getVisibleParamGroups(paramGroups) {
        if (!paramGroups)
            return []

        var visible = []
        for (var i=0; i<paramGroups.length; i++) {
            if (!paramGroups[i].isVisible)
                continue
            visible.push(paramGroups[i])
        }
        return visible
    }

    function getParamGroupNames(paramGroups) {
        var names = []
        for (var i=0; i<paramGroups.length; i++) {
            names.push(paramGroups[i].groupName)
        }
        return names
    }
}
