import QtQuick3D 1.15
import customgeometry 1.0

Model {
    id: rootModel
    property alias inputFile: geometry.inputFile
    property bool isPicked: false
    objectName: "STL geometry"
    pickable: true
    rotation: Qt.quaternion(0,0,0,0)//modelControls.commonRotationCheckBox.checked ?
                                    // rootModel.geometry.getRotationFromAxisAndAngle(Qt.vector3d(0,0,1), modelControls.pointModelRotationSlider.value) :
                                    // Qt.quaternion(0,0,0,0)


    geometry: TriangleGeometry {
        id: geometry
        warp: 0//modelControls.triangleModelWarpSlider.value

        onBoundsChanged: {
            var modelCenter = rootModel.geometry.minBounds.plus(rootModel.geometry.maxBounds).times(0.5)
            console.log(" model bounds min: " + rootModel.geometry.minBounds)
            console.log(" model bounds max: " + rootModel.geometry.maxBounds)
            console.log(" modelCenter : " + modelCenter)
        }
    }

    materials: [
        DefaultMaterial {
            cullMode: DefaultMaterial.NoCulling
            diffuseColor: rootModel.isPicked ? "cyan" : "lightgrey"
            specularAmount: 0.5
        }
    ]

    function exportModelToSTL(path) {
        geometry.exportModelToSTL(path)
    }

    function snapToFloor()
    {
        if (typeof snapToFloor.wasPressed == 'undefined') {
            snapToFloor.wasPressed = false;

            console.log("snapToFloor pressed")
            move(Qt.vector3d(0,0, -rootModel.geometry.minBounds.z))
        }
        else {
            console.log("snapToFloor already pressed")
        }
    }

    function move(offset) {
        rootModel.position = rootModel.position.plus(offset)
    }
}
