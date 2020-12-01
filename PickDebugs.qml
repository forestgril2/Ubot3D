import QtQuick 2.15
import QtQuick.Controls 2.15

Row {
    property alias pickPosition: pickPosition
    anchors.top: parent.top
    anchors.horizontalCenter: parent.horizontalCenter
    spacing: 20
    Label {
        id: pickName
        color: "#222840"
        font.pointSize: 14
        text: "Last Pick: None"
    }
    Label {
        id: pickPosition
        color: "#222840"
        font.pointSize: 14
        text: "Screen Position: (0, 0)"
    }
    Label {
        id: uvPosition
        color: "#222840"
        font.pointSize: 14
        text: "UV Position: (0.00, 0.00)"
    }
    Label {
        id: distance
        color: "#222840"
        font.pointSize: 14
        text: "Distance: 0.00"
    }
    Label {
        id: scenePosition
        color: "#222840"
        font.pointSize: 14
        text: "World Position: (0.00, 0.00)"
    }
}
