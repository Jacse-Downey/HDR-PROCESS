QT       += core gui charts
QT += widgets axcontainer

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    caijian.cpp \
    canshu.cpp \
    imageprocess.cpp \
    main.cpp \
    mainwindow.cpp \
    mapping_show.cpp \
    process.cpp \
    qconvert.cpp \
    quxian.cpp

HEADERS += \
    D:/HDR/app example/test4/imageprocess.h \
    D:/HDR/app example/test4/imageprocess.h \
    caijian.h \
    canshu.h \
    imageprocess.h \
    mainwindow.h \
    mapping_show.h \
    process.h \
    qconvert.h \
    quxian.h

FORMS += \
    caijian.ui \
    canshu.ui \
    mainwindow.ui \
    mapping_show.ui \
    quxian.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -LD:/HDR/OpenCV4.6/opencv/build/x64/vc14/lib/ -lopencv_world460
else:win32:CONFIG(debug, debug|release): LIBS += -LD:/HDR/OpenCV4.6/opencv/build/x64/vc14/lib/ -lopencv_world460d
else:unix: LIBS += -LD:/HDR/OpenCV4.6/opencv/build/x64/vc14/lib/ -lopencv_world460

INCLUDEPATH += D:/HDR/OpenCV4.6/opencv/build/include
DEPENDPATH += D:/HDR/OpenCV4.6/opencv/build/include
INCLUDEPATH += D:/HDR/HDR-APP/eigen-3.4.0
INCLUDEPATH += D:/HDR/HDR-APP/gsl/include
LIBS += -LD:/HDR/HDR-APP/gsl/lib -llibgsl-23
LIBS += -LD:/HDR/HDR-APP/gsl/lib -llibgslcblas-0
DEFINES += GSL_DLL

