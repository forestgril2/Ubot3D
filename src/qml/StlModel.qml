import QtQuick3D 1.15
import customgeometry 1.0

Model {
    id: stlModel
    property alias inputFile: geometry.inputFile
    property alias isSupportGenerated: geometry.isSupportGenerated
    property alias areRaftsGenerated: geometry.areRaftsGenerated
    property alias raftOffset: geometry.raftOffset
    property double raftHeight: 0

    property bool isPicked: false
    property bool isSnappedToFloor: false
    property vector3d modelCenter

    property double initialZ: 0

    objectName: "STL geometry"
    pickable: true
    rotation: Qt.quaternion(0,0,0,0)

    z: initialZ + raftHeight

    geometry: TriangleGeometry {
        id: geometry
        sceneTransform: stlModel.sceneTransform

        onBoundsChanged: {
            stlModel.modelCenter = minBounds.plus(maxBounds).times(0.5)
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

    onRaftHeightChanged: {
        stlModel.position.z
    }

    function snapToFloor()
    {
        if (isSnappedToFloor)
            return

        move(Qt.vector3d(0,0, -stlModel.geometry.minBounds.z))
        isSnappedToFloor = true
        initialZ = stlModel.position.z

    }

    function move(offset) {
        stlModel.position = stlModel.position.plus(offset)
    }
}
