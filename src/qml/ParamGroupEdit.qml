import QtQml 2.3
import Qt.labs.platform 1.1
import QtQuick3D.Helpers 1.15
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick3D 1.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

GridLayout {
    id: paramInputRows

    property var paramGroup

    signal paramValueChanged(var param, var value)

    // First, we will fill param edit column(s) and next - param names/descriptions column.
    flow: GridLayout.TopToBottom
    rows: getVisibleParamsInGroup(paramGroup).length


    Repeater {
        id: paramInputGridElementRepeater

//        property string paramGroupName: modelData
//        property var paramGroup: getParamGroupWithName(paramGroups, /*groupName*/ modelData)
//        model: /*params[]*/ getVisibleParamsInGroup(paramGroup)

        model: getParamGridColumnElementsInOrder(paramGroup)

        ParameterInputRow {
            id: parameterInput
            paramData: modelData

            onParamValueChanged: {
                paramInputRows.paramValueChanged(param, paramValue)
                const paramGroupIndex = getParamGroupIndexWithName(paramGroups, /*groupName*/ paramInputGridElementRepeater.paramGroupName)
                const paramIndex = getParamIndex(paramInputGridElementRepeater.paramGroup.params, paramData)
                paramGroups[paramGroupIndex].params[paramIndex].value = paramValue
            }
        }
    }

    function getParamGridColumnElementsInOrder(paramGroup) {

    }

//    function getParamGroupWithName(paramGroups, name) {
//        return paramGroups[getParamGroupIndexWithName(paramGroups, name) ]
//    }

    function getVisibleParamsInGroup(paramGroup) {
        if (!paramGroup)
            return []

        var visibleParams = []
        for (var i=0; i<paramGroup.params.length; i++) {
            if (!paramGroup.params[i].isVisible)
                continue
            visibleParams.push(paramGroup.params[i])
        }
        return visibleParams
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
}
