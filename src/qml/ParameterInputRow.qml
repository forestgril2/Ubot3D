import QtQml 2.3
import Qt.labs.platform 1.1
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.2

Row {
    id: root
    property var paramData
    property var paramValue: paramData.value
    padding: 4
    spacing: 11

    Frame {
        width: 100
            TextInput {
                id: textField
                anchors.fill: parent
//                width: parent.width
                text: paramValue !== "" ? paramValue : root.paramData.defaultValue
                Layout.alignment: Qt.AlignRight
                horizontalAlignment: TextInput.AlignRight
                rightPadding: root.padding
                onEditingFinished: {
                    if (parseInt(text) !== NaN)
                    {
                        paramValue = parseFloat(text)
                        console.log(" paramValue: ", paramValue)
                        console.log(" paramData.value: ", root.paramData.value)
                    }
                }
            }
    }

    Label {
        text: root.paramData.text ? root.paramData.text : root.paramData.name ? root.paramData.name : root.paramData.description
    }

}
