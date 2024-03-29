#QT += quick3d-private quick3druntimerender-private gui quick3dassetimport-private gui-private quick core quick3d quick3dutils-private qml core-private quick3dassetimport
QT += quick3d quickcontrols2 qml widgets xml
CONFIG += c++2a

#QTPLUGIN += assetimporters

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Refer to the documentation for the
# deprecated API to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# TODO: Don't use GMP library yet, but build it if necessary.
# CGAL\include\CGAL\internal\enable_third_party_libraries.h
DEFINES += CGAL_NO_GMP

# TODO: this is only for test.
DEFINES += CGAL_USE_BASIC_VIEWER
QMAKE_CXXFLAGS +=-DCGAL_USE_BASIC_VIEWER
LIBS+=-DCGAL_USE_BASIC_VIEWER

#DEFINES += QT_BUILD_QUICK3DASSETIMPORT_LIB

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

@TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS += src/cpp src/qml

SOURCES += \
    src/cpp/Edge.cpp \
    src/cpp/Extrusion.cpp \
    src/cpp/FileImportExport.cpp \
    src/cpp/GCodeProgramGenerator.cpp \
    src/cpp/Layer.cpp \
    src/cpp/NaivePerimeterGenerator.cpp \
    src/cpp/NaiveSlicer.cpp \
    src/cpp/PolygonTriangulation.cpp \
    src/cpp/SlicerParams.cpp \
    src/cpp/Utils.cpp \
    src/cpp/main.cpp \
    src/cpp/ProcessLauncher.cpp \
    src/cpp/SimplexGeometry.cpp \
    src/cpp/GCodeProgramProcessor.cpp \
    src/cpp/Helpers3D.cpp \
    src/cpp/TriangleConnectivity.cpp \
    src/cpp/TriangleGeometry.cpp \
    src/cpp/GCodeGeometry.cpp \
    src/cpp/Chronograph.cpp \
    $$PWD/../gpr/src/gcode_program.cpp \
    $$PWD/../gpr/src/parser.cpp


HEADERS += \
        src/cpp/Edge.h \
        src/cpp/Extrusion.h \
        src/cpp/FileImportExport.h \
        src/cpp/GCodeProgramGenerator.h \
        src/cpp/Layer.h \
        src/cpp/NaivePerimeterGenerator.h \
        src/cpp/NaiveSlicer.h \
        src/cpp/NaiveSlicerHeaders.h \
        src/cpp/PolygonTriangulation.h \
        src/cpp/SlicerParams.h \
        src/cpp/TriangleGeometry.h \
        src/cpp/CommonDefs.h \
        src/cpp/ProcessLauncher.h \
        src/cpp/SimplexGeometry.h \
        src/cpp/GCodeGeometry.h \
        src/cpp/Chronograph.h \
        $$PWD/../gpr/src/gcode_program.h \
        $$PWD/../gpr/src/parser.h \
        src/cpp/GCodeProgramProcessor.h \
        src/cpp/Helpers3D.h \
        src/cpp/TriangleConnectivity.h \
        src/cpp/Utils.h

RESOURCES += \
    src/qml/qml.qrc \
    Ubot3D_en_150.ts

