import QtQuick 2.15
import QtQml 2.15

MouseArea {
    anchors.fill: view3d
    signal modelDragged(var model, vector3d modelStartPos, vector3d pickCoords)

    property bool isDraggingModelGroup: false
    property vector3d startPickPosition
    property vector3d startModelPosition
    property var draggedModel

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

        var originAndRay = getOriginAndRay(mouse.x, mouse.y)

        var numModels = stlModels.count

        for (var i = 0; i < numModels; i++) {
            var stlModel = stlModels.objectAt(i);

            var modelIntersection = stlModel.geometry.getPick(originAndRay.origin, originAndRay.ray, stlModel.sceneTransform)
            console.log(" ### modelIntersection.intersection:" + modelIntersection.intersection)
            console.log(" ### modelIntersection.isHit:" + modelIntersection.isHit)

            var planeIntersection = helper3D.calculator.getLinePlaneIntersection(originAndRay.ray,
                                                                                 originAndRay.origin,
                                                                                 Qt.vector3d(0,0,1),
                                                                                 modelIntersection.intersection);

            console.log(" planeIntersection.isHit: " + planeIntersection.isHit)
            console.log(" planeIntersection.intersection: " + planeIntersection.intersection)

            //            console.log(" triangleModel.bounds : " + triangleModel.bounds)
            // Get screen coordinates of the click
            pickDebugs.pickPosition.text = "Screen Position: (" + mouse.x + ", " + mouse.y + ")"
            var result = view3d.pick(mouse.x, mouse.y);
            if (result.objectHit) {
                var pickedObject = result.objectHit;
                // Toggle the isPicked property for the model
                pickedObject.isPicked = !pickedObject.isPicked;
                // Get picked model name
                pickDebugs.pickName.text = "Last Pick: " + pickedObject.objectName;
                // Get other pick specifics
                pickDebugs.uvPosition.text = "UV Position: ("
                        + result.uvPosition.x.toFixed(2) + ", "
                        + result.uvPosition.y.toFixed(2) + ")";
                pickDebugs.distance.text = "Distance: " + result.distance.toFixed(2);
                pickDebugs.scenePosition.text = "World Position: ("
                        + result.scenePosition.x.toFixed(2) + ", "
                        + result.scenePosition.y.toFixed(2) + ")";
            }
            else {
                pickDebugs.pickName.text = "Last Pick: None";
            }
        }
    }

    onPressed: {
        var originAndRay = getOriginAndRay(mouse.x, mouse.y)
        var numModels = stlModels.count

        for (var i = 0; i < numModels; i++)
        {// Find out, if we are pressing an object.
            var stlModel = stlModels.objectAt(i);

            var modelIntersection = stlModel.geometry.getPick(originAndRay.origin, originAndRay.ray, stlModel.sceneTransform)
            if (modelIntersection.isHit)
            {// If so, remember the point of press and send modelDragged() signal with coordinates
                modelDragged(stlModel, stlModel.position, modelIntersection.intersection)
            }

            var planeIntersection = helper3D.calculator.getLinePlaneIntersection(originAndRay.ray,
                                                                                 originAndRay.origin,
                                                                                 Qt.vector3d(0,0,1),
                                                                                 modelIntersection.intersection);

        }


    }

    onPressAndHold: {

    }

    onReleased: {
        isDraggingModelGroup = false
    }

    onModelDragged: {
        isDraggingModelGroup = true
        startPickPosition = pickCoords
        startModelPosition = modelStartPos
        draggedModel = model
    }

    onPositionChanged: {
        if(isDraggingModelGroup) {

            var originAndRay = getOriginAndRay(mouse.x, mouse.y)
            var planeIntersection = helper3D.calculator.getLinePlaneIntersection(originAndRay.ray,
                                                                                 originAndRay.origin,
                                                                                 Qt.vector3d(0,0,1),
                                                                                 startPickPosition)
            draggedModel.position = startModelPosition.plus(planeIntersection.intersection.minus(startPickPosition))
        }
    }
}
