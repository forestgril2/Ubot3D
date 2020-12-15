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
    }

    PickDebugs {
        id: pickDebugs
    }

    View3D {
        id: view3d
        anchors.fill: parent
        camera: camera

        property vector3d gcodeModelCenter: Qt.vector3d(0, 0, 0)

        Component.onCompleted: {
            console.log(" ### ComponentComplete")
            view3d.gcodeModelCenter = getModelCenter(gcodeModel)
            console.log(" ### view3d.gcodeModelCenter :" + view3d.gcodeModelCenter)
//            gcodeModel.position = Qt.vector3d(0, 0, 0).minus(view3d.gcodeModelCenter)
            camera.lookAtModel(gcodeModel)

            modelControls.numSubPaths = gcodeGeometry.numSubPaths
            modelControls.numSubPathsSlider.value = gcodeGeometry.numSubPaths
            modelControls.numPointsInSubPath = gcodeGeometry.numPointsInSubPath
            modelControls.numPointsInSubPathSlider.value = gcodeGeometry.numPointsInSubPath
            modelControls.numSubPaths = gcodeGeometry.numSubPaths
            modelControls.numPathPointsUsed = gcodeGeometry.numPathPointsUsed
            modelControls.numPathPointsUsedSlider.value = gcodeGeometry.numPathPointsUsed
        }

        Ubot3DCameraWasdController {
            id: controller
            controlledObject: camera
            camera: camera
            isMouseDragInverted: modelControls.mouseInvertCheckBox.checked
        }

        PerspectiveCamera {
            id: camera

            property vector3d initDistToModel: Qt.vector3d(0, -75, 50)

            fieldOfView: 45
            clipNear: 0.1
            clipFar: 1000.0
            position: sceneCenter.plus(initDistToModel)


            function lookAtModel(model)
            {
                var modelCenter = getModelCenter(model)
                console.log(" ### lookAtModel: " + model.objectName + ", modelCenter: " + modelCenter);
                console.log(" ### " + gcodeModel.objectName + " modelLoaded with paths: " + gcodeGeometry.numSubPaths + ", max points in subPath: " + gcodeGeometry.numPointsInSubPath)
                var direction = modelCenter.minus(camera.position)
                var upDirection = Qt.vector3d(0,0,1)
                var lookAtModelCenterRotation = stlModel.geometry.getRotationFromDirection(direction, upDirection)
                camera.rotation = lookAtModelCenterRotation
            }
        }

        DirectionalLight {
            position: sceneCenter.plus(Qt.vector3d(50, 50, -10))
            color: Qt.rgba(0.4, 0.2, 0.6, 1.0)
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

        Model {
            id: gcodeModel
            property bool isPicked: false
            position: Qt.vector3d(0, 0, 0)
            objectName: "gCode geometry"
            pickable: true
            rotation: modelControls.commonRotationCheckBox.checked ?
                          stlModel.geometry.getRotationFromAxisAndAngle(Qt.vector3d(0,0,1), modelControls.pointModelRotationSlider.value) :
                          Qt.quaternion(0,0,0,0)

            geometry: GCodeGeometry {
                id: gcodeGeometry

                onModelLoaded: {
//                    view3d.gcodeModelCenter = getModelCenter(gcodeModel)
//                    console.log(" ### view3d.gcodeModelCenter :" + view3d.gcodeModelCenter)
//                    gcodeModel.position = Qt.vector3d(0, 0, 0).minus(view3d.gcodeModelCenter)
//                    camera.lookAtModel(gcodeModel)
//                    modelControls.numSubPaths = gcodeGeometry.numSubPaths
//                    modelControls.numPointsInSubPaths = gcodeGeometry.numPointsInSubPath
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

        StlModel {
            id: stlModel
        }

        PickArea {
        }

        ModelControls {
            id: modelControls
            anchors {
                left: parent.left
                top: parent.top
                bottom: parent.bottom
                topMargin: 10
                bottomMargin: 10
                leftMargin: 10
            }

            numSubPathsSlider.onValueChanged: {
                gcodeGeometry.numSubPaths = numSubPathsSlider.value
            }

            numPointsInSubPathSlider.onValueChanged: {
                gcodeGeometry.numPointsInSubPath = numPointsInSubPathSlider.value
            }

            numPathPointsUsedSlider.onValueChanged: {
                gcodeGeometry.numPathPointsUsed = numPathPointsUsedSlider.value
            }
        }
    }


    function getModelCenter(model) {
        return model.geometry.minBounds.plus(model.geometry.maxBounds).times(0.5)
    }
}