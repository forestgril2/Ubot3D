import QtQuick 2.15
import QtQuick.Controls 2.15
Row {
    property alias posXSlider: pointModelPosXSlider
    property alias mouseInvertCheckBox: mouseInvertCheckBox
    property alias commonRotationCheckBox: commonRotationCheckBox
    property alias pointModelRotationSlider: pointModelRotationSlider
    property alias triangleModelWarpSlider: triangleModelWarpSlider

    anchors {
        top: parent.top
        left: parent.left
    }

    Slider {
        id: triangleModelWarpSlider
        orientation: Qt.Vertical
        from: -0.1
        to: 0.1
        width: 50
    }

    Slider {
        id: pointModelPosXSlider
        orientation: Qt.Vertical
        from: -15
        to: 15
        width: 50
    }

    Slider {
        id: pointModelRotationSlider
        orientation: Qt.Vertical
        from: -180
        to: 180
        width: 50
    }

    Column {

        Button {
            id: snapToFloor

            text: "Snap model to floor"

            topInset: 20
            topPadding: 20
            height: 70

            onPressed: {
                stlModel.snapToFloor()
            }

        }

        CheckBox {
            id: commonRotationCheckBox
            text: qsTr("Rotate together")
            checkState: Qt.Unchecked
        }

        CheckBox {
            id: mouseInvertCheckBox
            text: qsTr("Invert mouse drag")
            checkState: Qt.Unchecked
        }
    }
}
