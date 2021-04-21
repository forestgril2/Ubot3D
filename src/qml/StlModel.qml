import QtQuick3D 1.15
import customgeometry 1.0

Model {
    id: stlModel
    property alias inputFile: geometry.inputFile
    property alias isSupportGenerated: geometry.isSupportGenerated

    property bool isPicked: false
    property bool isSnappedToFloor: false
    property vector3d modelCenter

    objectName: "STL geometry"
    pickable: true
    rotation: Qt.quaternion(0,0,0,0)//modelControls.commonRotationCheckBox.checked ?
                                    // rootModel.geometry.getRotationFromAxisAndAngle(Qt.vector3d(0,0,1), modelControls.pointModelRotationSlider.value) :
    // Qt.quaternion(0,0,0,0)

    geometry: TriangleGeometry {
        id: geometry

        onBoundsChanged: {
            stlModel.modelCenter = minBounds.plus(maxBounds).times(0.5)
            //TODO: Have to think about resnapping, after minBounds.z change.
//            isSnappedToFloor = false;
            snapToFloor()
        }

//        onTriangleIslandsChanged: {
//            console.log(" ### geometry.triangleIslands.size():" + geometry.triangleIslands.length)
//        }
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
