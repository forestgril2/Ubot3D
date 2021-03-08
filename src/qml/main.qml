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

    PickDebugs {
        id: pickDebugs
    }

    View3D {
        id: view3d
        anchors.fill: parent
        camera: camera

        property var gcodeModel
        property vector3d gcodeModelCenter: Qt.vector3d(0, 0, 0)

        Component.onCompleted: {
            if (gcodeModel) {
                camera.lookAtModel(gcodeModel)
                modelControls.resetSliders()
            }
            else {
                camera.lookAtPoint(Qt.vector3d(50,50,0))
            }
        }

        Ubot3DCameraWasdController {
            id: controller
            mouseEnabled: !pickArea.isDraggingModelGroup
            controlledObject: camera
            camera: camera
            isMouseDragInverted: modelControls.mouseInvertCheckBox.checked
        }

        PerspectiveCamera {
            id: camera

            property vector3d initDistToModel: Qt.vector3d(-150, -150, 100)

            fieldOfView: 45
            clipNear: 0.1
            clipFar: 1000.0
            position: sceneCenter.plus(initDistToModel)

            function lookAtPoint(point)
            {
                var direction = point.minus(camera.position)
                var upDirection = Qt.vector3d(0,0,1)
                var lookAtRotation = helper3D.getRotationFromDirection(direction, upDirection)
                camera.rotation = lookAtRotation
            }
        }

        DirectionalLight {
            eulerRotation.x: 30
            eulerRotation.y: 30
            eulerRotation.z: 30
            color: Qt.rgba(0.7, 0.7, 0.7, 1.0)
            ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0)
        }

        DirectionalLight {
            eulerRotation.x: -45
            eulerRotation.y: -45
            eulerRotation.z: -45
            color: Qt.rgba(0.3, 0.3, 0.3, 1.0)
            ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0)
        }

        PointLight {
            position: sceneCenter.plus(Qt.vector3d(30, 0, 100))
            color: Qt.rgba(0.1, 1.0, 0.1, 1.0)
            ambientColor: Qt.rgba(0.2, 0.2, 0.2, 1.0)
        }

        Model {
            scale: Qt.vector3d(100, 100, 100)
            position: sceneCenter
            geometry: GridGeometry {
                id: grid
                horizontalLines: 20
                verticalLines: 20
            }
            materials: [
                DefaultMaterial {
                }
            ]
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

//                        console.log(" ### gcodeModels.gcodeGeometry.inputFile:" + gcodeModels.gcodeGeometry.inputFile)
//                        console.log(" ### gcodeModels.gcodeGeometry.numSubPaths:" + gcodeModels.gcodeGeometry.numSubPaths)

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

        Repeater3D {
            id: stlModels

            delegate: StlModel {
                id: stlModel
                inputFile: stlModels.model[index]
            }
        }

        PickArea {
            id: pickArea
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
    }

    function getModelCenter(model) {
        return model.geometry.minBounds.plus(model.geometry.maxBounds).times(0.5)
    }
}
