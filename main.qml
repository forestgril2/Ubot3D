//import QtQuick.Dialogs 1.3

import Qt.labs.platform 1.1

import Qt3D.Core 2.15
import QtQuick3D.Helpers 1.15

//import QtQuick.Scene3D 2.15

import Qt3D.Render 2.15
//import Qt3D.Input 2.15
import Qt3D.Extras 2.15
import QtQuick 2.15
import QtQuick.Window 2.15
//import QtQuick.Dialogs 1.2
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
            triangleModel.geometry.inputFile = fullSystemFilePath
        }
    }

    MainMenu {

    }


    View3D {
        id: view3d
        anchors.fill: parent
        camera: camera

        Component.onCompleted: {
            console.log(" ### ComponentComplete")
            camera.lookAtModel()
        }

        Ubot3DCameraWasdController {
            id: controller
            controlledObject: camera
            camera: camera
        }

        PerspectiveCamera {
            id: camera
            fieldOfView: 45
            clipNear: 0.1
            clipFar: 10000.0
            x: 0
            y: -75
            z: 50

            function lookAtModel()
            {
                var modelCenter = triangleModel.geometry.minBounds.plus(triangleModel.geometry.maxBounds).times(0.5)
                var lookAtModelCenterRotation = triangleModel.geometry.getRotation(modelCenter, camera.position)
                console.log("camera.rotation: " + lookAtModelCenterRotation);
                camera.rotation = lookAtModelCenterRotation
            }

//            onRotationChanged: {
//                console.log(" camera eulerRotation: " + eulerRotation)
//            }
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
            visible: radioGridGeom.checked
            scale: Qt.vector3d(100, 100, 100)
            geometry: GridGeometry {
                id: grid
                horizontalLines: 20
                verticalLines: 20
            }
            materials: [
                DefaultMaterial {
                    lineWidth: sliderLineWidth.value
                }
            ]
        }


        Model {
            id: triangleModel
            property alias geometry: triangleModel.geometry
            visible: radioCustGeom.checked
            scale: Qt.vector3d(1, 1, 1)
            pickable: true
            rotation: commonRotationCheckBox.checked ?
                          triangleModel.geometry.getRotation(Qt.vector3d(0,0,1), pointModelRotationSlider.value) :
                          Qt.quaternion(0,0,0,0)
            geometry: ExampleTriangleGeometry {
                normals: cbNorm.checked
                normalXY: sliderNorm.value
                uv: cbUV.checked
                uvAdjust: sliderUV.value
                warp: triangleModelWarpSlider.value

                onBoundsChanged: {
                    var modelCenter = triangleModel.geometry.minBounds.plus(triangleModel.geometry.maxBounds).times(0.5)
                    console.log(" model bounds min: " + triangleModel.geometry.minBounds)
                    console.log(" model bounds max: " + triangleModel.geometry.maxBounds)
                    console.log(" modelCenter : " + modelCenter)
                }

                onModelLoaded: {
                    console.log(" modelLoaded")
                    camera.lookAtModel()
                }
            }

            ObjectPicker {
                id: picker

                onClicked: {
                    console.log("clicked")
                }

            }


            materials: [
                DefaultMaterial {
                    Texture {
                        id: baseColorMap
                        source: "qt_logo_rect.png"
                    }
                    cullMode: DefaultMaterial.NoCulling
                    diffuseMap: cbTexture.checked ? baseColorMap : null
                    specularAmount: 0.5
                }
            ]


            function snapToFloor()
            {
                if (typeof snapToFloor.wasPressed == 'undefined') {
                        snapToFloor.wasPressed = false;

                    console.log("snapToFloor pressed")
                    triangleModel.position = triangleModel.position.minus(Qt.vector3d(0,0, triangleModel.geometry.minBounds.z))
                }
                else {
                    console.log("snapToFloor already pressed")
                }
            }
        }

        Model {
            id: pointModel
            property alias geometry: pointModel.geometry
            visible: radioPointGeom.checked
            scale: Qt.vector3d(1, 1, 1)
            rotation: triangleModel.geometry.getRotation(Qt.vector3d(0,0,1), pointModelRotationSlider.value)
            geometry: ExamplePointGeometry {}
            position: Qt.vector3d(pointModelWarpSlider.value, 0, 0)
            materials: [
                DefaultMaterial {
                    lighting: DefaultMaterial.NoLighting
                    cullMode: DefaultMaterial.NoCulling
                    diffuseColor: "yellow"
//                    pointSize: sliderPointSize.value
                }
            ]

            Connections {
                target: triangleModel.geometry
                function onModelLoaded() {
                    pointModel.geometry.updateData()
                    pointModel.geometry.update()
                }
            }
        }
    }


    MouseArea {
        anchors.fill: parent
        onDoubleClicked: {
            camera.lookAtModel()
            controller.focus = true
        }
    }

    Row {
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
            id: pointModelWarpSlider
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
                    triangleModel.snapToFloor()
                }

            }

            CheckBox {
                id: commonRotationCheckBox
                text: qsTr("Rotate together")
                checkState: Qt.Unchecked
            }
        }
    }
}
