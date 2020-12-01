import QtQuick 2.15

MouseArea {
    anchors.fill: view3d
    onDoubleClicked: {
        gcodeModel.position = Qt.vector3d(0,0,0).minus(getModelCenter(gcodeModel))

        camera.lookAtModel(gcodeModel)
        controller.focus = true
    }

    function getOriginAndRay(x,y) {
        var origin = camera.position
        var pointAtScreen = Qt.vector3d(x/view3d.width, y/view3d.height, 0)
        var pointSceneTo = camera.mapFromViewport(pointAtScreen)
        var ray = pointSceneTo.minus(origin).normalized()
        return {origin, ray}
    }

    onClicked: {
        var originAndRay = getOriginAndRay(mouse.x, mouse.y)
        stlModel.geometry.getPick(originAndRay.origin, originAndRay.ray, stlModel.sceneTransform)
        //            console.log(" getOriginAndRay: " + originAndRay.origin + "," + originAndRay.ray)

        //            console.log(" triangleModel.bounds : " + triangleModel.bounds)
        // Get screen coordinates of the click
        pickDebugs.pickPosition.text = "Screen Position: (" + mouse.x + ", " + mouse.y + ")"
        var result = view3d.pick(mouse.x, mouse.y);
        if (result.objectHit) {
            var pickedObject = result.objectHit;
            // Toggle the isPicked property for the model
            pickedObject.isPicked = !pickedObject.isPicked;
            // Get picked model name
            pickName.text = "Last Pick: " + pickedObject.objectName;
            // Get other pick specifics
            uvPosition.text = "UV Position: ("
                    + result.uvPosition.x.toFixed(2) + ", "
                    + result.uvPosition.y.toFixed(2) + ")";
            distance.text = "Distance: " + result.distance.toFixed(2);
            scenePosition.text = "World Position: ("
                    + result.scenePosition.x.toFixed(2) + ", "
                    + result.scenePosition.y.toFixed(2) + ")";
        }
        else {
            pickName.text = "Last Pick: None";
        }
    }
}
