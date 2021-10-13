import QtQml 2.3
import Qt.labs.platform 1.1
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.2

Row {
    id: root
    property var paramData
    property var paramValue: paramData.value
    property var editFieldWidth: parameterEditRepeater.width
    property var largestEditWidth: 0

    padding: 4
    spacing: 11




}
