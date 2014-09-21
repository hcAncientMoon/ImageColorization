#include "imageoperator.h"
#include <QFile>
#include <QString>
#include <QDebug>
ImageOperator::ImageOperator()
{
}
void ImageOperator:: LoadImage(StillImageColorization* p ,QString fileName,LoadCase ca){
    if(ca==GRAY)p->initSource(fileName);
    if(ca==SCRIBBLE)p->initReference(fileName);
}
QImage ImageOperator::mat2QImage(StillImageColorization *p, LoadCase ca){
        int i = 0;
        const Mat* mat ;
        if (ca==GRAY) mat = p->getGray();
        else mat = p->getRefer();
        i = mat->channels();
        Mat rgb;
        switch(i)
        {
        case 1:
            cvtColor(*mat,rgb,CV_GRAY2RGB);
            break;
        case 2://never happen
            break;
        case 3:
            cvtColor(*mat,rgb,CV_BGR2RGB);
            // for debug
            //cvShowImage("debug",(IplImage*)mat);
            break;
        case 4:
            cvtColor(*mat,rgb,CV_BGRA2RGB);
        }
        //防止偏移
         return  QImage((const unsigned char*)rgb.data,rgb.cols,rgb.rows,rgb.cols*3,QImage::Format_RGB888).copy(0,0,rgb.cols,rgb.rows);
}
QImage ImageOperator::mat2QImage(const Mat& mat){
    int i = mat.channels();
    Mat rgb;
    switch(i)
    {
    case 1:
        cvtColor(mat,rgb,CV_GRAY2RGB);
        break;
    case 2://never happen
        break;
    case 3:
        cvtColor(mat,rgb,CV_BGR2RGB);
        // for debug
        //cvShowImage("debug",(IplImage*)mat);
        break;
    case 4:
        cvtColor(mat,rgb,CV_BGRA2RGB);
    }
    //防止偏移
     return  QImage((const unsigned char*)rgb.data,rgb.cols,rgb.rows,rgb.cols*3,QImage::Format_RGB888).copy(0,0,rgb.cols,rgb.rows);
}
Mat ImageOperator::qImage2Mat(const QImage& qimage){
    int channel = qimage.bytesPerLine()/qimage.width();
    Mat mat;
    switch (channel) {
    case 1:
        mat = Mat(qimage.height(),qimage.width(),CV_8UC1,(uchar*)qimage.bits(),size_t(qimage.bytesPerLine()));
        cvtColor(mat,mat,CV_GRAY2BGR);
        break;
    case 2://never happen
        break;
    case 3:
        mat = Mat(qimage.height(),qimage.width(),CV_8UC3,(uchar*)qimage.bits(),size_t(qimage.bytesPerLine()));
        cvtColor(mat,mat,CV_RGB2BGR);
        break;
    case 4:
        mat = Mat(qimage.height(),qimage.width(),CV_8UC4,(uchar*)qimage.bits(),size_t(qimage.bytesPerLine()));
        cvtColor(mat,mat,CV_RGBA2BGR);
    default:
        break;
    }
    return mat;
}
void ImageOperator::seeMat(const Mat& mat){
    QFile outFile("debuglog.txt");
    int channel;
    const int rows = mat.rows;
    const int cols = mat.cols;
    std::vector<Mat> splitVector(mat.channels());
    cv::split(mat,splitVector);
    QString txt;
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    for(channel=0;channel<mat.channels();channel++){
        for(int i =0;i<rows;i++)
            for(int j =0;j<cols;j++){
                 txt= "channel \t";
                 txt.append(splitVector[channel].at<uchar>(i,j));
                 txt.append("\n");
                ts << txt << endl;
            }
    }
}
