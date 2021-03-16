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
    property vector3d startPickPos
    property vector3d startDragPos
    property var pickedObject
    property bool wasDragStartedWithPick
    property var objects
    property var objectStartPositions

    function start(mouse) {
        var pickData = connectedPickArea.getPick(mouse)
        if (!pickData.object)
        {
            if (mouse.button !== Qt.RightButton)
                return

            // In case of right drag, we will rotate.
//            startPickPos = helper3D
        }

//        dragType = mouse.PickArea.DragType.Position
        startPickPos = pickData.coords
        pickedObject = pickData.object
        startDragPos = startPickPos

        if (pickedObject) {
            wasDragStartedWithPick = pickedObject.isPicked
            pickedObject.isPicked = true
        }

        objects = QmlHelpers.getSelected(stlObjects)
        objectStartPositions = QmlHelpers.getPositions(objects)

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
        var centerPosition = Qt.vector3d(0,0,0)
        for (var i=0; i<objects.length; i++)
        {
            centerPosition = centerPosition.plus(objects[i].position)
        }
        centerPosition = centerPosition.times(1/objects.length)

        console.log(" ### rotating!:" + "")
    }

    function dragPositionChanged(origin, ray) {
        var planeIntersection =
                helper3D.getLinePlaneIntersection(origin, ray, Qt.vector3d(0,0,1), modelGroupDrag.startPickPos)

        var dragVector = planeIntersection.intersection.minus(modelGroupDrag.startPickPos)

        if (pickArea.pressedButtons & Qt.LeftButton) {
            modelGroupDrag.moveModels(dragVector)
        }
        if (pickArea.pressedButtons & Qt.RightButton) {
            modelGroupDrag.rotateModels(modelGroupDrag.startPickPos, dragVector)
        }
    }
}
