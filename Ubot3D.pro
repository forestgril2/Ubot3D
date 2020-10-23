QT += 3dcore 3drender 3dinput 3dlogic 3dextras 3danimation quick quick3d

CONFIG += c++17

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Refer to the documentation for the
# deprecated API to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    examplegeometry.cpp

HEADERS += \
        examplegeometry.h

RESOURCES += \
    Ubot3D_en_150.ts

OTHER_FILES += \
        doc/src/*.*

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_IMPORT_PATH =
# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

QML_IMPORT_NAME = customgeometry
QML_IMPORT_MAJOR_VERSION = 1

CONFIG += qmltypes

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#target.path = $$[QT_INSTALL_EXAMPLES]/quick3d/customgeometry
#INSTALLS += target


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../assimp-5.0.1/build/code/release/ -lassimp-vc142-mtd
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../assimp-5.0.1/build/code/Debug/ -lassimp-vc142-mtd
else:unix: LIBS += -L$$PWD/../assimp-5.0.1/build/code/ -lassimp-vc142-mtd

INCLUDEPATH += $$PWD/../assimp-5.0.1/include
INCLUDEPATH += $$PWD/../assimp-5.0.1/build/include
DEPENDPATH += $$PWD/../assimp-5.0.1/include
DEPENDPATH += $$PWD/../assimp-5.0.1/build/include

