#ifndef IMAGEOPERATOR_H
#define IMAGEOPERATOR_H
#include "stillimagecolorization.h"
#include "global.h"
#include <QImage>
class ImageOperator
{
public:
    ImageOperator();
    static void LoadImage(StillImageColorization* p ,QString fileName,LoadCase ca);
    static QImage mat2QImage  (StillImageColorization* p ,LoadCase ca);
    static QImage mat2QImage(const Mat& mat);
    static Mat qImage2Mat(const QImage &qimage);
    static void seeMat(const Mat& mat);
};

#endif // IMAGEOPERATOR_H
