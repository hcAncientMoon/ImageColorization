#ifndef BLENDING_H
#define BLENDING_H
#include <map>
#include "cv.hpp"
#include "cxcore.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "global.h"
using namespace std;
class CBlendPixel{
 public:
    CBlendPixel(){
        m_x=0;m_y=0;
        for(int i=0;i<MAX_BLEND_COLOR;i++){
            m_c[i] = 0;
            m_d[i] = MAX_DISTANCE;
        }
    }
    ~CBlendPixel() {}
    void SetColor(int index,COLORREF c, float distance){
        m_c[index]=c;
        m_d[index]=distance;
    }
    void SetXY(int y,int x){
        m_x = x;
        m_y = y;
    }
    int m_x;    // x-coordinate of the pixel
    int m_y;    // y-coordinate of the pixel
    COLORREF m_c[MAX_BLEND_COLOR];  // maintained chrominance
    float    m_d[MAX_BLEND_COLOR];  // maintained intrinsic distance
};
typedef set<CBlendPixel*> BlendPixelSet;
class Blending{
public:
    cv::Mat Colorize(const cv::Mat pGreyImage, const cv::Mat pLayerImage);
    cv::Mat VideoColorize(const cv::Mat pGreyImage, const cv::Mat pLayerImage);
    Blending();
    virtual ~Blending();
protected:
    void BlendColor();
    void PropagateColor();
    void GenerateColoredSet();
    bool ModifyPixel(CBlendPixel& testPixel,const CBlendPixel & curPixel);
    bool Internal8(int x,int y);
    inline double WeightDistance(double d);

    void AllocateArray();
    void ReleaseArray();

    BlendPixelSet m_ActiveSet;
    CBlendPixel*** m_PixelArray;
private:
    int row;
    int col;
    cv::Mat* pY;
    cv::Mat* pU;
    cv::Mat* pV;
    const cv::Mat* pGrey;
};

#endif // BLENDING_H
