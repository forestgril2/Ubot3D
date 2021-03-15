import QtQuick 2.15
import QtQuick.Controls 2.15
Row {

    property alias numSubPathsSlider: numSubPathsSlider
    property alias numPointsInSubPathSlider: numPointsInSubPathSlider
    property alias numPathStepsUsedSlider: numPathStepsUsedSlider

    property alias mouseInvertCheckBox: mouseInvertCheckBox
    property alias commonRotationCheckBox: commonRotationCheckBox
    property alias triangleModelWarpSlider: triangleModelWarpSlider

    property var referencedGcodeGeometry
    property int numSubPaths
    property int numPointsInSubPath
    property int numPathStepsUsed

    anchors {
        left: parent.left
        top: parent.top
        bottom: parent.bottom
        topMargin: 10
        bottomMargin: 10
        leftMargin: 10
    }

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

    ValueEditSlider {
        id: numSubPathsSlider
        from: 0
        to: numSubPaths

        onValueChanged: {
            if (!referencedGcodeGeometry)
                return;
            referencedGcodeGeometry.numSubPaths = numSubPathsSlider.value
        }
    }

    ValueEditSlider {
        id: numPointsInSubPathSlider

        from: 0
        to: numPointsInSubPath

        onValueChanged: {
            if (!referencedGcodeGeometry)
                return;
            referencedGcodeGeometry.numPointsInSubPath = numPointsInSubPathSlider.value
        }
    }

    ValueEditSlider {
        id: numPathStepsUsedSlider

        from: 0
        to: numPathStepsUsed

        onValueChanged: {
            if (!referencedGcodeGeometry)
                return;
            referencedGcodeGeometry.numPathStepsUsed = numPathStepsUsedSlider.value
        }
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
                var numModels = stlModels.count

                for (var i = 0; i < numModels; i++)
                {
                    var stlModel = stlModels.objectAt(i);
                    stlModel.snapToFloor()
                }
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

    function resetSliders(gcodeGeometry) {

        referencedGcodeGeometry = gcodeGeometry;

        console.log(" ### resetSliders(): gcodeModels.gcodeGeometry.inputFile:" + gcodeGeometry.inputFile)

        numSubPaths = gcodeGeometry.numSubPaths
        numSubPathsSlider.value = gcodeGeometry.numSubPaths
        numPointsInSubPath = gcodeGeometry.numPointsInSubPath
        numPointsInSubPathSlider.value = gcodeGeometry.numPointsInSubPath
        numSubPaths = gcodeGeometry.numSubPaths
        numPathStepsUsed = gcodeGeometry.numPathStepsUsed
        numPathStepsUsedSlider.value = gcodeGeometry.numPathStepsUsed
    }
}
