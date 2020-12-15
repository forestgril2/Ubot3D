import Qt.labs.platform 1.1
import QtQuick.Controls 2.15
import QtQuick 2.15

FileDialog {
    id: root

    enum FileType {
        Gcode,
        Stl
    }

    property int fileType: TypedFileDialog.Stl

    onAccepted:
    {
        setInputFile(fileDialog.currentFile)
    }

    function setInputFile(filePath) {
        var fullSystemFilePath = filePath.toString()
        var pos = fullSystemFilePath.search("file:///");
        fullSystemFilePath = fullSystemFilePath.substring(pos+8)
        console.log(fullSystemFilePath)

        switch(fileType)
        {
        case TypedFileDialog.Stl:
            stlModel.geometry.inputFile = fullSystemFilePath
            break
        case TypedFileDialog.Gcode:
            gcodeGeometry.inputFile = fullSystemFilePath

        }
    }
}
