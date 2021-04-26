import QtQml 2.3
import "HelperFunctions.js" as QmlHelpers

QtObject {
    id: modelGroupDrag
    property var connectedPickArea: undefined

//    enum DragType {
//        Position,
//        Rotation
//    }

//    property int dragType: PickArea.DragType.Position
    property bool isActive: false
    property vector3d startDragPos
    property var pickedObject
    property bool wasDragStartedWithPick
    property var objects
    property var objectStartPositions
    property var objectStartRotations

    function start(mouse) {
        var pickData = connectedPickArea.getPick(mouse)
        if (!pickData.object)
        {
            if (mouse.button !== Qt.RightButton)
                return

            // In case of right drag, we will rotate.
//            startDragPos = helper3D
        }

//        dragType = mouse.PickArea.DragType.Position
        startDragPos = pickData.coords
        pickedObject = pickData.object

        if (pickedObject) {
            wasDragStartedWithPick = pickedObject.isPicked
            pickedObject.isPicked = true
        }

        objects = QmlHelpers.getSelected(stlObjectsRepeater)
        objectStartPositions = QmlHelpers.getPositions(objects)
        objectStartRotations = QmlHelpers.getRotations(objects)

        isActive = true
    }

    function finish() {
        if (!isActive)
            return
        isActive = false

        if (pickedObject) {
            pickedObject.isPicked = wasDragStartedWithPick
        }
    }

    function moveModels(dragVector) {
        for (var i=0; i<objects.length; i++)
        {
            objects[i].position = objectStartPositions[i].plus(dragVector)
        }
    }

    function rotateModels(dragStart, dragVector) {
        console.assert(objects.length === objectStartPositions.length, "Operating on lists of different sizes.");

        var centerPosition = Qt.vector3d(0,0,0)
        for (var i=0; i<objectStartPositions.length; i++)
        {
            centerPosition = centerPosition.plus(objectStartPositions[i])
        }
        centerPosition = centerPosition.times(1/objects.length)
        centerPosition.z = dragStart.z

        var startDir = dragStart.minus(centerPosition)
        var endDir = startDir.plus(dragVector)
        var additionalRotation = helper3D.getRotationFromAxes(startDir, endDir)

        for (var i=0; i<objects.length; i++) {
            objects[i].rotation = helper3D.getRotationFromQuaternions(objectStartRotations[i], additionalRotation)
            var initialObjectRadiusFromCenter = objectStartPositions[i].minus(centerPosition)
            var rotatedObjectRadius = helper3D.getRotatedVector(additionalRotation, initialObjectRadiusFromCenter)
            objects[i].position = centerPosition.plus(rotatedObjectRadius)
        }
    }

    function dragPositionChanged(origin, ray) {
        var planeIntersection =
                helper3D.getLinePlaneIntersection(origin, ray, Qt.vector3d(0,0,1), modelGroupDrag.startDragPos)

        var dragVector = planeIntersection.intersection.minus(modelGroupDrag.startDragPos)

        if (pickArea.pressedButtons & Qt.LeftButton) {
            modelGroupDrag.moveModels(dragVector)
        }
        if (pickArea.pressedButtons & Qt.RightButton) {
            modelGroupDrag.rotateModels(modelGroupDrag.startDragPos, dragVector)
        }
    }
}
