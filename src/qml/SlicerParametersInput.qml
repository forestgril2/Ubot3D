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
            id: paramGroupNameRepeater
            model: /*groupName[]*/ getVisibleParamGroupNames(root.paramGroups)

            Column {
                id: paramColumn

                Repeater {
                    id: paramRepeater
                    property string paramGroupName: modelData
                    property var paramGroup: getParamGroupWithName(root.paramGroups, /*groupName*/ modelData)
                    property int largestInputRowEditWidth: 0

                    model: /*params[]*/ getVisibleParamsInGroup(paramGroup)

                    ParameterInputRow {
                        id: parameterInput
                        paramData: modelData

                        Connections {
                            target: paramRepeater
                            function onLargestInputRowEditWidthChanged() {
                                parameterInput.largestEditWidth = paramRepeater.largestInputRowEditWidth
                            }
                        }

                        onEditFieldWidthChanged: {
                            if (editFieldWidth > paramRepeater.largestInputRowEditWidth) {
                                paramRepeater.largestInputRowEditWidth = editFieldWidth
                            }
                            else {
                                largestEditWidth = paramRepeater.largestInputRowEditWidth
                            }
                        }

                        Component.onCompleted: {
                            if (editFieldWidth > paramRepeater.largestInputRowEditWidth) {
                                paramRepeater.largestInputRowEditWidth = editFieldWidth
                            }
                            else {
                                largestEditWidth = paramRepeater.largestInputRowEditWidth
                            }
                        }

                        onParamValueChanged: {
                            const paramGroupIndex = getParamGroupIndexWithName(root.paramGroups, /*groupName*/ paramRepeater.paramGroupName)
                            const paramIndex = getParamIndex(paramRepeater.paramGroup.params, paramData)
                            paramGroups[paramGroupIndex].params[paramIndex].value = paramValue
                        }
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
            onPressed: {
                fileDialog.openExportJsonFileDialog()
            }
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

    function getParamIndex(params, param) {
        if (!params || 0 === params.length)
            return null

        for (var i=0; i<params.length; i++) {
            if (param.cliSwitchLong === params[i].cliSwitchLong &&
                param.cliSwitchShort === params[i].cliSwitchShort)
                return i
        }
        return null
    }

    function getParamGroupIndexWithName(paramGroups, name) {
        if (!paramGroups)
            return null

        for (var i=0; i<paramGroups.length; i++) {
            if (name === paramGroups[i].groupName)
                return i
        }
        return null
    }

    function getParamGroupWithName(paramGroups, name) {
        return paramGroups[getParamGroupIndexWithName(paramGroups, name) ]
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
