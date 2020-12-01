import QtQuick3D 1.15
import customgeometry 1.0

Model {
    id: triangleModel
    property alias geometry: triangleModel.geometry
    property bool isPicked: false
    objectName: "STL geometry"
    pickable: true
    rotation: modelControls.commonRotationCheckBox.checked ?
                  triangleModel.geometry.getRotationFromAxisAndAngle(Qt.vector3d(0,0,1), modelControls.pointModelRotationSlider.value) :
                  Qt.quaternion(0,0,0,0)


    geometry: ExampleTriangleGeometry {
        warp: modelControls.triangleModelWarpSlider.value

        onBoundsChanged: {
            var modelCenter = triangleModel.geometry.minBounds.plus(triangleModel.geometry.maxBounds).times(0.5)
            console.log(" model bounds min: " + triangleModel.geometry.minBounds)
            console.log(" model bounds max: " + triangleModel.geometry.maxBounds)
            console.log(" modelCenter : " + modelCenter)
            //                    console.log(" triangleModel.geometry. : " + triangleModel.bounds)
        }

        //                onModelLoaded: {
        //                    console.log(" modelLoaded")
        //                    camera.lookAtModel()
        //                }
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
