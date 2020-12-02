import Qt.labs.platform 1.1
import QtQuick3D.Helpers 1.15
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick3D 1.15
import QtQuick.Controls 2.15
import customgeometry 1.0

Window {
    id: window
    width: 1280
    height: 720
    visible: true
    color: "#848895"

    FileDialog {
        id: fileDialog
        onAccepted:
        {
            console.log(fileDialog.currentFile)
            var fullSystemFilePath = fileDialog.currentFile.toString()
            console.log(fullSystemFilePath)
            var pos = fullSystemFilePath.search("file:///");
            console.log(pos)
            fullSystemFilePath = fullSystemFilePath.substring(pos+8)
            console.log(fullSystemFilePath)
            stlModel.geometry.inputFile = fullSystemFilePath
        }
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
            camera.lookAtModel(gcodeModel)
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
            position: initDistToModel


            function lookAtModel(model)
            {
                var modelCenter = getModelCenter(model)
                console.log(" ### lookAtModel: " + model.objectName + ", modelCenter: " + modelCenter);
                console.log(" ### " + gcodeModel.objectName + " modelLoaded with paths: " + gcodeGeometry.numSubpaths + ", max points in subpath: " + gcodeGeometry.numPointsInSubpath)
                var direction = modelCenter.minus(camera.position)
                var upDirection = Qt.vector3d(0,0,1)
                var lookAtModelCenterRotation = stlModel.geometry.getRotationFromDirection(direction, upDirection)
                camera.rotation = lookAtModelCenterRotation
            }
        }

        DirectionalLight {
            position: Qt.vector3d(-500, 500, -100)
            color: Qt.rgba(0.4, 0.2, 0.6, 1.0)
            ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0)
        }

        PointLight {
            position: Qt.vector3d(0, 0, 100)
            color: Qt.rgba(0.1, 1.0, 0.1, 1.0)
            ambientColor: Qt.rgba(0.2, 0.2, 0.2, 1.0)
        }


        Model {
            scale: Qt.vector3d(100, 100, 100)
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
            position: Qt.vector3d(0, 0, 0).minus(view3d.gcodeModelCenter)
            objectName: "gCode geometry"
            pickable: true
            rotation: modelControls.commonRotationCheckBox.checked ?
                          stlModel.geometry.getRotationFromAxisAndAngle(Qt.vector3d(0,0,1), modelControls.pointModelRotationSlider.value) :
                          Qt.quaternion(0,0,0,0)

            geometry: GCodeGeometry {
                id: gcodeGeometry

                onModelLoaded: {
                    camera.lookAtModel(gcodeModel)
                    view3d.gcodeModelCenter = getModelCenter(gcodeModel)
                }
            }
            materials: [
                DefaultMaterial {
                    cullMode: DefaultMaterial.NoCulling
                    diffuseColor: "red"
                    specularAmount: 0.5
                }
            ]
        }

        StlModel {
            id: stlModel
        }

        PickArea {

        }
    }



    ModelControls {
        id: modelControls
    }

    function getModelCenter(model) {
        return model.geometry.minBounds.plus(model.geometry.maxBounds).times(0.5)
    }
}
