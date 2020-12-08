import QtQuick 2.15
import QtQuick.Controls 2.15

Column {
    id: root
    property alias to: slider.to
    property alias from: slider.from
    property alias value: slider.value

    anchors {
        top: parent.top
        bottom: parent.bottom
    }

    TextField {
        id: textField
        text: Math.round(root.value)
        width: slider.width

        onEditingFinished: {
            if (parseInt(text) !== NaN)
            {
                root.value = parseInt(text)
            }
        }
    }

    Slider {
        id: slider
        orientation: Qt.Vertical
        width: 50
        height: root.height - textField.height
    }
}
