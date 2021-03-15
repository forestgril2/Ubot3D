#QT += quick3d-private quick3druntimerender-private gui quick3dassetimport-private gui-private quick core quick3d quick3dutils-private qml core-private quick3dassetimport
QT += quick3d qml

CONFIG += c++20

#QTPLUGIN += assetimporters

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Refer to the documentation for the
# deprecated API to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
#DEFINES += QT_BUILD_QUICK3DASSETIMPORT_LIB

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

@TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS += src/cpp src/qml

SOURCES += src/cpp/main.cpp \
    src/cpp/GCodeProgramProcessor.cpp \
    src/cpp/Helpers3D.cpp \
    src/cpp/TriangleGeometry.cpp \
    src/cpp/GCodeGeometry.cpp \
    src/cpp/Chronograph.cpp \
    $$PWD/../gpr/src/gcode_program.cpp \
    $$PWD/../gpr/src/parser.cpp


HEADERS +=  src/cpp/TriangleGeometry.h \
        ../gpr/src/CommonDefs.h \
        src/cpp/GCodeGeometry.h \
        src/cpp/Chronograph.h \
        $$PWD/../gpr/src/gcode_program.h \
        $$PWD/../gpr/src/parser.h \
        src/cpp/GCodeProgramProcessor.h \
        src/cpp/Helpers3D.h

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

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../assimp-5.0.1/build/code/release/ -lassimp-vc142-mtd
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../assimp-5.0.1/build/code/Debug/ -lassimp-vc142-mtd# -L$$PWD/../gpr/build/ -lmsys-gpr
else:unix: LIBS += -L$$PWD/../assimp-5.0.1/build/code/ -lassimp-vc142-mtd

INCLUDEPATH += $$PWD/../eigen
INCLUDEPATH += $$PWD/../gpr/src
INCLUDEPATH += $$PWD/src/cpp
DEPENDPATH += $$PWD/../gpr/src
INCLUDEPATH += $$PWD/../assimp-5.0.1/include
INCLUDEPATH += $$PWD/../assimp-5.0.1/build/include
DEPENDPATH += $$PWD/../assimp-5.0.1/include
DEPENDPATH += $$PWD/../assimp-5.0.1/build/include

INCLUDEPATH += D:\Projects\qt6-build-release-a80e52\qtbase\include\QtQuick3DUtils\6.0.1
INCLUDEPATH += D:\Projects\qt6-build-release-a80e52\qtbase\include\QtGui\6.0.1
INCLUDEPATH += D:\Projects\qt6-build-release-a80e52\qtbase\include\QtQuick3DAssetImport\6.0.1

#QT += quick3dassetimport-private quick3dutils-private shadertools-private quick-private

#include(graphobjects/graphobjects.pri)
#include(rendererimpl/rendererimpl.pri)
#include(resourcemanager/resourcemanager.pri)

#HEADERS += \
#    qssgrendercommands_p.h \
#    qtquick3druntimerenderglobal_p.h \
#    qssgrenderableimage_p.h \
#    qssgrenderclippingfrustum_p.h \
#    qssgrendercontextcore_p.h \
#    qssgrhicustommaterialsystem_p.h \
#    qssgrenderdefaultmaterialshadergenerator_p.h \
#    qssgrendererutil_p.h \
#    qssgrenderimagetexturedata_p.h \
#    qssgrenderinputstreamfactory_p.h \
#    qssgrendermaterialshadergenerator_p.h \
#    qssgrendermesh_p.h \
#    qssgrenderray_p.h \
#    qssgrendershadercache_p.h \
#    qssgrendershadercodegenerator_p.h \
#    qssgrendershaderkeys_p.h \
#    qssgrendershadowmap_p.h \
#    qssgruntimerenderlogging_p.h \
#    qssgperframeallocator_p.h \
#    qssgshaderresourcemergecontext_p.h \
#    qssgrendershadermetadata_p.h \
#    qssgrhiquadrenderer_p.h \
#    qssgrhieffectsystem_p.h \
#    qssgrhicontext_p.h \
#    qssgshadermaterialadapter_p.h \
#    qssgshadermapkey_p.h

#SOURCES += \
#    qssgrenderclippingfrustum.cpp \
#    qssgrendercommands.cpp \
#    qssgrendercontextcore.cpp \
#    qssgrhicustommaterialsystem.cpp \
#    qssgrenderdefaultmaterialshadergenerator.cpp \
#    qssgrenderinputstreamfactory.cpp \
#    qssgrenderray.cpp \
#    qssgrendershadercache.cpp \
#    qssgrendershadercodegenerator.cpp \
#    qssgrendershadermetadata.cpp \
#    qssgrendershadowmap.cpp \
#    qssgruntimerenderlogging.cpp \
#    qssgrhiquadrenderer.cpp \
#    qssgrhieffectsystem.cpp \
#    qssgrhicontext.cpp \
#    qssgshadermaterialadapter.cpp
