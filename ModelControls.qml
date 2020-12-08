import QtQuick 2.15
import QtQuick.Controls 2.15
Row {
    property alias numSubPathsSlider: numSubPathsSlider
    property alias numPointsInSubPathSlider: numPointsInSubPathSlider
    property alias numPathPointsUsedSlider: numPathPointsUsedSlider

    property alias mouseInvertCheckBox: mouseInvertCheckBox
    property alias commonRotationCheckBox: commonRotationCheckBox
    property alias triangleModelWarpSlider: triangleModelWarpSlider

    property int numSubPaths
    property int numPointsInSubPath
    property int numPathPointsUsed

    Slider {
        id: triangleModelWarpSlider
        orientation: Qt.Vertical
        from: -0.1
        to: 0.1
        width: 50

        anchors {
            top: parent.top
            bottom: parent.bottom
        }
    }

    Column {
        id: numSubPathsSliderColumn
        anchors {
            top: parent.top
            bottom: parent.bottom
        }

        TextField {
            id: numSubPathsSliderTextField
            overwriteMode: true
            width: numSubPathsSlider.width

            text: Math.round(numSubPathsSlider.value)

            onEditingFinished: {
                if (parseInt(text) !== NaN)
                {
                    numSubPathsSlider.value = parseInt(text)
                }

            }
        }

        Slider {
            id: numSubPathsSlider
            orientation: Qt.Vertical
            from: 0
            to: numSubPaths
            value: to
            width: 50
            height: parent.height - numSubPathsSliderTextField.height
        }
    }

    Column {
        id: numPointsInSubPathSliderColumn
        anchors {
            top: parent.top
            bottom: parent.bottom
        }

        TextField {
            id: numPointsInSubPathSliderTextField
            text: Math.round(numPointsInSubPathSlider.value)
            width: numPointsInSubPathSlider.width

            onEditingFinished: {
                if (parseInt(text) !== NaN)
                {
                    numPointsInSubPathSlider.value = parseInt(text)
                }

            }
        }

        Slider {
            id: numPointsInSubPathSlider
            orientation: Qt.Vertical
            from: 0
            to: numPointsInSubPath
            width: 50
            height: parent.height - numPointsInSubPathSliderTextField.height
        }
    }

    ValueEditSlider {
        id: numPathPointsUsedSlider
        anchors {
            top: parent.top
            bottom: parent.bottom
        }

        from: 0
        to: numPathPointsUsed
    }

    Column {
        anchors {
            bottom: parent.bottom
        }

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
            checkState: Qt.Checked
        }
    }
}
