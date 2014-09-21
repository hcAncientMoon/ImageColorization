#ifndef STILLIMAGECOLORIZATION_H
#define STILLIMAGECOLORIZATION_H
#include "cv.hpp"
#include "cxcore.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <QString>
#include <QImage>
#include "global.h"
#include "mymatrix.h"
#include <map>
using namespace cv;

//使用单例模式
class StillImageColorization
{
private :
    Mat* grayScale;
    Mat* referenceScale;
    static StillImageColorization* instance;
public:
    static StillImageColorization* getInstance();
    ~StillImageColorization(){
        if(instance){
            delete instance->getGray();
            delete instance->getRefer();
            delete instance;
        }
    }
    //初始化参数
    void initSource(QString);
    void initSource(Mat);
    void initReference(QString);
    void initReference(const QImage&);
    //
    const Mat* getGray(){return grayScale;}
    const Mat* getRefer(){return referenceScale;}
    //
    Mat doColorization(ColorizationMethod method);
    Mat levinColorization();
    Mat quadTreeDecomposition();
    Mat geodesicColorization(MediaType media);
    //
    Mat forVideoColorizationWithYUV(Mat& gray,Mat& refer);
    Mat forVideoColorizationWithBGR(Mat &gray, Mat &refer);
    //
    void clearLoadImages();
private:
    StillImageColorization();
};
struct NodeOfImage {
    NodeOfImage(int r,int c,int height,int width):_r(r),_c(c),_width(width),_height(height){;}
    int _r;
    int _c;
    int _width;
    int _height;
};
#endif // STILLIMAGECOLORIZATION_H
