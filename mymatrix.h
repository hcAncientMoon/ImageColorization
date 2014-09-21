#ifndef MYMATRIX_H
#define MYMATRIX_H
#include "core/core.hpp"
#include "Eigen/Eigen"
#include "global.h"
using namespace Eigen;

class MyMatrix
{
public:
    MyMatrix(int );
    MyMatrix(const cv::Mat& m);
    MyMatrix operator -(const MyMatrix mm);
    MatrixXf* operator []  (int n ) const;

    int getChannel () const;
    MatrixXf* getChannelElem(int channel)const;
    cv::Mat convertToMat()const;
    ~MyMatrix();
private :
    MatrixXf* image[MAXCHANNEL];
    int channelOfImage;
};

#endif // MYMATRIX_H
