import QtQuick 2.15
import QtQuick.Controls 2.15
Row {
    property alias numSubpathsSlider: numSubpathsSlider
    property alias numPointsInSubpathSlider: numPointsInSubpathSlider

    property alias mouseInvertCheckBox: mouseInvertCheckBox
    property alias commonRotationCheckBox: commonRotationCheckBox
    property alias triangleModelWarpSlider: triangleModelWarpSlider

    property int numSubpaths
    property int numPointsInSubpaths

    Slider {
        id: triangleModelWarpSlider
        orientation: Qt.Vertical
        from: -0.1
        to: 0.1
        width: 50

        anchors {
            top: parent.top
            bottom: parent.bottom
//            left: parent.left
        }
    }

    Column {
        id: numSubpathsSliderColumn
        anchors {
            top: parent.top
            bottom: parent.bottom
//            left: triangleModelWarpSlider.right
        }

        TextField {
            id: numSubpathsSliderTextField
            overwriteMode: true
            width: numSubpathsSlider.width

            text: Math.round(numSubpathsSlider.value)

            onEditingFinished: {
                if (parseInt(text) !== NaN)
                {
                    numSubpathsSlider.value = parseInt(text)
                }

            }
        }

        Slider {
            id: numSubpathsSlider
            orientation: Qt.Vertical
            from: 0
            to: numSubpaths
            value: to
            width: 50
            height: parent.height - numSubpathsSliderTextField.height

//            anchors {
//                top: numSubpathsSliderTextField.bottom
//                bottom: parent.bottom
//            }
        }
    }

    Column {
        id: numPointsInSubpathSliderColumn
        anchors {
            top: parent.top
            bottom: parent.bottom
//            left: numSubpathsSliderColumn.right
        }

        TextField {
            id: numPointsInSubpathSliderTextField
            text: Math.round(numPointsInSubpathSlider.value)
            width: numPointsInSubpathSlider.width

            onEditingFinished: {
                if (parseInt(text) !== NaN)
                {
                    numPointsInSubpathSlider.value = parseInt(text)
                }

            }
        }

        Slider {
            id: numPointsInSubpathSlider
            orientation: Qt.Vertical
            from: 0
            to: numPointsInSubpaths
            width: 50
            height: parent.height - numPointsInSubpathSliderTextField.height

//             anchors {
//                top: numPointsInSubpathSliderTextField.bottom
//                bottom: parent.bottom
//            }
        }
    }

    Column {
        anchors {
            bottom: parent.bottom
//            left: numPointsInSubpathSliderColumn.right
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
