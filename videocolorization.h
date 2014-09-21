#ifndef VIDEOCOLORIZATION_H
#define VIDEOCOLORIZATION_H
#include <QWidget>
#include "stillimagecolorization.h"
#include "highgui.h"
#include <iostream>
#include <set>
using namespace std;
//使用单例模式
class VideoColorization
{
public:
    static VideoColorization* getInstance();
    void init(QString fileName, StillImageColorization * & ptr);
    void setStartFrame(double d);
    Mat getFirstFrame();
    //
    Mat doColorization(VideoColorizationMethod method, Mat coloredFirstFrame);
    //各彩色化算法
    void frameSubColorization(Mat coloredFirstFrame);
    //
    void motionEstimationColorization(Mat coloredFirstFrame);
//    void adjustMotionVector(const Mat& pre,const Mat & pro,Mat& motionVector,int blockSize=MBLOCKSIZE);
    void ColorizaitonWithFrameAndMotion(Mat coloredFirstFrame);
    //
    Mat keyFrameColorization(Mat coloredFirstFrame);

private:
    VideoColorization();
    void getEstimateVector(const Mat& pre, const Mat& pro, Mat& motionVector, int blockSize = MBLOCKSIZE);
    inline  float costMADFun(const Mat&m1,const Mat& m2,int topLeftX1,int topLeftY1,int topLeftX2,int topLeftY2,int blockSize);
    void generateScribbleByMotionVector(const Mat& motionVector, const Mat& curFrame, Mat& scribbleFrame, int blockSize = MBLOCKSIZE);
    void calHistVector(Mat& src,Mat& histVector,int cntOfRB=4,int cntOfCB=4,int bins=18);
    void extractKeyFrameUsingFCM();
private:
    static VideoColorization* instance;
    StillImageColorization* imageColorizationPtr;
    VideoCapture* capPtr;
    double startFrame;//起始帧
    Mat firstGrayFrame;
    QString videoName;

    VideoWriter* vWPtr;
    long frameCnt;
    set<long> keyFrames;
};

#endif // VIDEOCOLORIZATION_H
