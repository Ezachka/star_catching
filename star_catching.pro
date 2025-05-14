QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    star.cpp

HEADERS += \
    mainwindow.h \
    star.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


# INCLUDEPATH += D:/opencv/build/include
# LIBS += -LD:/opencv/build/x64/vc16/lib -lopencv_world4110
# LIBS += -LD:/opencv/build/x64/vc16/lib -lopencv_world4110d
# LIBS += -L$$quote(D:/opencv/build/x64/vc16/lib) -lopencv_world4110
# LIBS += -L$$quote(D:/opencv/build/x64/vc16/lib) -lopencv_world4110d

INCLUDEPATH += D:/opencv_mingw/include
LIBS += -LD:/opencv_mingw/x64/mingw/lib \
    -lopencv_core455 \
    -lopencv_imgproc455 \
    -lopencv_highgui455 \
    -lopencv_imgcodecs455
