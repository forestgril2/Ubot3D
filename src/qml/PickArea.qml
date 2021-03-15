import QtQuick 2.15
import QtQml 2.15


MouseArea {
    id: pickArea
    property bool isNextMouseClickDisabled: false
    property var postponedOnClickedActionData: undefined
    property var camera

    anchors.fill: view3d
    onDoubleClicked: {
        var model = getPickedModel()
        if (model) {
            camera.lookAt(getModelCenter(model))
        }
        else {
            camera.lookAt(sceneCenter)
        }        
        camera.setUpPlane(Qt.vector3d(0,0,1))
    }

    Timer {
        id: doubleClickTimer
        interval: 200
        repeat: false

        onTriggered: {
            if (postponedOnClickedActionData) {
                onClickedAction(postponedOnClickedActionData)
                postponedOnClickedActionData = undefined
            }
        }
    }

    onPressed: {
        if (doubleClickCountdown(mouse))
            return
    }

    onClicked: {
        if (doubleClickTimer.running) {
            postponedOnClickedActionData = mouse
            return
        }

        onClickedAction(mouse)
    }

    function onClickedAction(mouse) {
        var closestPick = getClosestPick(mouse)
        if (!closestPick.model && !doubleClickTimer.running)
        {
            stlModels.deselectAll()
            return
        }

        if (isNextMouseClickDisabled) {
            isNextMouseClickDisabled = false
            return
        }

        closestPick.model.isPicked = !closestPick.model.isPicked
    }

    function doubleClickCountdown(mouse) {
        if (doubleClickTimer.running) {
            doubleClickTimer.stop()
            postponedOnClickedActionData = false
            doubleClicked(mouse)
            return true
        }
        doubleClickTimer.start()
        return false
    }

    function getPickedModel() {
        var pickedModels = []
        for (var i=0; i<stlModels.count; i++)
        {// Find out, which objects are selected
            var stlModel = stlModels.objectAt(i)
            if (stlModel.isPicked) {
                pickedModels.push(stlModel)
            }
        }
        if (pickedModels.length != 1)
            return undefined

        return pickedModels[0]
    }

    function getOriginAndRay(x,y) {
        var origin = camera.position
        var pointAtScreen = Qt.vector3d(x/view3d.width, y/view3d.height, 0)
        var pointSceneTo = camera.mapFromViewport(pointAtScreen)
        var ray = pointSceneTo.minus(origin).normalized()
        return {origin, ray}
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
