/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Quick 3D.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick
import QtQuick3D
import customgeometry 1.0

Item {
    id: root
    property Node controlledObject: undefined
    property var camera: undefined
    property bool isMouseDragInverted: false

    property real speed: 0.2
    property real shiftSpeed: 5

    property real forwardSpeed: 1
    property real rotationSpeed: 1
    property real backSpeed: 1
    property real rightSpeed: 2
    property real leftSpeed: 2
    property real upSpeed: 2
    property real downSpeed: 2
    property real xSpeed: 0.1
    property real ySpeed: 0.1

    property bool xInvert: false
    property bool yInvert: true

    property bool mouseEnabled: true
    property bool keysEnabled: true

    readonly property bool inputsNeedProcessing: status.moveForward   | status.moveBack   |
                                                 status.moveLeft      | status.moveRight  |
                                                 status.moveUp        | status.moveDown   |
                                                 status.rotateForward | status.rotateBack |
                                                 status.useMouse

    property alias acceptedButtons: dragHandler.acceptedButtons

    implicitWidth: parent.width
    implicitHeight: parent.height
    focus: keysEnabled

    DragHandler {
        id: dragHandler
        target: null
        enabled: mouseEnabled
        onCentroidChanged: {
            mouseMoved(Qt.vector2d(centroid.position.x, centroid.position.y));
        }

        onActiveChanged: {
            if (active)
                mousePressed(Qt.vector2d(centroid.position.x, centroid.position.y));
            else
                mouseReleased(Qt.vector2d(centroid.position.x, centroid.position.y));
        }
    }

    Keys.onPressed: (event) => {
                        if (keysEnabled) handleKeyPress(event)
                    }
    Keys.onReleased: (event) => {
                         if (keysEnabled) handleKeyRelease(event)
                     }
    function mousePressed(newPos) {
        status.currentPos = newPos
        status.lastPos = newPos
        status.useMouse = true;
    }

    function mouseReleased(newPos) {
        status.useMouse = false;
    }

    function mouseMoved(newPos) {
        status.currentPos = newPos;
    }

    function forwardPressed() { 
        if (!status.controlDown)
        {
            status.moveForward = true
            status.rotateForward = false
        }
        else
        {
            status.rotateForward = true
            status.moveForward = false
        }
        status.rotateBack = false
        status.moveBack = false
    }

    function forwardReleased() {
        status.moveForward = false
        status.rotateForward = false
    }

    function backPressed() {
        if (!status.controlDown)
        {
            status.moveBack = true
            status.moveForward = false
        }
        else
        {
            status.rotateBack = true
            status.moveBack = false
        }
        status.moveForward = false
        status.rotateForward = false
    }

    function backReleased() {
        status.moveBack = false
        status.rotateBack = false
    }

    function rightPressed() {
        status.moveRight = true
        status.moveLeft = false
    }

    function rightReleased() {
        status.moveRight = false
    }

    function leftPressed() {
        status.moveLeft = true
        status.moveRight = false
    }

    function leftReleased() {
        status.moveLeft = false
    }

    function upPressed() {
        status.moveUp = true
        status.moveDown = false
    }

    function upReleased() {
        status.moveUp = false
    }

    function downPressed() {
        status.moveDown = true
        status.moveUp = false
    }

    function downReleased() {
        status.moveDown = false
    }

    function shiftPressed() {
        status.shiftDown = true
    }

    function shiftReleased() {
        status.shiftDown = false
    }

    function controlPressed() {
        status.controlDown = true
    }

    function controlReleased() {
        status.controlDown = false
    }

    function handleKeyPress(event)
    {
        switch (event.key) {
        case Qt.Key_W:
        case Qt.Key_Up:
            focus = true
            forwardPressed();
            break;
        case Qt.Key_S:
        case Qt.Key_Down:
            focus = true
            backPressed();
            break;
        case Qt.Key_A:
        case Qt.Key_Left:
            focus = true
            leftPressed();
            break;
        case Qt.Key_D:
        case Qt.Key_Right:
            focus = true
            rightPressed();
            break;
        case Qt.Key_R:
        case Qt.Key_PageUp:
            focus = true
            upPressed();
            break;
        case Qt.Key_F:
        case Qt.Key_PageDown:
            focus = true
            downPressed();
            break;
        case Qt.Key_Shift:
            focus = true
            shiftPressed();
            break;
        case Qt.Key_Control:
            focus = true
            controlPressed();
            break;
        }
    }

    function handleKeyRelease(event)
    {
        switch (event.key) {
        case Qt.Key_W:
        case Qt.Key_Up:
            forwardReleased();
            break;
        case Qt.Key_S:
        case Qt.Key_Down:
            backReleased();
            break;
        case Qt.Key_A:
        case Qt.Key_Left:
            leftReleased();
            break;
        case Qt.Key_D:
        case Qt.Key_Right:
            rightReleased();
            break;
        case Qt.Key_R:
        case Qt.Key_PageUp:
            upReleased();
            break;
        case Qt.Key_F:
        case Qt.Key_PageDown:
            downReleased();
            break;
        case Qt.Key_Shift:
            shiftReleased();
            break;
        case Qt.Key_Control:
            controlReleased();
            break;
        }
    }

    Timer {
        id: updateTimer
        interval: 16
        repeat: true
        running: root.inputsNeedProcessing
        onTriggered: {
            processInputs();
        }
    }

    function processInputs()
    {
        if (root.inputsNeedProcessing)
            status.processInput();
    }


    QtObject {
        id: status

        property bool moveForward: false
        property bool moveBack: false

        property bool moveLeft: false
        property bool moveRight: false

        property bool moveUp: false
        property bool moveDown: false

        property bool rotateForward: false
        property bool rotateBack: false

        property bool shiftDown: false
        property bool controlDown: false
        property bool useMouse: false


        property vector2d lastPos: Qt.vector2d(0, 0)
        property vector2d currentPos: Qt.vector2d(0, 0)

        function updatePosition(vector, speed, position)
        {
            if (shiftDown)
                speed *= shiftSpeed;
            else
                speed *= root.speed

            var direction = vector;
            var velocity = Qt.vector3d(direction.x * speed,
                                       direction.y * speed,
                                       direction.z * speed);
            controlledObject.position = Qt.vector3d(position.x + velocity.x,
                                                    position.y + velocity.y,
                                                    position.z + velocity.z);

//            console.log(" ### controlledObject.position:" + controlledObject.position)
        }

        function updateRotation(vector, speed)
        {
            if (controlledObject.rotation.scalar === 0) {
                controlledObject.rotation.scalar = 1
            }

            if (shiftDown)
                speed *= shiftSpeed;
            else
                speed *= root.speed

            var axis = vector.crossProduct(Qt.vector3d(0,1,0)).normalized();

            var additionalRotation = helpers3D.getRotationFromAxisAndAngle(vector, speed)
            controlledObject.rotation = helpers3D.getRotationFromQuaternions(controlledObject.rotation, additionalRotation)
        }

        function negate(vector) {
            return Qt.vector3d(-vector.x, -vector.y, -vector.z)
        }

        function processInput() {
            if (controlledObject == undefined)
                return;

            if (moveForward)
                updatePosition(controlledObject.forward, forwardSpeed, controlledObject.position);
            else if (moveBack)
                updatePosition(negate(controlledObject.forward), backSpeed, controlledObject.position);
            else if (rotateForward)
                updateRotation(controlledObject.forward, rotationSpeed)
            else if (rotateBack)
                updateRotation(negate(controlledObject.forward), rotationSpeed)

            if (moveRight)
                updatePosition(controlledObject.right, rightSpeed, controlledObject.position);
            else if (moveLeft)
                updatePosition(negate(controlledObject.right), leftSpeed, controlledObject.position);

            if (moveDown)
                updatePosition(negate(controlledObject.up), downSpeed, controlledObject.position);
            else if (moveUp)
                updatePosition(controlledObject.up, upSpeed, controlledObject.position);

            if (useMouse) {
                // Get the delta
                var rotation = camera.rotation;
                var mouseRotationDragSpeed = 6.28; // 2*Pi seems a good setting, by chance.
                var delta = Qt.vector2d(lastPos.x - currentPos.x,
                                        lastPos.y - currentPos.y).times(isMouseDragInverted ? -mouseRotationDragSpeed : 1);

                var origin = camera.position
                // Use mouse translation delta to designate, how view center would move.
                var pointSceneTo = camera.mapFromViewport(Qt.vector3d(0.5+delta.x/implicitWidth, 0.5+delta.y/implicitHeight, 0))
                camera.rotation = helpers3D.getRotationFromDirection(pointSceneTo.minus(origin), Qt.vector3d(0,0,1))
                lastPos = currentPos;
                focus = true
                return

                var axisFrom = pointSceneFrom.minus(camera.position)
                var axisTo = pointSceneTo.minus(camera.position)

                if (axisFrom.length() > 0 && axisTo.length() > 0)
                {
                    var additionalRotation = isMouseDragInverted ? helpers3D.getRotationFromAxes(axisTo, axisFrom) :
                                                                   helpers3D.getRotationFromAxes(axisFrom, axisTo)
                    var axis = helpers3D.getRotationAxis(additionalRotation)
                    var angle = helpers3D.getRotationAngle(additionalRotation)

                    angle *= isMouseDragInverted ? 1.0 : mouseRotationDragSpeed

                    additionalRotation = helpers3D.getRotationFromAxisAndAngle(axis, angle)

                    if (additionalRotation.x !== 0)
                    {
                        var preFinalRotation = helpers3D.getRotationFromQuaternions(additionalRotation, rotation)

                        if (preFinalRotation.x)
                        {
                            controlledObject.setRotation(preFinalRotation)
                            camera.setUpPlane(Qt.vector3d(0,0,1))
                        }
                    }
                }

                lastPos = currentPos;
                focus = true
            }

        }
    }
}
