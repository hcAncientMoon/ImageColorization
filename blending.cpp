#include "blending.h"
#include "global.h"
#include <math.h>
#include <QDebug>
Blending::Blending(){
}
Blending::~Blending(){}
void Blending::AllocateArray(){
    m_ActiveSet.clear();
    m_PixelArray = new CBlendPixel** [row];
    for(int y=0;y<row;y++){
        m_PixelArray[y] = new CBlendPixel* [col];
        for(int x=0;x<col;x++)
            m_PixelArray[y][x] = new CBlendPixel();
    }
}
void Blending::ReleaseArray(){
    for(int y=0;y<row;y++)
        {
            for(int x=0;x<col;x++)
            {
                if (m_PixelArray[y][x]!=NULL)
                    delete m_PixelArray[y][x];
            }
            delete [] m_PixelArray[y];
        }
        delete [] m_PixelArray;

        m_ActiveSet.clear();
}
void Blending::GenerateColoredSet(){
    int x,y;
    for(y=0;y<row;y++){
        for(x=0;x<col;x++){
            CBlendPixel * pPixel =  m_PixelArray[y][x];
            pPixel->SetXY(y,x);
            if (pY->at<uchar>(y,x)!=0){
                pY->at<uchar>(y,x) = 1;
                int Cb = pU->at<uchar>(y,x);
                int Cr = pV->at<uchar>(y,x);
                COLORREF key = RGB(Cb,Cr,0);

                pPixel->SetColor(0,key,0);
                //只加入边界的值
                if (!Internal8(x,y)){
                    m_ActiveSet.insert(pPixel);
                }
            }
        }
    }
}
bool Blending::Internal8(int x, int y){
    int result = 1;
    if (x>0)  {
        result*= pY->at<uchar>(y,x-1);
        if (result==0) return false;
        if (y>0){
            result*=pY->at<uchar>(y-1,x-1);
            if (result==0) return false;
        }
        if (y<row-1){
            result *=pY->at<uchar>(y+1,x-1);
            if (result==0) return false;
        }
    }
    if (x<col-1){
        result*=pY->at<uchar>(y,x+1);
        if (result==0) return false;
        if (y>0){
            result*=pY->at<uchar>(y-1,x+1);
            if (result==0) return false;
        }
        if (y<row-1){
            result*=pY->at<uchar>(y+1,x+1);
            if (result==0) return false;
        }
    }
    if (y>0)
        result*= pY->at<uchar>(y-1,x);
    if (y<row-1)
        result*=pY->at<uchar>(y+1,x);
    return result==0?false:true;
}
void Blending::PropagateColor(){
    BlendPixelSet::iterator it =m_ActiveSet.begin();
    BlendPixelSet::iterator it_temp;
    while (!m_ActiveSet.empty()){
        it =m_ActiveSet.begin();
        while (it!=m_ActiveSet.end()){
            CBlendPixel * pPixel = *it;
            for(int y=pPixel->m_y-1;y<=pPixel->m_y+1;y++){
                for(int x=pPixel->m_x-1;x<=pPixel->m_x+1;x++)
                 {
                     if (x==pPixel->m_x||y==pPixel->m_y)//四邻域
                         if (x>=0&&x<=col-1&&y>=0&&y<=row-1){
                             CBlendPixel * testPixel = m_PixelArray[y][x];
                             if (ModifyPixel(*testPixel,*pPixel))
                                m_ActiveSet.insert(testPixel);
                     }
                 }
            }
                it_temp = it; it++;
                m_ActiveSet.erase(it_temp);
        }
    }
}
bool Blending::ModifyPixel(CBlendPixel &testPixel, const CBlendPixel &curPixel){
    if (pY->at<uchar>(testPixel.m_y,testPixel.m_x)==1)return false;
    double y1 = pGrey->at<uchar>(curPixel.m_y,curPixel.m_x);
    double y2 = pGrey->at<uchar>(testPixel.m_y,testPixel.m_x);
    double d = abs(y1-y2);
    bool result = false;
    for(int i=0;i<MAX_BLEND_COLOR;i++){
        if (curPixel.m_d[i]<MAX_DISTANCE){
           int j=0;
           while (((curPixel.m_d[i]+d)>=testPixel.m_d[j])&&(j<=MAX_BLEND_COLOR)){
               if (curPixel.m_c[i]==testPixel.m_c[j]){
                   j=MAX_BLEND_COLOR;
                   break;
               }
               j++;
           }
           if (j<MAX_BLEND_COLOR){
               if (curPixel.m_c[i]!=testPixel.m_c[j])
               for(int k=MAX_BLEND_COLOR-1;k>j;k--){
                   if (testPixel.m_c[k-1]!=curPixel.m_c[i]){
                    testPixel.m_c[k]=testPixel.m_c[k-1];
                    testPixel.m_d[k]=testPixel.m_d[k-1];
                   }
               }
               testPixel.m_c[j]=curPixel.m_c[i];
               testPixel.m_d[j]=curPixel.m_d[i]+d;
               result = true;
           }
            else
               break;
        }
    }
    return result;
}
void Blending::BlendColor(){
    for(int y=0;y<row;y++){
        for(int x=0;x<col;x++){
            CBlendPixel * pPixel =  m_PixelArray[y][x];
            double Cb=0;
            double Cr=0;
            double Weight=0;
            if (pY->at<uchar>(y,x)==0){
                for(int i=0;i<MAX_BLEND_COLOR;i++){
                    double w = WeightDistance(pPixel->m_d[i]);
                    Cb += GetRValue(pPixel->m_c[i])*w;
                    Cr += GetGValue(pPixel->m_c[i])*w;
                    Weight += w;
                }
                Cb = Cb/ Weight;
                Cr = Cr/ Weight;
                pU->at<uchar>(y,x)=(int)(Cb+0.5);
                pV->at<uchar>(y,x)=(int)(Cr+0.5);
            }
        }
    }
}
double Blending::WeightDistance(double d){
    if (d==MAX_DISTANCE) return 0.0;
    if (d==0) d=0.5;
    return 1.0/(d*d*d*d);
}
cv::Mat Blending::Colorize( const cv::Mat pGreyImage, const cv::Mat pLayerImage){
    using namespace cv;
    row = pGreyImage.rows;
    col = pGreyImage.cols;
    pGrey = &pGreyImage;
    cvtColor(pLayerImage,pLayerImage,CV_BGR2YUV);
   vector<Mat> layerVector(3);
    split(pLayerImage,layerVector);//here
    pY = new Mat(layerVector[0]);
    pU = new Mat(layerVector[1]);
    pV = new Mat(layerVector[2]);
    // Colorize Algorithm here
    AllocateArray();
    GenerateColoredSet();
    PropagateColor();
    BlendColor();
    ReleaseArray();
    vector<Mat> resultVector(3);
    resultVector[0] = *pGrey;
    resultVector[1] = *pU;
    resultVector[2] = *pV;
    Mat m ;
    merge(resultVector,m);
    cvtColor(m,m,CV_YUV2BGR);
    return m;
}
//输入输出均使用YUV空间
cv::Mat Blending::VideoColorize( const cv::Mat pGreyImage, const cv::Mat pLayerImage){
    using namespace cv;
    row = pGreyImage.rows;
    col = pGreyImage.cols;
    pGrey = &pGreyImage;
   vector<Mat> layerVector(3);
    split(pLayerImage,layerVector);//here
    pY = new Mat(layerVector[0]);
    pU = new Mat(layerVector[1]);
    pV = new Mat(layerVector[2]);
    // Colorize Algorithm here
    AllocateArray();
    GenerateColoredSet();
    PropagateColor();
    BlendColor();
    ReleaseArray();
    vector<Mat> resultVector(3);
    resultVector[0] = *pGrey;
    resultVector[1] = *pU;
    resultVector[2] = *pV;
    Mat m ;
    merge(resultVector,m);
    return m;
}
