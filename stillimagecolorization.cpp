#include "stillimagecolorization.h"
#include "imageoperator.h"
#include <vector>
#include <math.h>
#include <QDebug>
#include <QTime>
#include <Eigen/Eigen>
#include <Eigen/Sparse>
#include <queue>
#include "blending.h"
using namespace std;
StillImageColorization* StillImageColorization::instance = NULL;
StillImageColorization::StillImageColorization(){
    grayScale = NULL;
    referenceScale = NULL;
}
void StillImageColorization::initSource(QString fileName){
        Mat m = imread(fileName.toStdString());
        imshow("orignalImage",m);
        //变成灰度图后转到三通道
        switch (m.channels()) {
        case 1:
            cvtColor(m,m,CV_GRAY2BGR);
            break;
        case 2://never happen
            break;
        case 3:
              cvtColor(m,m,CV_BGR2GRAY);
              cvtColor(m,m,CV_GRAY2BGR);
              break;
        case 4:
                cvtColor(m,m,CV_BGRA2GRAY);
                cvtColor(m,m,CV_GRAY2BGR);
                break;
        default:
            break;
        }
//        if(grayScale!=NULL) grayScale->release();
        grayScale = new Mat(m);
}
void StillImageColorization::initSource(Mat m){
        //变成灰度图后转到三通道
        switch (m.channels()) {
        case 1:
            cvtColor(m,m,CV_GRAY2BGR);
            break;
        case 2://never happen
            break;
        case 3:
              cvtColor(m,m,CV_BGR2GRAY);
              cvtColor(m,m,CV_GRAY2BGR);
              break;
        case 4:
                cvtColor(m,m,CV_BGRA2GRAY);
                cvtColor(m,m,CV_GRAY2BGR);
                break;
        default:
            break;
        }
//        if(grayScale!=NULL)  grayScale->release();
        grayScale = new Mat(m);
}
void StillImageColorization::initReference(QString fileName){
        Mat m  = imread(fileName.toStdString());
        switch (m.channels()) {
        case 1:
            cvtColor(m,m,CV_GRAY2BGR);
            break;
        case 2://never happen
            break;
        case 3:
              cvtColor(m,m,CV_RGB2BGR);
        case 4:
                cvtColor(m,m,CV_RGBA2BGR);
        default:
            break;
        }
//        if(referenceScale!=NULL) referenceScale->release();
        referenceScale = new Mat(m);
}
StillImageColorization* StillImageColorization::getInstance(){
    if(instance==NULL){
       instance = new  StillImageColorization();
    }
//    instance->clearLoadImages();
    return instance;
}