OTHER_FILES += \
        doc/src/*.*

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_IMPORT_PATH =
# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

QML_IMPORT_NAME = customgeometry
QML_IMPORT_MAJOR_VERSION = 1

## Additional import path used to resolve QML modules just for Qt Quick Designer
#QML_IMPORT_PATH =
## Additional import path used to resolve QML modules just for Qt Quick Designer
#QML_DESIGNER_IMPORT_PATH =

#QML_IMPORT_NAME = GCodeGeometry
#QML_IMPORT_MAJOR_VERSION = 1

CONFIG += qmltypes

# link against gcode parser library = doesn't work
#LIBS += -L$$PWD/../gpr/build/msys-gpr.dll

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#target.path = $$[QT_INSTALL_EXAMPLES]/quick3d/customgeometry
#INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$$$PWD/..\assimp\build\bin\Release\libassimp-5.dll
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$$$PWD/..\assimp\build\bin\Release\libassimp-5.dll# -L$$PWD/../gpr/build/ -lmsys-gpr
else:win32:CONFIG(debug, debug|release): LIBS += -L$$$$PWD/..\assimp\build_debug_msvc\bin\Debug\assimp-vc143-mtd.dll# -L$$PWD/../gpr/build/ -lmsys-gpr


unix {
CGAL_INCLUDEPATH=/usr/include
BOOST_INCLUDEPATH=/usr/include/boost
ASSIMP_INCLUDEPATH=/usr/include/assimp
CLIPPER_INCLUDEPATH=/usr/include/polyclipping

EIGEN_PATH=$$PWD/../eigen
GLM_PATH=$$PWD/../glm
GPR_PATH=$$PWD/../gpr/src

INCLUDEPATH += \
$$[QT_INSTALL_HEADERS]/QtQuick3DRuntimeRender/$$[QT_VERSION]/ \
$$[QT_INSTALL_HEADERS]/QtQuick3DUtils/$$[QT_VERSION]/ \
$$[QT_INSTALL_HEADERS]/QtQuick3DAssetImport/$$[QT_VERSION]/ \
$$[QT_INSTALL_HEADERS]/QtGui/$$[QT_VERSION] \
$$[QT_INSTALL_HEADERS]/QtGui/$$[QT_VERSION]/QtGui

QT_SRC=$$[QT_INSTALL_PREFIX]/../Src
HEADERS += $$QT_SRC/qtquick3d/src/runtimerender/qssgrenderray_p.h
SOURCES += $$QT_SRC/qtquick3d/src/runtimerender/qssgrenderray.cpp

LIBS += -lpolyclipping -lassimp
}

win32 {
CLIPPER_INCLUDEPATH=$$PWD/../clipper/cpp/
CGAL_INCLUDEPATH=D:/Programy/CGAL/include
BOOST_INCLUDEPATH=D:/Programy/boost_1_75_0

EIGEN_PATH=$$PWD/../eigen
GLM_PATH=$$PWD/../glm
GPR_PATH=$$PWD/../gpr/src
JSON_PATH=$$PWD/../json/single_include/nlohmann

INCLUDEPATH += \
$$[QT_INSTALL_HEADERS]/ \
$$[QT_INSTALL_HEADERS]/QtQuick3DRuntimeRender/$$[QT_VERSION]/ \
$$[QT_INSTALL_HEADERS]/QtQuick3DUtils/$$[QT_VERSION]/ \
$$[QT_INSTALL_HEADERS]/QtQuick3DAssetImport/$$[QT_VERSION]/ \
$$[QT_INSTALL_HEADERS]/QtGui/$$[QT_VERSION] \
$$[QT_INSTALL_HEADERS]/QtGui/$$[QT_VERSION]/QtGui

message($$[QT_INSTALL_HEADERS])
message($$[QT_VERSION])

INCLUDEPATH += D:\Qt62b\qtbase\include\QtQuick3DUtils\6.2.1
INCLUDEPATH += D:\Qt62b\qtbase\include\QtGui\6.2.1
INCLUDEPATH += D:\Qt62b\qtbase\include\QtQuick3DAssetImport\6.2.1
INCLUDEPATH += D:\Qt62b\qtbase\include\QtQuick3DAssetImport\6.2.1
INCLUDEPATH += $$PWD/..\assimp\include
INCLUDEPATH += $$PWD/..\assimp\build\include


HEADERS += $$PWD/../clipper/cpp/clipper.hpp
SOURCES += $$PWD/../clipper/cpp/clipper.cpp
}


INCLUDEPATH += $$PWD/src/cpp
INCLUDEPATH += $$ASSIMP_INCLUDEPATH
INCLUDEPATH += $$CLIPPER_INCLUDEPATH
INCLUDEPATH += $$BOOST_INCLUDEPATH
INCLUDEPATH += $$EIGEN_PATH
INCLUDEPATH += $$GLM_PATH
INCLUDEPATH += $$GPR_PATH
INCLUDEPATH += $$CGAL_INCLUDEPATH
INCLUDEPATH += $$JSON_PATH

DEPENDPATH += $$GPR_PATH
DEPENDPATH += $$ASSIMP_INCLUDEPATH

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../assimp/build/lib/Release/ -llibassimp.dll
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../assimp/build/lib/Release/ -llibassimp
#else:unix:!macx: LIBS += -L$$PWD/../assimp/build/lib/Release/ -llibassimp.dll

LIBS+="D:\Projects\assimp\build_debug_msvc\lib\Debug\assimp-vc143-mtd.lib"

INCLUDEPATH += $$PWD/../assimp/build/lib/Release
DEPENDPATH += $$PWD/../assimp/build/lib/Release
