#-------------------------------------------------
#
# Project created by QtCreator 2014-03-13T12:48:01
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = V2
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    imageoperator.cpp \
    stillimagecolorization.cpp \
    scribblearea.cpp \
    mymatrix.cpp \
    blending.cpp \
    videocolorization.cpp \
    methodselection.cpp

HEADERS  += widget.h \
    imageoperator.h \
    stillimagecolorization.h \
    scribblearea.h \
    global.h \
    mymatrix.h \
    blending.h \
    videocolorization.h \
    methodselection.h

FORMS    +=     widget.ui\
    methodselection.ui

INCLUDEPATH +=  E:\learningResource\OpenCV\opencv\build\include
INCLUDEPATH +=  E:\learningResource\OpenCV\opencv\build\include\opencv
INCLUDEPATH +=  E:\learningResource\OpenCV\opencv\build\include\opencv2
INCLUDEPATH += E:\learningResource\eigen
CONFIG(release,debug|release)
{
    LIBS +=E:\learningResource\OpenCV\opencv\LibForQt\lib\libopencv_calib3d247.dll.a  \
           E:\learningResource\OpenCV\opencv\LibForQt\lib\libopencv_calib3d_pch_dephelp.a \
           E:\learningResource\OpenCV\opencv\LibForQt\lib\libopencv_contrib247.dll.a  \
           E:\learningResource\OpenCV\opencv\LibForQt\lib\libopencv_contrib_pch_dephelp.a \
           E:\learningResource\OpenCV\opencv\LibForQt\lib\libopencv_core247.dll.a \
           E:\learningResource\OpenCV\opencv\LibForQt\lib\libopencv_core_pch_dephelp.a    \
           E:\learningResource\OpenCV\opencv\LibForQt\lib\libopencv_features2d247.dll.a   \
           E:\learningResource\OpenCV\opencv\LibForQt\lib\libopencv_features2d_pch_dephelp.a  \
           E:\learningResource\OpenCV\opencv\LibForQt\lib\libopencv_flann247.dll.a    \
           E:\learningResource\OpenCV\opencv\LibForQt\lib\libopencv_flann_pch_dephelp.a   \
           E:\learningResource\OpenCV\opencv\LibForQt\lib\libopencv_gpu247.dll.a  \
           E:\learningResource\OpenCV\opencv\LibForQt\lib\libopencv_gpu_pch_dephelp.a \
           E:\learningResource\OpenCV\opencv\LibForQt\lib\libopencv_haartraining_engine.a \
           E:\learningResource\OpenCV\opencv\LibForQt\lib\libopencv_highgui247.dll.a  \
           E:\learningResource\OpenCV\opencv\LibForQt\lib\libopencv_highgui_pch_dephelp.a \
           E:\learningResource\OpenCV\opencv\LibForQt\lib\libopencv_imgproc247.dll.a  \
           E:\learningResource\OpenCV\opencv\LibForQt\lib\libopencv_imgproc_pch_dephelp.a \
           E:\learningResource\OpenCV\opencv\LibForQt\lib\libopencv_legacy247.dll.a   \
           E:\learningResource\OpenCV\opencv\LibForQt\lib\libopencv_legacy_pch_dephelp.a  \
           E:\learningResource\OpenCV\opencv\LibForQt\lib\libopencv_ml247.dll.a   \
           E:\learningResource\OpenCV\opencv\LibForQt\lib\libopencv_ml_pch_dephelp.a  \
           E:\learningResource\OpenCV\opencv\LibForQt\lib\libopencv_objdetect247.dll.a    \
           E:\learningResource\OpenCV\opencv\LibForQt\lib\libopencv_objdetect_pch_dephelp.a   \
           E:\learningResource\OpenCV\opencv\LibForQt\lib\libopencv_test_calib3d_pch_dephelp.a    \
           E:\learningResource\OpenCV\opencv\LibForQt\lib\libopencv_test_core_pch_dephelp.a   \
           E:\learningResource\OpenCV\opencv\LibForQt\lib\libopencv_test_features2d_pch_dephelp.a \
           E:\learningResource\OpenCV\opencv\LibForQt\lib\libopencv_test_gpu_pch_dephelp.a    \
           E:\learningResource\OpenCV\opencv\LibForQt\lib\libopencv_test_highgui_pch_dephelp.a    \
           E:\learningResource\OpenCV\opencv\LibForQt\lib\libopencv_test_imgproc_pch_dephelp.a    \
           E:\learningResource\OpenCV\opencv\LibForQt\lib\libopencv_test_ml_pch_dephelp.a \
           E:\learningResource\OpenCV\opencv\LibForQt\lib\libopencv_test_objdetect_pch_dephelp.a  \
           E:\learningResource\OpenCV\opencv\LibForQt\lib\libopencv_test_video_pch_dephelp.a  \
           E:\learningResource\OpenCV\opencv\LibForQt\lib\libopencv_ts247.a   \
           E:\learningResource\OpenCV\opencv\LibForQt\lib\libopencv_ts_pch_dephelp.a  \
           E:\learningResource\OpenCV\opencv\LibForQt\lib\libopencv_video247.dll.a    \
           E:\learningResource\OpenCV\opencv\LibForQt\lib\libopencv_video_pch_dephelp.a \

}
