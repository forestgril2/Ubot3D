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
            triangleModel.geometry.inputFile = fullSystemFilePath
        }
    }

    MainMenu {

    }

    Row {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 20
        Label {
            id: pickName
            color: "#222840"
            font.pointSize: 14
            text: "Last Pick: None"
        }
        Label {
            id: pickPosition
            color: "#222840"
            font.pointSize: 14
            text: "Screen Position: (0, 0)"
        }
        Label {
            id: uvPosition
            color: "#222840"
            font.pointSize: 14
            text: "UV Position: (0.00, 0.00)"
        }
        Label {
            id: distance
            color: "#222840"
            font.pointSize: 14
            text: "Distance: 0.00"
        }
        Label {
            id: scenePosition
            color: "#222840"
            font.pointSize: 14
            text: "World Position: (0.00, 0.00)"
        }
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
            isMouseDragInverted: mouseMoveDragInvertCheckBox.checked
        }

        PerspectiveCamera {
            id: camera
            fieldOfView: 45
            clipNear: 0.1
            clipFar: 1000.0
            x: 0
            y: -75
            z: 50

            function lookAtModel()
            {
                var modelCenter = triangleModel.geometry.minBounds.plus(triangleModel.geometry.maxBounds).times(0.5)
                var direction = modelCenter.minus(camera.position)
                var upDirection = Qt.vector3d(0,0,1)
                var lookAtModelCenterRotation = triangleModel.geometry.getRotationFromDirection(direction, upDirection)
                console.log("camera.rotation: " + lookAtModelCenterRotation);
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

//        Node091_W_Aya_100K {
//            scale: Qt.vector3d(0.1, 0.1, 0.1)
//        }

        Model {
            scale: Qt.vector3d(100, 100, 100)
            geometry: GridGeometry {
                id: grid
                horizontalLines: 20
                verticalLines: 20
            }
            materials: [
                DefaultMaterial {
//                    lineWidth: sliderLineWidth.value
                }
            ]
        }

        Model {
            id: triangleModel
            property alias geometry: triangleModel.geometry
            property bool isPicked: false
            objectName: "STL geometry"
            pickable: true
            rotation: commonRotationCheckBox.checked ?
                          triangleModel.geometry.getRotationFromAxisAndAngle(Qt.vector3d(0,0,1), pointModelRotationSlider.value) :
                          Qt.quaternion(0,0,0,0)

//            ObjectPicker {
//                id: picker

//                onClicked: {
//                    console.log("clicked")
//                }

//            }
//            source: "#Cube"
//            source: "node091_W_Aya_100K_01.mesh"


            geometry: ExampleTriangleGeometry {
                warp: triangleModelWarpSlider.value

                onBoundsChanged: {
                    var modelCenter = triangleModel.geometry.minBounds.plus(triangleModel.geometry.maxBounds).times(0.5)
                    console.log(" model bounds min: " + triangleModel.geometry.minBounds)
                    console.log(" model bounds max: " + triangleModel.geometry.maxBounds)
                    console.log(" modelCenter : " + modelCenter)
//                    console.log(" triangleModel.geometry. : " + triangleModel.bounds)
                }

                onModelLoaded: {
                    console.log(" modelLoaded")
                    camera.lookAtModel()
                }
            }

            materials: [
                DefaultMaterial {
//                    Texture {
//                        id: baseColorMap
//                        source: "Ikona.png"
//                    }
                    cullMode: DefaultMaterial.NoCulling
                    diffuseColor: triangleModel.geometry.isPicked ? "lightgreen" : "lightgrey"
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

//        PointModel {
//            id: pointModel
//        }
    }


    MouseArea {
        anchors.fill: view3d
        onDoubleClicked: {
            camera.lookAtModel()
            controller.focus = true
        }

        function getOriginAndRay(x,y) {
            var origin = camera.position
            var pointAtScreen = Qt.vector3d(x/view3d.width, y/view3d.height, 0)
            var pointSceneTo = camera.mapFromViewport(pointAtScreen)
            var ray = pointSceneTo.minus(origin).normalized()
            return {origin, ray}
        }

        onClicked: {
            var originAndRay = getOriginAndRay(mouse.x, mouse.y)
            triangleModel.geometry.getPick(originAndRay.origin, originAndRay.ray, triangleModel.sceneTransform)
//            console.log(" getOriginAndRay: " + originAndRay.origin + "," + originAndRay.ray)

//            console.log(" triangleModel.bounds : " + triangleModel.bounds)
            // Get screen coordinates of the click
            pickPosition.text = "Screen Position: (" + mouse.x + ", " + mouse.y + ")"
            var result = view3d.pick(mouse.x, mouse.y);
            if (result.objectHit) {
                var pickedObject = result.objectHit;
                // Toggle the isPicked property for the model
                pickedObject.isPicked = !pickedObject.isPicked;
                // Get picked model name
                pickName.text = "Last Pick: " + pickedObject.objectName;
                // Get other pick specifics
                uvPosition.text = "UV Position: ("
                        + result.uvPosition.x.toFixed(2) + ", "
                        + result.uvPosition.y.toFixed(2) + ")";
                distance.text = "Distance: " + result.distance.toFixed(2);
                scenePosition.text = "World Position: ("
                        + result.scenePosition.x.toFixed(2) + ", "
                        + result.scenePosition.y.toFixed(2) + ")";
            }
            else {
                pickName.text = "Last Pick: None";
            }
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

            CheckBox {
                id: mouseMoveDragInvertCheckBox
                text: qsTr("Invert mouse drag")
                checkState: Qt.Unchecked
            }
        }
    }
}
