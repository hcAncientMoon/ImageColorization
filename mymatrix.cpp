#include "mymatrix.h"
#include <vector>
#include <QDebug>
#include "cv.hpp"
#include "cxcore.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "imageoperator.h"
MyMatrix::MyMatrix(int channel = 0)
{
    memset(this->image,0,MAXCHANNEL*sizeof(MatrixXf*));
    channelOfImage = channel;
}
MyMatrix::MyMatrix(const cv::Mat& m){
    int channel = m.channels();
    if(channel>MAXCHANNEL) {
        memset(this->image,0,MAXCHANNEL*sizeof(MatrixXf*));
        channelOfImage = 0;
        return ;
    }
    channelOfImage = channel;
    std::vector<cv::Mat> tmpVector (channelOfImage);
    cv::split(m,tmpVector);
    for(channel = 0;channel<channelOfImage;channel++){
        image[channel] = new MatrixXf(m.rows,m.cols);
        cv::Mat tmpMat = tmpVector[channel];
        for(int cntI = 0;cntI<m.rows;cntI++){
            for(int cntJ = 0;cntJ<m.cols;cntJ++){
                    (*(image[channel]))(cntI,cntJ) = tmpMat.at<unsigned char>(cntI,cntJ);
            }
        }
    }
}
//大小通道需要相同
MyMatrix MyMatrix::operator -(MyMatrix mm){
    MyMatrix* myMatrix = new MyMatrix(channelOfImage);
    for(int i =0;i<channelOfImage;i++){
        *(*myMatrix)[i] = *(image[i])-*(mm[i]);
    }
    return *myMatrix;
}
MatrixXf* MyMatrix::operator [](int n )const{
    return getChannelElem(n);
}
MyMatrix::~MyMatrix(){
//    delete []image;
}
int MyMatrix::getChannel()const {
    return channelOfImage;
}
MatrixXf* MyMatrix::getChannelElem(int channel)const {
    return image[channel];
}
cv::Mat MyMatrix::convertToMat()const{

    const int row = (*(image[0])).rows();
    const int col = (*(image[0])).cols();
    std::vector<cv::Mat>vectorMerge(channelOfImage);
    for(int channel =0;channel<channelOfImage;channel++){
        cv::Mat* m = new cv::Mat(row,col,CV_8U);
        for(int i =0;i<row;i++)
            for(int j =0;j<col;j++){
                m->at<unsigned char >(i,j) = (*(image[channel]))(i,j);
            }
        vectorMerge[channel] = *m;
    }
    cv::Mat result(row,col,CV_8UC3) ;
    cv::merge(vectorMerge,result);
    return result;
}
