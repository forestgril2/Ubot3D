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

        switch(fileType)
        {
        case TypedFileDialog.Stl:
            stlModel.geometry.inputFile = fullSystemFilePath
            console.log(" ### stlModel.geometry.inputFile: " + stlModel.geometry.inputFile)
            break
        case TypedFileDialog.Gcode:
            gcodeModel.geometry.inputFile = fullSystemFilePath
            console.log(" ### gcodeModel.geometry.inputFile: " + gcodeModel.geometry.inputFile)
            break

        }
    }
}