void StillImageColorization::initReference(const QImage& image){
//    if(referenceScale!=NULL) referenceScale->release();;
    referenceScale = new Mat(ImageOperator::qImage2Mat(image));
    imwrite("reference.bmp",*referenceScale);
}
void StillImageColorization::clearLoadImages(){
    if(grayScale!=NULL&&grayScale->data)grayScale->release();
    if(referenceScale!=NULL&&referenceScale->data)referenceScale->release();
}
///
Mat StillImageColorization::doColorization(ColorizationMethod method){
    Mat m ;
    switch (method) {
    case LEVIN:
        m = levinColorization();
        break;
    case GEODESIC:
        m = geodesicColorization(IMAGE);
        break;
    case QUADTREEDECOMP:
        m = quadTreeDecomposition();
       break;
    default:
        break;
    }
    return m;
}
//使用LEVIN方法
Mat StillImageColorization::levinColorization(){
    QTime time;
    time.start();
    using namespace Eigen;
    const int cols = grayScale->cols;
    const int rows = grayScale->rows;
    Mat yUVColor ;
    cvtColor(*referenceScale,yUVColor,CV_BGR2YUV);
        //转换使用Eigen库函数
    const MyMatrix grayImage(*grayScale);
    MyMatrix yUVImage(yUVColor);
    SparseMatrix<float> weightMatrix(cols*rows,cols*rows);
    vector<Triplet<float> > triplets;
    int iR ,iC,lR,lC,hR,hC,index,cntOfNeighbor;
    int i,j,tR,tC,indexOfCol,cnt;
    long sCnt = 0;//用于统计
    float storeNeighbor[9]={0};
    float tmp,summary;
    for (iR = 0;iR<rows;iR++){
                //边界矫正
        lR=iR-1;hR=iR+1;
        lR=lR<0?0:lR;
        hR=hR>rows-1?rows-1:hR;
        for(iC = 0;iC<cols;iC++){
            lC=iC-1;hC=iC+1;
            lC=lC<0?0:lC;
            hC=hC>cols-1?cols-1:hC;
            index = iC*rows+iR;//列堆积
                    //已经着色 权值为1
            triplets.push_back(Triplet<float>(index,index,1));
            if(grayScale->ptr(iR,iC)[0]!=referenceScale->ptr(iR,iC)[0]||grayScale->ptr(iR,iC)[1]!=referenceScale->ptr(iR,iC)[1]
                    ||grayScale->ptr(iR,iC)[2]!=referenceScale->ptr(iR,iC)[2]){
                    sCnt++;
                    continue;
            }
                    //没有着色 求权值
            (*yUVImage[0])(iR,iC)=0;//未涂色 则将对应的位置赋值为0
            (*yUVImage[1])(iR,iC)=0;//未涂色 则将对应的位置赋值为0
            (*yUVImage[2])(iR,iC)=0;//未涂色 则将对应的位置赋值为0
                        //计算该点领域的方差
            MatrixXf tmpMat = grayImage[0]->block(lR,lC,hR-lR+1,hC-lC+1);
            float var = (tmpMat.array()-tmpMat.mean()).array().square().sum()/(tmpMat.cols()*tmpMat.rows());//方差
            ACCURACYCONTROL(var);
             //以当前点为中心计算权重
            cntOfNeighbor = 0;
            for (i=-1;i<2;i++){
                tR = iR+i;
                if(tR<0)continue;
                else if(tR>=rows)break;
                for(j=-1;j<2;j++){
                    tC =iC+j;
                    if(tC<0)continue;
                    else if (tC>=cols)break;
                    tmp = exp(-pow((*(grayImage[0]))(iR,iC)-(*(grayImage[0]))(tR,tC),2)/(2*var));
                    ACCURACYCONTROL(tmp);
                    storeNeighbor[cntOfNeighbor++]= tmp;
                }
            }
            //权重归一化
            summary = 0;
            for(i =0;i<cntOfNeighbor;i++)summary+=storeNeighbor[i];
            summary-=1;
            //存入到权重矩阵中
            cnt = 0;
            for (i=-1;i<2;i++){
                for(j=-1;j<2;j++){
                    tR=iR+i;tC=iC+j;
                    if(tR<0||tR>=rows||tC<0||tC>=cols)continue;
                    if((i==0&&j==0)){
                        cnt+=1;
                        continue;
                    }
                    indexOfCol = index+i+j*rows;
                    float f = -storeNeighbor[cnt++]/summary;
                    triplets.push_back(Triplet<float>(index,indexOfCol,f));
                }
            }
        }
    }
    weightMatrix.setFromTriplets(triplets.begin(),triplets.end());
    VectorXf u(cols*rows),v(cols*rows),b;

        //u
    (*yUVImage[1]).resize(cols*rows,1);
    b = VectorXf((*yUVImage[1]));
    SparseLU <SparseMatrix<float> ,COLAMDOrdering<int> >  solver;
    solver.analyzePattern(weightMatrix);
    solver.factorize(weightMatrix);
    u= solver.solve(b);

    //    v
    (*yUVImage[2]).resize(cols*rows,1);
    b = VectorXf((*yUVImage[2]));
    v = solver.solve(b);
    MatrixXf U(u),V(v);
    U.resize(rows,cols);
    V.resize(rows,cols);
    *(grayImage[1]) = U;
    *(grayImage[2]) = V;
    Mat mat;
    cvtColor(grayImage.convertToMat(),mat,CV_YUV2BGR);
    if(SHOWINFO){
        qDebug()<<rows*cols;
        qDebug()<<sCnt;
        qDebug()<<"用时"<<time.elapsed()/1000.0<<"s";
        qDebug()<<"着色率"<<(float)sCnt*100.0/(rows*cols)<<"%";
    }
    return mat;
}
//使用二叉树分解的方法对Levin算法进行优化
Mat StillImageColorization::quadTreeDecomposition(){
    const int cols = grayScale->cols;
    const int rows = grayScale->rows;
    //随机取值进行阈值计算
    long cntOfBlock = (rows%4==0?rows/4:rows/4+1)*(cols%4==0?cols/4:cols/4+1);
    vector<long> randValue(cntOfBlock);
    for(long l =0;l<cntOfBlock;l++){
        randValue[l] = l;
    }
    srand(time(0));
    int tmp =0;
    int index,tR,tC,min,max;
    int diffOfBlock[cntOfSample];
    float sum=0,mean =0,variance=0;
    memset(diffOfBlock,0,sizeof(int)*cntOfSample);
    int i;
    for(i =0;i<cntOfSample&&randValue.begin()!=randValue.end();i++){
        tmp = rand()%randValue.size();
        index = randValue[tmp]*16;
        tR=index/cols;
        tC=index-cols*tR;
        min =256;
        max =-1;
        for(int ii=0;ii<4;ii++){
            for(int jj=0;jj<4;jj++){
                if(tR+ii>rows||tC+jj>cols)continue;
                if(min>grayScale->ptr(tR+ii,tC+jj)[0]) min=grayScale->ptr(tR+ii,tC+jj)[0];
                if(max<grayScale->ptr(tR+ii,tC+jj)[0]) max=grayScale->ptr(tR+ii,tC+jj)[0];
            }
        }
        diffOfBlock[i] = max-min;
        sum+=diffOfBlock[i];
        randValue.erase(randValue.begin()+tmp);
    }
    const int cntOfSampleR = i;
    //计算统计值
    mean = sum/cntOfSampleR;
    sum =0;
    for(i=0;i<cntOfSampleR;i++){
        sum+=(diffOfBlock[i]-mean)*(diffOfBlock[i]-mean);
    }
    variance = sqrt(sum/cntOfSampleR);
    const float threshodOfSplit = (mean+2*variance);
//以下进行图片的二叉分解
    list<NodeOfImage*>nodeStorage;
    //初始化
    nodeStorage.push_back( new NodeOfImage(0,0,rows/2,cols/2));
    nodeStorage.push_back(new NodeOfImage(0,cols/2,rows/2,cols-cols/2));
    nodeStorage.push_back(new NodeOfImage(rows/2,0,rows-rows/2,cols/2));
    nodeStorage.push_back(new NodeOfImage(rows/2,cols/2,rows-rows/2,cols-cols/2));
    list<NodeOfImage*>::iterator ite ,tmpIte;
    NodeOfImage* tmpPtrOfNode;
    for(ite=nodeStorage.begin();ite!=nodeStorage.end();){
        tmpPtrOfNode = *ite;
        min =256;
        max =-1;
        for(int ii=0;ii<tmpPtrOfNode->_height;ii++){
            for(int jj=0;jj<tmpPtrOfNode->_width;jj++){
                if(min>grayScale->ptr(tmpPtrOfNode->_r+ii,tmpPtrOfNode->_c+jj)[0])
                    min = grayScale->ptr(tmpPtrOfNode->_r+ii,tmpPtrOfNode->_c+jj)[0];
                if(max<grayScale->ptr(tmpPtrOfNode->_r+ii,tmpPtrOfNode->_c+jj)[0])
                    max = grayScale->ptr(tmpPtrOfNode->_r+ii,tmpPtrOfNode->_c+jj)[0];
            }
        }
        //是否继续进行分割
        if((max-min<threshodOfSplit&&tmpPtrOfNode->_height<=8&&tmpPtrOfNode->_width<=8)||
                (tmpPtrOfNode->_height==1&&tmpPtrOfNode->_width==1)){
            //不再进行分割
            ite++;
        }else{
            //进行分割
            if(tmpPtrOfNode->_width>1){
                if(tmpPtrOfNode->_height>1){
                    nodeStorage.push_back(new NodeOfImage(tmpPtrOfNode->_r,tmpPtrOfNode->_c,
                                                          tmpPtrOfNode->_height/2,tmpPtrOfNode->_width/2));
                    nodeStorage.push_back(new NodeOfImage(tmpPtrOfNode->_r,tmpPtrOfNode->_c+tmpPtrOfNode->_width/2,
                                                          tmpPtrOfNode->_height/2,tmpPtrOfNode->_width-tmpPtrOfNode->_width/2));
                    nodeStorage.push_back(new NodeOfImage(tmpPtrOfNode->_r+tmpPtrOfNode->_height/2,tmpPtrOfNode->_c,
                                                          tmpPtrOfNode->_height-tmpPtrOfNode->_height/2,tmpPtrOfNode->_width/2));
                    nodeStorage.push_back(new NodeOfImage(tmpPtrOfNode->_r+tmpPtrOfNode->_height/2,tmpPtrOfNode->_c+tmpPtrOfNode->_width/2,
                                                          tmpPtrOfNode->_height-tmpPtrOfNode->_height/2,tmpPtrOfNode->_width-tmpPtrOfNode->_width/2));
                }else{
                    nodeStorage.push_back(new NodeOfImage(tmpPtrOfNode->_r,tmpPtrOfNode->_c,
                                                          1,tmpPtrOfNode->_width/2));
                    nodeStorage.push_back(new NodeOfImage(tmpPtrOfNode->_r,tmpPtrOfNode->_c+tmpPtrOfNode->_width/2,
                                                          1,tmpPtrOfNode->_width-tmpPtrOfNode->_width/2));//?
                }
            }else{
                if(tmpPtrOfNode->_height>1){
                    nodeStorage.push_back(new NodeOfImage(tmpPtrOfNode->_r, tmpPtrOfNode->_c,
                                                          tmpPtrOfNode->_height/2,1));
                    nodeStorage.push_back(new NodeOfImage(tmpPtrOfNode->_r+tmpPtrOfNode->_height/2,tmpPtrOfNode->_c,
                                                          tmpPtrOfNode->_height-tmpPtrOfNode->_height/2,1));
                }else{
                }
            }
            tmpIte=ite;
            ite++;
            delete *tmpIte;
            nodeStorage.erase(tmpIte);
        }
    }
    //标记像素属于哪个块
    Mat label;
    label.create(rows,cols,CV_32S);
    int cntOfLabel =0;
    bool* isLabeled = new bool[rows*cols];
    memset(isLabeled,0,sizeof(bool)*rows*cols);
    for(ite=nodeStorage.begin();ite!=nodeStorage.end();ite++){
        tmpPtrOfNode = *ite;
        for(int ii=0;ii<tmpPtrOfNode->_height;ii++){
            for(int jj =0 ;jj<tmpPtrOfNode->_width;jj++){
                label.ptr<int>(tmpPtrOfNode->_r+ii,tmpPtrOfNode->_c+jj)[0] = cntOfLabel;
            }
        }
        cntOfLabel++;
    }
    //预处理一次 提高着色率
    for(int iR=0;iR<rows;iR++){
        for(int iC =0;iC<cols;iC++){
            //只有未扩展过的像素 且被人工上色的点才进行处理
            if((grayScale->ptr(iR,iC)[0]!=referenceScale->ptr(iR,iC)[0]||grayScale->ptr(iR,iC)[1]!=referenceScale->ptr(iR,iC)[1]
                    ||grayScale->ptr(iR,iC)[2]!=referenceScale->ptr(iR,iC)[2])&&!isLabeled[iR*cols+iC]){
                tmp =label.ptr<int>(iR,iC)[0];
                for(ite=nodeStorage.begin();tmp>0;ite++)tmp--;
                tmpPtrOfNode = *ite;
                for(int ii=0;ii<tmpPtrOfNode->_height;ii++){
                    for(int jj =0 ;jj<tmpPtrOfNode->_width;jj++){
                        referenceScale->ptr(tmpPtrOfNode->_r+ii,tmpPtrOfNode->_c+jj)[0] = referenceScale->ptr(iR,iC)[0];
                        referenceScale->ptr(tmpPtrOfNode->_r+ii,tmpPtrOfNode->_c+jj)[1] = referenceScale->ptr(iR,iC)[1];
                        referenceScale->ptr(tmpPtrOfNode->_r+ii,tmpPtrOfNode->_c+jj)[2] = referenceScale->ptr(iR,iC)[2];
                        isLabeled[(tmpPtrOfNode->_r+ii)*cols+tmpPtrOfNode->_c+jj] = true;
                    }
                }
            }
        }
    }
    //申请内存清除
    delete []isLabeled;
    for(ite=nodeStorage.begin();ite!=nodeStorage.end();){
        tmpIte = ite;
        ite++;
        delete * tmpIte;
        nodeStorage.erase(tmpIte);
    }
    imwrite("a.bmp",*referenceScale);
    return levinColorization();
}
Mat StillImageColorization::geodesicColorization(MediaType media){

    QTime time ;
    time.start();
    const int rows = grayScale->rows;
    const int cols = grayScale->cols;
    int i,j;
    long sCnt = 0;//用于统计
    vector<Mat>grayVector(grayScale->channels());
    vector<Mat>referVector(referenceScale->channels());
    split(*grayScale,grayVector);
    split(*referenceScale,referVector);
    for(i=0;i<rows;i++){
        for(j=0;j<cols;j++)
            if(referVector[0].at<uchar>(i,j)==grayVector[0].at<uchar>(i,j)&&referVector[1].at<uchar>(i,j)==grayVector[1].at<uchar>(i,j)
                    &&referVector[2].at<uchar>(i,j)==grayVector[2].at<uchar>(i,j)){
                referVector[0].at<uchar>(i,j)=0;
                referVector[1].at<uchar>(i,j) = 0;
                referVector[2].at<uchar>(i,j) = 0;
                sCnt++;
            }
    }
    Mat mark;
    merge(referVector,mark);
    Blending blending;
    Mat m;
    //使用image时 输入输出为bgr 使用video时 输入输出为YUV
    if(media==IMAGE)
     m=blending.Colorize(grayVector[0],mark);
    else m = blending.VideoColorize(grayVector[0],mark);
    if(SHOWINFO){
        qDebug()<<rows*cols;
        qDebug()<<rows*cols-sCnt;
        qDebug()<<"着色率 "<<(1-(float)sCnt/(rows*cols))*100<<"%";
        qDebug()<<"用时"<<time.elapsed()/1000.0<<"s";
    }
    return m;
}
//使用测地线 三通道
Mat StillImageColorization::forVideoColorizationWithYUV( Mat &gray, Mat &refer){
    grayScale = &gray;
    referenceScale = &refer;
    return geodesicColorization(VIDEO);
}
Mat StillImageColorization::forVideoColorizationWithBGR(Mat &gray, Mat &refer){
    grayScale = &gray;
    referenceScale = &refer;
    return geodesicColorization(IMAGE);
}
