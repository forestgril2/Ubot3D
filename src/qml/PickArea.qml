import QtQuick 2.15
import QtQml 2.15

MouseArea {
    property alias isDragActive: modelGroupDrag.isActive

    anchors.fill: view3d

    enum DragType {
        Position,
        Rotation
    }

    QtObject {
        id: modelGroupDrag

        property int dragType: PickArea.DragType.Position
        property bool isActive: false
        property vector3d startPickPos
        property vector3d startModelPos
        property var pickedModel
        property bool wasPickedModelPickedBefore
        property var models
        property var modelsStartPositions

        function start(pickData) {
            dragType = PickArea.DragType.Position
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
            console.log(" ### :" + "moveModels")
            for (var i=0; i<models.length; i++)
            {
                console.log(" ### i:" + i)
                models[i].position = modelsStartPositions[i].plus(dragVector)
            }
        }
    }

    onDoubleClicked: {
        console.log(" ### onDoubleClicked")
        console.log(" ### gcodeModel.position before double click:" + gcodeModel.position)
        gcodeModel.position = Qt.vector3d(0,0,0).minus(getModelCenter(gcodeModel))
        console.log(" ### gcodeModel.position after:" + gcodeModel.position )

        camera.lookAtModel(gcodeModel)
    }

    function getOriginAndRay(x,y) {
        var origin = camera.position
        var pointAtScreen = Qt.vector3d(x/view3d.width, y/view3d.height, 0)
        var pointSceneTo = camera.mapFromViewport(pointAtScreen)
        var ray = pointSceneTo.minus(origin).normalized()
        return {origin, ray}
    }

    Timer {
        id: doubleClickTimer
        interval: 200
        repeat: false
    }

    onClicked: {
        if (doubleClickTimer.running) {
            doubleClickTimer.stop()
            doubleClicked(mouse)
            return
        }

        doubleClickTimer.start()

        var closestPick = getClosestPick(mouse)
        if (!closestPick)
            return

        closestPick.model.isPicked = !closestPick.model.isPicked
    }

    onPressed: {
        modelGroupDrag.start(getClosestPick(mouse))
    }

    onReleased: {
        modelGroupDrag.finish()
    }

    onPositionChanged: {
        if(isDragActive) {
            var originAndRay = getOriginAndRay(mouse.x, mouse.y)
            var planeIntersection = helper3D.getLinePlaneIntersection(originAndRay.ray,
                                                                      originAndRay.origin,
                                                                      Qt.vector3d(0,0,1),
                                                                      modelGroupDrag.startPickPos)

            console.log(" ### modelGroupDrag.startPickPos:" + modelGroupDrag.startPickPos)

            var dragVector = planeIntersection.intersection.minus(modelGroupDrag.startPickPos)

            console.log(" ### dragVector :" + dragVector )

            modelGroupDrag.moveModels(dragVector)
        }
    }

    function getClosestPick(mouse) {
        var originAndRay = getOriginAndRay(mouse.x, mouse.y)
        var coords = undefined
        var dist = Infinity
        var model = undefined

        for (var i = 0; i < stlModels.count; i++)
        {// Find out, if we are pressing an object.
            var stlModel = stlModels.objectAt(i)

            var modelIntersection = stlModel.geometry.getPick(originAndRay.origin, originAndRay.ray, stlModel.sceneTransform)
            if (!modelIntersection.isHit)
                continue

            var distToHit = modelIntersection.intersection.minus(originAndRay.origin).length()
            if (distToHit >= dist)
                continue

            model = stlModel
            dist = distToHit
            coords = modelIntersection.intersection
        }

        return {model: model, coords: coords}
    }
}
