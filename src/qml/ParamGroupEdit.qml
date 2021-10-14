import QtQml 2.3
import Qt.labs.platform 1.1
import QtQuick3D.Helpers 1.15
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick3D 1.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Grid {
    id: root

    property var paramGroup
    property var visibleParams: paramGroup.params.filter(function(param){return param.isVisible})

    signal paramValueChanged(var param, var value)

    flow: GridLayout.LeftToRight
    horizontalItemAlignment: Qt.AlignLeft
    verticalItemAlignment: Qt.AlignVCenter
    columns: 3
    spacing: 6

    Repeater {
        id: paramInputGridElementRepeater
        model: getParamGridElementSpecifiersInOrder(visibleParams)
        delegate: ParamRowElement {
            specifier: modelData

            onParamValueChanged: root.paramValueChanged(param, value)
        }
    }

    function getParamGridElementSpecifiersInOrder(params) {
        if (!params)
            return []

        var specifiers = []
        for (var i=0; i<params.length; i++) {
            var paramData = params[i]
            var firstInputspecifier = {paramData: paramData, isFirstParam: true}
            specifiers.push(firstInputspecifier)

            var secondInputspecifier = paramData.isExtruderParam ? {paramData: paramData, isFirstParam: false} : null
            specifiers.push(secondInputspecifier)

            var paramNameSpecifier = {paramName: paramData.text ? paramData.text : paramData.name ? paramData.name : paramData.description}
            specifiers.push(paramNameSpecifier)
        }

        return specifiers
    }
}
