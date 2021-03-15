import QtQml 2.3

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
    property vector3d startModelPos
    property var pickedModel
    property bool wasPickedModelPickedBefore
    property var models
    property var modelsStartPositions

    function start(mouse) {
        var pickData = connectedPickArea.getClosestPick(mouse)
        if (!pickData.model)
            return

//        dragType = mouse.PickArea.DragType.Position
        startPickPos = pickData.coords
        pickedModel = pickData.model
        startModelPos = pickedModel.position
        wasPickedModelPickedBefore = pickedModel.isPicked

        var pickedModels = []
        var pickedModelsPositions = []
        pickedModel.isPicked = true
        for (var i=0; i<stlModels.count; i++)
        {// Find out, which objects are selected
            var stlModel = stlModels.objectAt(i)
            if (stlModel.isPicked) {
                pickedModels.push(stlModel)
                var pickedModelPosition = Qt.vector3d(stlModel.position.x, stlModel.position.y, stlModel.position.z)
                pickedModelsPositions.push(pickedModelPosition)
            }
        }

        models = pickedModels
        modelsStartPositions = pickedModelsPositions
        isActive = true
    }

    function finish() {
        if (!isActive)
            return
        isActive = false

        pickedModel.isPicked = wasPickedModelPickedBefore
    }

    function moveModels(dragVector) {
        for (var i=0; i<models.length; i++)
        {
            models[i].position = modelsStartPositions[i].plus(dragVector)
        }
    }

    function dragPositionChanged(originAndRay) {
        var planeIntersection = helper3D.getLinePlaneIntersection(originAndRay.ray,
                                                                  originAndRay.origin,
                                                                  Qt.vector3d(0,0,1),
                                                                  modelGroupDrag.startPickPos)

        var dragVector = planeIntersection.intersection.minus(modelGroupDrag.startPickPos)

        if (pickArea.pressedButtons & Qt.LeftButton) {
            modelGroupDrag.moveModels(dragVector)
        }
        if (pickArea.pressedButtons & Qt.RightButton) {
            modelGroupDrag.rotateModels(dragVector)
        }
    }
}
