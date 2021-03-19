import QtQuick3D 1.15
import customgeometry 1.0

Model {
    id: stlModel
    property alias inputFile: geometry.inputFile
    property bool isPicked: false
    property bool isSnappedToFloor: false
    objectName: "STL geometry"
    pickable: true
    rotation: Qt.quaternion(0,0,0,0)//modelControls.commonRotationCheckBox.checked ?
                                    // rootModel.geometry.getRotationFromAxisAndAngle(Qt.vector3d(0,0,1), modelControls.pointModelRotationSlider.value) :
                                    // Qt.quaternion(0,0,0,0)


    geometry: TriangleGeometry {
        id: geometry

        onBoundsChanged: {
//            var modelCenter = rootModel.geometry.minBounds.plus(rootModel.geometry.maxBounds).times(0.5)
//            console.log(" model bounds min: " + stlModel.geometry.minBounds)
//            console.log(" model bounds max: " + stlModel.geometry.maxBounds)
//            console.log(" modelCenter : " + modelCenter)

            //TODO: Have to think about resnapping, after minBounds.z change.
//            isSnappedToFloor = false;
            snapToFloor()
        }
    }

    materials: [
        DefaultMaterial {
            cullMode: DefaultMaterial.NoCulling
            diffuseColor: stlModel.isPicked ? "cyan" : "lightgrey"
            specularAmount: 0.5
        }
    ]

    function snapToFloor()
    {
//        console.log(" ### snapToFloor:" + "")
        if (isSnappedToFloor)
            return

        move(Qt.vector3d(0,0, -stlModel.geometry.minBounds.z))
        isSnappedToFloor = true

    }

    function move(offset) {
        stlModel.position = stlModel.position.plus(offset)
    }
}
