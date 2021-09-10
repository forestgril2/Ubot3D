import QtQml 2.3
import Qt.labs.platform 1.1
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.2

Row {
    id: root
    property var paramData
    padding: 4
    spacing: 11

    signal dataChanged()

    Frame {
        width: 100
        RowLayout {
            anchors.fill: parent
            TextInput {
                id: textField
                text: paramData.value ? paramData.value : paramData.defaultValue
                Layout.alignment: Qt.AlignRight
                horizontalAlignment: TextInput.AlignRight
                rightPadding: root.padding
                onEditingFinished: {
                    if (parseInt(text) !== NaN)
                    {
                        paramData.value = parseFloat(text)
                        console.log(" paramData.value: ", paramData.value)
                        dataChanged()
                    }
                }
            }
        }
    }

    Label {
        text: paramData.text ? paramData.text : paramData.name ? paramData.name : paramData.description
    }

}
