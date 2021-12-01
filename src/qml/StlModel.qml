import QtQuick3D
import customgeometry 1.0

Model {
    id: stlModel
    // These properties rule the underlying geometry properties
    property string inputFile
    property bool isSupportGenerated
    property bool areRaftsGenerated
    property double raftOffset
    property double raftHeight

    property bool isPicked: false
    property bool isSnappedToFloor: false
    property vector3d modelCenter

    property double initialZ: 0


    position: Qt.vector3d(0,0,0)
    rotation: Qt.quaternion(1,0,0,0)
    pickable: true

    geometry: TriangleGeometry {
        id: geometry
        inputFile: stlModel.inputFile
        isSupportGenerated: stlModel.isSupportGenerated
        areRaftsGenerated: stlModel.areRaftsGenerated
        sceneTransform: stlModel.sceneTransform
        raftOffset: stlModel.raftOffset
        raftHeight: stlModel.raftHeight

        objectName: "STL geometry"

        onBoundsChanged: {
            stlModel.modelCenter = minBounds.plus(maxBounds).times(0.5)
        }

        onMinZBoundsChanged: {
            if (isMainGeometry) {
                snapToFloor()
            }
        }

        onModelLoaded: {
            stlModel.modelCenter = minBounds.plus(maxBounds).times(0.5)
            if (isMainGeometry) {
                stlModel.snapToFloor()
            }
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
        console.log(" ### onRaftHeightChanged:", geometry.raftHeight)
        var currentPosition = Qt.vector3d(position.x, position.y, position.z)
        z = initialZ + raftHeight
    }

    onZChanged: {
        console.log(" ### zChanged to:", z)
    }

    function snapToFloor()
    {
        console.log(" ### SNAP!")
        if (isSnappedToFloor)
            return

        console.log(" ### SNAPPING! where stlModel.geometry.minBounds.z = ", stlModel.geometry.minBounds.z, ",position =", position)
        move(Qt.vector3d(0,0, -stlModel.geometry.minBounds.z))
        console.log(" ### AFTER SNAPPING! position =", position)
        isSnappedToFloor = true
        initialZ = stlModel.position.z

    }

    function move(offset) {
        var newPosition = stlModel.position.plus(offset);
        stlModel.position = newPosition
    }
}
