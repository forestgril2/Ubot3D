import Qt.labs.platform 1.1
import QtQuick.Controls 2.15
import QtQuick 2.15

FileDialog {
    id: root

    enum FileType {
        GcodeImport,
        StlImport,
        StlExport
    }

    property int fileType: TypedFileDialog.Stl

    onAccepted:
    {
        setInputFile(fileDialog.currentFile)
    }

    function importStl()
    {
        nameFilters = ["STL files (*.stl *.STL)", "Object files (*.obj)"]
        fileMode = FileDialog.OpenFiles
        fileType = TypedFileDialog.StlImport
        open()
    }

    function exportStl()
    {
        nameFilters = ["STL files (*.stl *.STL)"]
        fileMode = FileDialog.SaveFile
        fileType = TypedFileDialog.StlExport
        open()
    }

    function importGcode()
    {
        nameFilters = ["Gcode files (*.gcode)"]
        fileMode = FileDialog.OpenFile
        fileType = TypedFileDialog.GcodeImport
        open()
    }

    function setInputFile(filePath) {
        var fullSystemFilePath = filePath.toString()
        var pos = fullSystemFilePath.search("file:///");
        fullSystemFilePath = fullSystemFilePath.substring(pos+8)

        switch(fileType)
        {
        case TypedFileDialog.StlImport:
            stlModel.geometry.inputFile = fullSystemFilePath
            console.log(" ### stlModel.geometry.inputFile: " + stlModel.geometry.inputFile)
            break
        case TypedFileDialog.StlExport:
            stlModel.exportModelToSTL(fullSystemFilePath)
            console.log(" ### stlModel.geometry.exportFile: " + fullSystemFilePath)
            break
        case TypedFileDialog.GcodeImport:
            gcodeModel.geometry.inputFile = fullSystemFilePath
            console.log(" ### gcodeModel.geometry.inputFile: " + gcodeModel.geometry.inputFile)
            break

        }
    }
}
