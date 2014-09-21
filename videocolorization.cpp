#include "videocolorization.h"
#include "global.h"
#include <vector>
#include <map>
#include <QDebug>
#include <QMessageBox>
#define T 30
#define B 50
/*
 * impelement of the video colorization
 * by huchang
 * @2014 - 3 - 28
*/
VideoColorization* VideoColorization::instance = NULL;
VideoColorization::VideoColorization(){
    capPtr = NULL;
    imageColorizationPtr = NULL;
    vWPtr = NULL;
    startFrame = 0;
    frameCnt = 0;
}
VideoColorization* VideoColorization::getInstance(){
    if (instance==NULL){
        instance = new VideoColorization();
    }
    return instance;
}
void VideoColorization::init(QString fileName,StillImageColorization* & ptr){
    if(!fileName.isEmpty()){
        if(capPtr!=NULL) capPtr->release();
        capPtr = new VideoCapture(fileName.toStdString());
        if(ptr==NULL){
            ptr=StillImageColorization::getInstance();
        }
        imageColorizationPtr = ptr;
        QStringList file= fileName.split(".");
        videoName = file[0];
    }
}
void VideoColorization::setStartFrame(double d){
    startFrame = d;
    capPtr->set(CV_CAP_PROP_POS_FRAMES, startFrame);
}
Mat VideoColorization::doColorization(VideoColorizationMethod method,Mat coloredFirstFrame){
    switch(method){
        case FRAMESUB:
        frameSubColorization(coloredFirstFrame);
        return Mat();
        break;
    case MOTIONESTIMATE:
        motionEstimationColorization(coloredFirstFrame);
        return Mat();
        break;
    case MIXED:
        ColorizaitonWithFrameAndMotion(coloredFirstFrame);
        return Mat();
        break;
    case KEYFRAME:
        return keyFrameColorization(coloredFirstFrame);
    default:
        break;
    }
}
Mat VideoColorization::getFirstFrame(){
    Mat m ;
    (*capPtr)>>m;
    firstGrayFrame = m;
    return firstGrayFrame;
}
//使用帧差法
void VideoColorization::frameSubColorization(Mat coloredFirstFrame){
    using namespace std;
    long curFrameCnt = startFrame;
    long total = capPtr->get(CV_CAP_PROP_FRAME_COUNT);
    Mat curFrame;//使用YUV通道
    cvtColor(coloredFirstFrame,curFrame,CV_BGR2YUV);
    const int rows = curFrame.rows;
    const int cols = curFrame.cols;
    int i,j;
    Mat nextFrame;//为灰度图
    Mat outputFrame;//使用BGR通道
    Mat scribbleFrame(rows,cols,CV_8UC3);
    //
    VideoWriter outputVideo;
    outputVideo.open(videoName.append("_result.avi").toStdString(),(int)capPtr->get(CV_CAP_PROP_FOURCC),capPtr->get(CV_CAP_PROP_FPS)
                     ,Size((int)capPtr->get(CV_CAP_PROP_FRAME_WIDTH),(int)capPtr->get(CV_CAP_PROP_FRAME_HEIGHT)),true);
    //对每一帧进行处理


    capPtr->set(CV_CAP_PROP_POS_FRAMES, startFrame+1);
    for(;curFrameCnt<total-1;curFrameCnt++){
        cvtColor(curFrame,outputFrame,CV_YUV2BGR);
        outputVideo<<outputFrame;
        outputFrame.release();
//        QString s = QString::number(curFrameCnt).append(".jpg");
//        if(!imwrite(s.toStdString(),outputFrame))qDebug()<<"ok";
        (*capPtr)>>nextFrame;
        scribbleFrame= nextFrame.clone();
        for(i=0;i<rows;i++){
            for(j=0;j<cols;j++){
                if(scribbleFrame.ptr(i,j)[0]==curFrame.ptr(i,j)[0]){
                    scribbleFrame.ptr(i,j)[1]=curFrame.ptr(i,j)[1];
                    scribbleFrame.ptr(i,j)[2]=curFrame.ptr(i,j)[2];
                }
            }
        }
        //curFrame 为YUV空间
        curFrame = imageColorizationPtr->forVideoColorizationWithYUV(nextFrame,scribbleFrame);
        nextFrame.release();
        scribbleFrame.release();
    }
    imageColorizationPtr->clearLoadImages();
    QMessageBox::information(NULL,"提醒","视频彩色化完成");
    capPtr->release();
    outputVideo.release();
}
//使用运动估计
void VideoColorization::motionEstimationColorization(Mat coloredFirstFrame){
    const int rows = coloredFirstFrame.rows;
    const int cols = coloredFirstFrame.cols;
    long curFrameNum,totalFrameNum;
    totalFrameNum = capPtr->get(CV_CAP_PROP_FRAME_COUNT);
    curFrameNum = startFrame;
    Mat curFrame;
    cvtColor(coloredFirstFrame,curFrame,CV_BGR2YUV);//使用YUV通道
    Mat outputFrame;//使用BGR通道
    Mat nextFrame;//灰度
    Mat scribbleFrame(rows,cols,CV_8UC3);//有涂鸦的
    capPtr->set(CV_CAP_PROP_POS_FRAMES,curFrameNum+1);

    vector<Mat>* curVector = new vector<Mat>(3);
    vector<Mat>* scribbleVector= new vector<Mat>(3);
    //
    VideoWriter outputVideo;
    outputVideo.open(videoName.append("_result.avi").toStdString(),(int)capPtr->get(CV_CAP_PROP_FOURCC),capPtr->get(CV_CAP_PROP_FPS)
                     ,Size((int)capPtr->get(CV_CAP_PROP_FRAME_WIDTH),(int)capPtr->get(CV_CAP_PROP_FRAME_HEIGHT)),true);
    //
    Mat motionVector;
    for(;curFrameNum<totalFrameNum-1;curFrameNum++){
        //输出当前已经彩色化的帧
        cvtColor(curFrame,outputFrame,CV_YUV2BGR);
        outputVideo<<outputFrame;
        outputFrame.release();
//        QString s = QString::number(curFrameNum).append(".jpg");
//        if(!imwrite(s.toStdString(),outputFrame))qDebug()<<"ok";
        //下一帧
        (*capPtr)>>nextFrame;
        nextFrame.copyTo(scribbleFrame);
        split(scribbleFrame,*scribbleVector);
        split(curFrame,*curVector);
        getEstimateVector((*curVector)[0], (*scribbleVector)[0],motionVector);
        generateScribbleByMotionVector(motionVector,curFrame,scribbleFrame);
        curFrame.release();
        curFrame = imageColorizationPtr->forVideoColorizationWithYUV(nextFrame,scribbleFrame);
        nextFrame.release();
        scribbleFrame.release();
    }
    QMessageBox::information(NULL,"提醒","视频彩色化完成");
    delete curVector;
    delete scribbleVector;
    capPtr->release();
    outputVideo.release();
}
//使用混合彩色化
void VideoColorization::ColorizaitonWithFrameAndMotion(Mat coloredFirstFrame){
    const int rows = coloredFirstFrame.rows;
    const int cols = coloredFirstFrame.cols;
    long curFrameNum,totalFrameNum;
    totalFrameNum = capPtr->get(CV_CAP_PROP_FRAME_COUNT);
    curFrameNum = startFrame;
    Mat curFrame;
    cvtColor(coloredFirstFrame,curFrame,CV_BGR2YUV);//使用YUV通道
    Mat outputFrame;//使用BGR通道
    Mat nextFrame;//灰度
    Mat scribbleFrame(rows,cols,CV_8UC3);//有涂鸦的
    capPtr->set(CV_CAP_PROP_POS_FRAMES,curFrameNum+1);

    vector<Mat>* curVector = new vector<Mat>(3);
    vector<Mat>* scribbleVector= new vector<Mat>(3);
    //
    VideoWriter outputVideo;
    outputVideo.open(videoName.append("_result.avi").toStdString(),(int)capPtr->get(CV_CAP_PROP_FOURCC),capPtr->get(CV_CAP_PROP_FPS)
                     ,Size((int)capPtr->get(CV_CAP_PROP_FRAME_WIDTH),(int)capPtr->get(CV_CAP_PROP_FRAME_HEIGHT)),true);
    //
    Mat motionVector;
    int i ,j;
    for(;curFrameNum<totalFrameNum-1;curFrameNum++){
            cvtColor(curFrame,outputFrame,CV_YUV2BGR);
            outputVideo<<outputFrame;
            (*capPtr)>>nextFrame;
            split(curFrame,*curVector);
            scribbleFrame= nextFrame.clone();
            split(scribbleFrame,*scribbleVector);
            if(curFrameNum%2==1){
                for(i=0;i<rows;i++){
                    for(j=0;j<cols;j++){
                        if((*scribbleVector)[0].at<uchar>(i,j)==(*curVector)[0].at<uchar>(i,j)){
                            (*scribbleVector)[1].at<uchar>(i,j) = (*curVector)[1].at<uchar>(i,j);
                            (*scribbleVector)[2].at<uchar>(i,j) = (*curVector)[2].at<uchar>(i,j);
                        }
                    }
                }
                merge(*scribbleVector,scribbleFrame);
            }
            else{
                getEstimateVector((*curVector)[0], (*scribbleVector)[0],motionVector,30);
                generateScribbleByMotionVector(motionVector,curFrame,scribbleFrame,30);
            }
            //curFrame 为YUV空间
            curFrame = imageColorizationPtr->forVideoColorizationWithYUV(nextFrame,scribbleFrame);
        }
}
Mat VideoColorization::keyFrameColorization(Mat coloredFirstFrame){
    if(vWPtr==NULL){
        vWPtr = new VideoWriter();
        vWPtr->open(videoName.append("_result.avi").toStdString(),(int)capPtr->get(CV_CAP_PROP_FOURCC),capPtr->get(CV_CAP_PROP_FPS)
                             ,Size((int)capPtr->get(CV_CAP_PROP_FRAME_WIDTH),(int)capPtr->get(CV_CAP_PROP_FRAME_HEIGHT)),true);
        keyFrames.clear();
        if(INDEBUG){
            keyFrames.insert(0);
            keyFrames.insert(20);
            keyFrames.insert(40);
            keyFrames.insert(60);
            keyFrames.insert(120);
            keyFrames.insert(200);
            keyFrames.insert(250);
            keyFrames.insert(330);
        }else {
            extractKeyFrameUsingFCM();
        }
        keyFrames.insert(capPtr->get(CV_CAP_PROP_FRAME_COUNT)-1);
        qDebug()<<"关键帧提取完毕";
        frameCnt =0;
    }
    const int rows = coloredFirstFrame.rows;
    const int cols = coloredFirstFrame.cols;
    Mat curFrame;
    cvtColor(coloredFirstFrame,curFrame,CV_BGR2YUV);//使用YUV通道
    Mat outputFrame;//使用BGR通道
    Mat nextFrame;//灰度
    Mat scribbleFrame(rows,cols,CV_8UC3);//有涂鸦的
    capPtr->set(CV_CAP_PROP_POS_FRAMES,frameCnt+1);

    vector<Mat>* curVector = new vector<Mat>(3);
    vector<Mat>* scribbleVector= new vector<Mat>(3);
    //
    Mat motionVector;
    set<long>::iterator ite = keyFrames.begin();
    ite++;
    for(;frameCnt<*ite;frameCnt++){
        //输出当前已经彩色化的帧
//        qDebug()<<"here";
        cvtColor(curFrame,outputFrame,CV_YUV2BGR);
        (*vWPtr)<<outputFrame;
//        QString s = QString::number(frameCnt).append(".jpg");
//        if(!imwrite(s.toStdString(),outputFrame))qDebug()<<"ok";
        //下一帧
        (*capPtr)>>nextFrame;
        nextFrame.copyTo(scribbleFrame);
//        cvtColor(nextFrame,nextFrame,CV_BGR2GRAY);
        split(scribbleFrame,*scribbleVector);
        split(curFrame,*curVector);
        getEstimateVector((*curVector)[0], (*scribbleVector)[0],motionVector);
        generateScribbleByMotionVector(motionVector,curFrame,scribbleFrame);
        curFrame = imageColorizationPtr->forVideoColorizationWithYUV(nextFrame,scribbleFrame);
    }
    delete curVector;
    delete scribbleVector;
    keyFrames.erase(keyFrames.begin());
    if(*(keyFrames.begin())==capPtr->get(CV_CAP_PROP_FRAME_COUNT)-1){
        QMessageBox::information(NULL,"提醒","视频彩色化完成");
        vWPtr->release();
        delete vWPtr;
        vWPtr = NULL;
        return Mat();
    }else{
        (*capPtr)>>nextFrame;
        return nextFrame;
    }
}
//motionVector 双通道 curFrame scribbleFrame 三通道
 void VideoColorization::generateScribbleByMotionVector(const Mat& motionVector,const Mat& curFrame,Mat& scribbleFrame,int blockSize){
    const int rows = curFrame.rows;
    const int cols = curFrame.cols;
//    qDebug()<<"scribbleFrame.channels() "<<scribbleFrame.channels();
    int i,j,topLeftX,topLeftY,m,n;
    for(i=0;i<=rows-blockSize;i+=blockSize)
        for(j=0;j<=cols-blockSize;j+=blockSize){
            topLeftX = motionVector.ptr<short>(i,j)[0];
            topLeftY = motionVector.ptr<short>(i,j)[1];
            if(topLeftX<0&&topLeftY<0) continue;
            for(m=0;m<blockSize;m++)
                for(n=0;n<blockSize;n++){
                    scribbleFrame.ptr(m+topLeftX,n+topLeftY)[1] = curFrame.ptr(m+i,n+j)[1];
                    scribbleFrame.ptr(m+topLeftX,n+topLeftY)[2] = curFrame.ptr(m+i,n+j)[2];
                }
        }
 }
 // pre pro 是单通道 motionVector是双通道 保存每个块在pro中的对应块的左上顶点已经进行了噪声干扰抑制
 void VideoColorization::getEstimateVector(const Mat& pre,const Mat& pro,Mat& motionVector, int blockSize){
     using namespace std;
     const int rows = pre.rows;
     const int cols = pre.cols;
     if(!motionVector.data)motionVector.create(rows,cols,CV_16UC2);
     int i ,j,m,n,tR,tC;
     int tmp,nearRow,nearCol,dis,flag;
     int stepMax = blockSize/2;
     int stepSize = stepMax;
     multimap<float,int,less<float> > costs;
     ///
     Mat frameSub;
     absdiff(pro,pre,frameSub);//帧差
     //噪声干扰抑制参数
     int FD = sum(frameSub)[0];
     float blockMean,blockVariance;
     ///
     //range is [i,i+blockSize-1]
     for(i=0;i<=rows-blockSize;i+=blockSize){
         for(j=0;j<=cols-blockSize;j+=blockSize){
             //噪声干扰抑制
             blockMean =0;
             blockVariance =0;
             for(m=0;m<blockSize;m++)
                 for(n=0;n<blockSize;n++){
                     blockMean+=pre.at<uchar>(i+m,j+n);
                 }
             blockMean/=(blockSize*blockSize);
             for(m=0;m<blockSize;m++)
                 for(n=0;n<blockSize;n++){
                     blockVariance+=(pre.at<uchar>(i+m,j+n)-blockMean)*(pre.at<uchar>(i+m,j+n)-blockMean);
                 }
             blockVariance/=(blockSize*blockSize);
             if((blockVariance<T*T)&&(FD*FD<B*B*blockVariance)){
                     motionVector.ptr<short>(i,j)[0]=i;
                     motionVector.ptr<short>(i,j)[1]=j;
                     continue;
             }
             //
             costs.clear();
            //搜索最外层8个点
            stepSize = stepMax;
            for(m=-stepSize;m<=stepSize;m+=stepSize)
                for(n=-stepSize;n<=stepSize;n+=stepSize){
                    tR=i+m;tC=j+n;
                    if(tR<0||tR>rows-1||tC<0||tC>cols-1)continue;//防越界
//                    if (m==0&&n==0)continue;
                    costs.insert(pair<float,int>(costMADFun(pre,pro,i,j,tR,tC,blockSize),tR*cols+tC));
                }
           //搜索步长为1的8个点
            stepSize = 1;
            for(m=-stepSize;m<=stepSize;m+=stepSize)
                for(n=-stepSize;n<=stepSize;n+=stepSize){
                    tR=i+m;tC=j+n;
                    if(tR<0||tR>rows-1||tC<0||tC>cols-1)continue;//防越界
                    costs.insert(pair<float,int>(costMADFun(pre,pro,i,j,tR,tC,blockSize),tR*cols+tC));
                }
            //判断进入哪种模式
             tmp = costs.begin()->second;
             nearRow = tmp/cols;
            nearCol = tmp-nearRow*cols;
            dis = abs(nearRow-i)+abs(nearCol-j);
            flag = -1;
            if(dis==0) flag = -1;
            else if(dis<4) flag =0;
            else flag = 1;
            //第二步
            float threshod = 1e10;//sqrt(blockVariance)/4;
            switch (flag) {
            case -1:
                //严格运动估计
                if(costs.begin()->first>threshod){
                    motionVector.ptr<short>(i,j)[0] = -1;
                    motionVector.ptr<short>(i,j)[1] = -1;
                }else{
                    motionVector.ptr<short>(i,j)[0] =i;
                    motionVector.ptr<short>(i,j)[1] = j;
                }
                break;
            case 0:
                //搜索步长为1
                costs.clear();
                for(m=-stepSize;m<=stepSize;m+=stepSize)
                    for(n=-stepSize;n<=stepSize;n+=stepSize){
                        tR=nearRow+m;tC=nearCol+n;
                        if(tR<0||tR>rows-1||tC<0||tC>cols-1)continue;//防越界
                        costs.insert(pair<float,int>(costMADFun(pre,pro,i,j,tR,tC,blockSize),tR*cols+tC));
                    }
                if(costs.begin()->first>threshod){
                    motionVector.ptr<short>(i,j)[0] = -1;
                    motionVector.ptr<short>(i,j)[1] = -1;
                }else{
                    tmp = costs.begin()->second;
                    motionVector.ptr<short>(i,j)[0] = tmp/cols;
                    motionVector.ptr<short>(i,j)[1] = tmp-(tmp/cols)*cols;
                }
                break;
            case 1:
                //搜索最外层
                stepSize = stepMax/2;
                while(stepSize>=1){
                    costs.clear();
                    for(m=-stepSize;m<=stepSize;m+=stepSize)
                        for(n=-stepSize;n<=stepSize;n+=stepSize){
                            tR=nearRow+m;tC=nearCol+n;
                            if(tR<0||tR>rows-1||tC<0||tC>cols-1)continue;//防越界
                            costs.insert(pair<float,int>(costMADFun(pre,pro,i,j,tR,tC,blockSize),tR*cols+tC));
                        }
                    tmp = costs.begin()->second;
                    nearRow = tmp/cols;
                    nearCol = tmp-nearRow*cols;
                    stepSize/=2;
                }
                if(costs.begin()->first>threshod){
                    motionVector.ptr<short>(i,j)[0] = -1;
                    motionVector.ptr<short>(i,j)[1] = -1;
                }else{
                    motionVector.ptr<short>(i,j)[0] = nearRow;
                    motionVector.ptr<short>(i,j)[1] = nearCol;
                }
                break;
            default:
                break;
            }
        }
    }
}
float VideoColorization::costMADFun(const Mat&m1,const Mat& m2,int topLeftX1,int topLeftY1,int topLeftX2,int topLeftY2,int blockSize){
    int i,j;
    float sum = .0;
    for(i=0;i<blockSize;i++){
        for(j=0;j<blockSize;j++){
            sum+=abs(m1.at<uchar>(topLeftX1+i,topLeftY1+j)-m2.at<uchar>(topLeftX2+i,topLeftY2+j));
        }
    }
    return sum/(blockSize*blockSize);
}
// src 单通道 cntOfRB 将列分为若干段 cntOfCB 将行分为若干段 bins将灰度分为若干段 histVector 为产生的列向量
void VideoColorization::calHistVector(Mat& src,Mat& histVector,int cntOfRB,int cntOfCB,int bins){
    const int rows = src.rows;
    const int cols = src.cols;
    const int rSize = rows/cntOfRB;
    const int cSize = cols/cntOfRB;
    int i ,j,ii,jj;
    Mat mark;
    if(rows%cntOfRB!=0)cntOfRB+=1;
    if(cols%cntOfCB!=0)cntOfCB+=1;
    histVector=Mat::zeros(cntOfCB*cntOfRB*bins,1,CV_32F);
    float yranges[] = {0,256};
    const float* ranges []= {yranges};
    int channels =0;
    Mat hist;
    int cnt =0;
    for(i=0;i<rows;i+=rSize){
        for(j=0;j<cols;j+=cSize){
            mark=Mat::zeros(rows,cols,CV_8UC1);
            for(ii=i;ii<i+rSize&&ii<rows;ii++){
                for(jj=j;jj<j+cSize&&jj<cols;jj++){
                    mark.ptr<uchar>(ii,jj)[0]=1;
                }
            }
            calcHist(&src,1,&channels,mark,hist,1,&bins,ranges,true,false);
            int tmp = hist.rows;
            histVector.ptr<float>(cnt,0)[0]=hist.ptr<float>(0,0)[0];
            for(ii=1;ii<tmp;ii++){
                histVector.ptr<float>(cnt+ii,0)[0]=histVector.ptr<float>(cnt+ii-1,0)[0]+hist.ptr<float>(ii,0)[0];
            }
            cnt+=18;
        }
    }
}
//keyFrames 数组存放关键帧的序号 需要提前初始化
void VideoColorization::extractKeyFrameUsingFCM(){
    const long cntOfTotalFrame = capPtr->get(CV_CAP_PROP_FRAME_COUNT);
    //计算出每帧的统计值 以便加快聚类运算 帧数较多时不能使用此方法
    Mat* X= new Mat[cntOfTotalFrame];
    Mat tmpFrame;
    long cntOfCurFrame =0;
    capPtr->set(CV_CAP_PROP_POS_FRAMES,0);
    while(cntOfCurFrame<cntOfTotalFrame){
        Mat tmpHistVecor;
        (*capPtr)>>tmpFrame;
        cvtColor(tmpFrame,tmpFrame,CV_BGR2GRAY);
        calHistVector(tmpFrame,tmpHistVecor);
        X[cntOfCurFrame++] = tmpHistVecor;
    }
    const int rows = tmpFrame.rows;
    const int cols = tmpFrame.cols;
    keyFrames.insert(0);
    keyFrames.insert(cntOfTotalFrame/2);
    keyFrames.insert(cntOfTotalFrame-1);
    //聚类参数设置
    const int m =4;//m值
//    Mat u;//隶属矩阵
    int maxCntOfIte = 10;//最大循环次数
    //循环参量
    set<long>::iterator ite,tmpIte;
    int cntOfLoop = 0;
    double disOfKeyFrames ;
    bool breakFlag=true;
    while(cntOfLoop<maxCntOfIte){
        breakFlag = true;
        for(ite=keyFrames.begin();ite!=keyFrames.end();){
            tmpIte =ite;
            ite++;
            if(ite==keyFrames.end())break;
            disOfKeyFrames = norm(X[*tmpIte],X[*ite],NORM_L2)/(rows*cols);
//            cout<<"disOfKeyFrames"<<disOfKeyFrames<<endl;
//            getchar();
            if(disOfKeyFrames<0.015){
                //合并两个聚类中心
                keyFrames.erase(ite);
                ite = tmpIte;
                breakFlag &=false;
            } else if(disOfKeyFrames>0.03){
                //如果两帧相邻 不插入新的聚类中心
                if(abs(*ite-*tmpIte)==1){
                    breakFlag&=true;
                }
                //如果两帧不相邻 插入新的聚类中心
                else{
                    keyFrames.insert((*ite+*tmpIte)/2);
                    breakFlag&=false;
                }
            }else{

            }
        }

        if(!breakFlag){
//            float price;
            int cntOfLoop2 =0;
            vector<Mat>centerOfCluster;
            for(ite=keyFrames.begin();ite!=keyFrames.end();ite++){
                centerOfCluster.push_back(X[*ite]);
            }
            vector<Mat>::iterator ite2,tmpIte2;
            //迭代50次
            while(cntOfLoop2<10){
                //进行聚类计算
                //隶属度计算
                cntOfCurFrame =0;
                int cntOfCenter = centerOfCluster.size();
                float u[cntOfCenter][cntOfTotalFrame];
                ite2 = centerOfCluster.begin();
                vector<Mat> newCenterOfCluster;
                for(int i =0;i<cntOfCenter;i++){
                    float uSummary =0;
                    Mat mSummary=Mat::zeros(X[0].rows,X[0].cols,CV_32F);
                    for(cntOfCurFrame=0;cntOfCurFrame<cntOfTotalFrame;cntOfCurFrame++){
                        float dij = norm(*ite2,X[cntOfCurFrame],NORM_L2);
                        float summary=0;
                        for(tmpIte2=centerOfCluster.begin();tmpIte2!=centerOfCluster.end();tmpIte2++){
                            float dkj = norm(*tmpIte2,X[cntOfCurFrame],NORM_L2);
                            if(dkj==0) dkj=1;
                            summary+=pow(dij/dkj,2.0/(m-1));
                        }
                        if(summary==0) summary=1;
                        u[i][cntOfCurFrame] = 1/summary;
                        uSummary+=pow(1/summary,m);
                        mSummary=mSummary+X[cntOfCurFrame]*(pow(1/summary,m));
                    }
                    //得到新的聚类中心
                    newCenterOfCluster.push_back(mSummary/uSummary);
                    ite2++;
                }
                //计算价格函数
                //更新聚类中心
                centerOfCluster.clear();
                centerOfCluster=newCenterOfCluster;
                newCenterOfCluster.clear();
                cntOfLoop2++;
    }
             //得到关键帧的序号
            float di =1000;
            float tmp ;
            long cnt;
            for(ite2=centerOfCluster.begin();ite2!=centerOfCluster.end();ite2++){
                di=1000;
                cnt = 0;
                for(cntOfCurFrame=0;cntOfCurFrame<cntOfTotalFrame;cntOfCurFrame++){
                    tmp = norm(X[cntOfCurFrame],*ite2,NORM_L2);
                    if(tmp<di){
                            cnt =cntOfCurFrame;
                            tmp=di;
                    }
                }
                keyFrames.insert(cnt);
            }
        }
        else break;
        cntOfLoop++;
    }
    delete []X;
}
