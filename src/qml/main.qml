import QtQml 2.3
import Qt.labs.platform 1.1
import QtQuick3D.Helpers 1.15
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick3D 1.15
import QtQuick.Controls 2.15
import customgeometry 1.0

Window {
    id: window

    property vector3d sceneCenter: Qt.vector3d(100, 100, 0)

    width: 1280
    height: 720
    visible: true
    color: "#848895"

    TypedFileDialog {
        id: fileDialog
    }

    MainMenu {
        id: mainMenu

        onModelCloseRequested: {
            stlModels.model = []
            gcodeModels.model = []
        }
    }

    View3D {
        id: view3d
        anchors.fill: parent
        camera: sceneBase.camera

        property var gcodeModel
        property vector3d gcodeModelCenter: Qt.vector3d(0, 0, 0)

        SceneBase {
            id: sceneBase
        }

        Ubot3DCameraWasdController {
            id: controller
            mouseEnabled: !modelGroupDrag.isActive
            controlledObject: sceneBase.camera
            camera: sceneBase.camera
            isMouseDragInverted: modelControls.mouseInvertCheckBox.checked
        }

        Repeater3D {
            id: stlModels

            delegate: StlModel {
                id: stlModel
                inputFile: stlModels.model[index]
            }

            function deselectAll() {
                for (var i = 0; i < stlModels.count; i++)
                {// Find out, if we are pressing an object.
                    var stlModel = stlModels.objectAt(i)
                    stlModel.isPicked = false
                }
            }
        }

        Repeater3D {
            id: gcodeModels
            property var gcodeGeometry: (objectAt(0) === null ? null : objectAt(0).geometry)
            model: []

            delegate: Model {
                id: thisModel

                property bool isPicked: false
                position: Qt.vector3d(0, 0, 0)
                objectName: "gCode geometry"
                pickable: true
                rotation: modelControls.commonRotationCheckBox.checked ?
                              helper3D.getRotationFromAxisAndAngle(Qt.vector3d(0,0,1), modelControls.pointModelRotationSlider.value) :
                              Qt.quaternion(0,0,0,0)

                geometry: GCodeGeometry {
                    id: gcodeGeometry
                    inputFile: gcodeModels.model[index]

                    onModelLoaded: {
                        modelControls.resetSliders(gcodeGeometry)
                        view3d.gcodeModel = thisModel
                    }
                }
                materials: [
                    DefaultMaterial {
                        cullMode: DefaultMaterial.NoCulling
                        diffuseColor: "lightgreen"
                        specularAmount: 0.5
                    }
                ]
            }
        }

        PickArea {
            id: pickArea
            camera: sceneBase.camera
        }

        ModelGroupDrag {
            id: modelGroupDrag
            connectedPickArea: pickArea
        }

        Connections {
            target: pickArea
            enabled: target
            function onPressed(mouse) {
                modelGroupDrag.start(mouse)
            }
            function onReleased(mouse) {
                modelGroupDrag.finish()
            }
            function onPositionChanged(mouse) {
                if(modelGroupDrag.isActive) {
                    modelGroupDrag.dragPositionChanged(pickArea.getOriginAndRay(mouse.x, mouse.y))
                    pickArea.isNextMouseClickDisabled = true
                }
            }
        }

        ModelControls {
            id: modelControls

            property var referencedGcodeGeometry
            anchors {
                left: parent.left
                top: parent.top
                bottom: parent.bottom
                topMargin: 10
                bottomMargin: 10
                leftMargin: 10
            }

            numSubPathsSlider.onValueChanged: {
                if (!referencedGcodeGeometry)
                    return;
                referencedGcodeGeometry.numSubPaths = numSubPathsSlider.value
            }

            numPointsInSubPathSlider.onValueChanged: {
                if (!referencedGcodeGeometry)
                    return;
                referencedGcodeGeometry.numPointsInSubPath = numPointsInSubPathSlider.value
            }

            numPathStepsUsedSlider.onValueChanged: {
                if (!referencedGcodeGeometry)
                    return;
                referencedGcodeGeometry.numPathStepsUsed = numPathStepsUsedSlider.value
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

        Component.onCompleted: {
            if (gcodeModel) {
                camera.lookAtModel(gcodeModel)
                modelControls.resetSliders()
            }
            else {
                camera.lookAtPoint(Qt.vector3d(50,50,0))
            }
        }
    }

    function getModelCenter(model) {
        return model.geometry.minBounds.plus(model.geometry.maxBounds).times(0.5)
    }
}
