#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
#include "methodselection.h"
Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    connectActions();
    windowSetting();
    imageColor =NULL;
    videoColor = NULL;
    cm=GEODESIC;
    vm=KEYFRAME;
}

Widget::~Widget()
{
    delete ui;
}
void Widget::connectActions(){
    connect(ui->cLinBut_OpenFile,SIGNAL(clicked()),this,SLOT(actionOpenGrayFile()));
    connect(ui->cLinBut_SelectScribble,SIGNAL(clicked()),this,SLOT(actionOpenScribbleFile()));
    connect(ui->cLinBut_SaveFile,SIGNAL(clicked()),this,SLOT(actionSaveFile()));
    connect(ui->cLinBut_SelectCorlor,SIGNAL(clicked()),this,SLOT(actionSelectColor()));
    connect(ui->spinBox_LineWidth,SIGNAL(valueChanged(int)),this,SLOT(actionSelectLineWidth(int)));
    connect(ui->cLinBut_ClearScribble,SIGNAL(clicked()),this,SLOT(actionClearScribble()));
    connect(ui->cLinkBut_MethodSelection,SIGNAL(clicked()),this,SLOT(actionMethodSelection()));
    connect(ui->label_ShowImage,SIGNAL(startScribble()),this,SLOT(setDoColorization()));
    connect(ui->cLinBut_DoColorization,SIGNAL(clicked()),this,SLOT(actionDoImageColorization()));
    connect(ui->cLinBut_VideoColorization,SIGNAL(clicked()),this,SLOT(acitonDoVideoColorization()));//need change
}
void Widget::windowSetting(){
    mediaType = IMAGE;
    ////////////////
    ui->cLinBut_DoColorization->setEnabled(false);
    ui->cLinBut_VideoColorization->setEnabled(false);
    QPalette p;
    p.setColor(QPalette::Window,Qt::red);
     ui->label_ShowColor->setPalette(p);
     ui->label_ShowColor->show();
     ///////////////////////////
     QSize qSize = ui->label_ShowLineWidth->size();
     qSize.setHeight(5);
     ui->label_ShowLineWidth->resize(qSize);
     QPalette pe;
     pe.setColor(QPalette::Window,Qt::black);
     ui->label_ShowLineWidth->setPalette(pe);
     ui->label_ShowLineWidth->show();
     //////////////////////////////
     ui->label_ShowImage->setPenWidth(ui->spinBox_LineWidth->value());
}
// slots
void Widget::setDoColorization(){
    if(mediaType==IMAGE)
        ui->cLinBut_DoColorization->setEnabled(true);
    else
        ui->cLinBut_VideoColorization->setEnabled(true);
}
void Widget::actionOpenGrayFile(){
    QString  fileName = QFileDialog::getOpenFileName(this,tr("选择源灰度图像或视频"),"../","图像文件(*.jpg *.bmp)\n 视频文件(*.avi)");
    if(!fileName.isEmpty()){
        //装载数据到矩阵中
        if(fileName.contains(".avi")){
            videoColor =VideoColorization::getInstance();
            videoColorizationProWork(fileName);
        }
        else {

            imageColor = StillImageColorization::getInstance();

            imageColorizationProWork(fileName);

        }
        ui->cLinBut_DoColorization->setEnabled(false);
        ui->cLinBut_VideoColorization->setEnabled(false);
    }
}
void Widget::actionOpenScribbleFile(){
    QString  fileName = QFileDialog::getOpenFileName(this,tr("选择着色图像"),"../","图像文件( *.bmp)");
    if(!fileName.isEmpty()){
        //装载数据到矩阵中
        ImageOperator::LoadImage(imageColor,fileName,SCRIBBLE);
        ui->label_ShowImage->setImage(ImageOperator::mat2QImage(*(imageColor->getRefer())));
        ui->scrollArea->update();
        ui->cLinBut_DoColorization->setEnabled(true);
    }
}
void Widget::actionSaveFile(){
    if(imageColor->getGray()==NULL){
        return ;
    }
    QString fileName = QFileDialog::getSaveFileName(this,tr("选择存储路径(不要包含中文)"),"../","图像文件( *.bmp)");
    if(fileName.isEmpty()){
        return ;
    }
    ui->label_ShowImage->getImage().save(fileName,"BMP",100);
}
void Widget::actionSelectColor(){
    QColor qColor = QColorDialog::getColor();
    ui->label_ShowImage->setPenColor(qColor);
    QPalette p;
    p.setColor(QPalette::Window,qColor);
     ui->label_ShowColor->setPalette(p);
}
void Widget::actionSelectLineWidth(int lineWidth){
    QSize qSize = ui->label_ShowLineWidth->size();
    qSize.setHeight(lineWidth);
    ui->label_ShowLineWidth->resize(qSize);
    ui->label_ShowImage->setPenWidth(lineWidth);
    ui->label_ShowImage->show();
}
void Widget::actionDoImageColorization(){
    imageColor->initReference(ui->label_ShowImage->getImage());

    /////////
    Mat m = imageColor->doColorization(cm);
//    Mat m = imageColor->doColorization(LEVIN);
    ui->label_ShowImage->setImage(ImageOperator::mat2QImage(m));
}
void Widget::acitonDoVideoColorization(){

    imageColor->initReference(ui->label_ShowImage->getImage());
    Mat m = imageColor->doColorization(GEODESIC);
    ui->label_ShowImage->setImage(ImageOperator::mat2QImage(m));

//    videoColor->doColorization(MIXED,m);
//    videoColor->doColorization(MOTIONESTIMATE,m);
//    videoColor->doColorization(FRAMESUB,m);
    Mat m2 =videoColor->doColorization(vm,m);
    if(m2.data){
        ui->label_ShowImage->setImage(ImageOperator::mat2QImage(m2));
        imageColor->initSource(m2);
    }
}
void Widget::actionClearScribble(){
    if(imageColor==NULL||imageColor->getGray()==NULL)return ;
    ui->label_ShowImage->setImage(ImageOperator::mat2QImage(imageColor,GRAY));
    //不可以进行彩色化
    ui->cLinBut_DoColorization->setEnabled(false);
    ui->cLinBut_VideoColorization->setEnabled(false);
}
void Widget::actionUpdateArea(Mat m){
    if(m.data){
        ui->label_ShowImage->setImage(ImageOperator::mat2QImage(m));
        //不可以进行彩色化
        ui->cLinBut_DoColorization->setEnabled(false);
        ui->cLinBut_VideoColorization->setEnabled(false);
    }
}
void Widget::actionMethodSelection(){
    MethodSelection* m = new MethodSelection(cm,vm);
    m->setModal(true);
    m->setAttribute(Qt::WA_DeleteOnClose);
    m->show();
     //other connction
    connect(m,SIGNAL(methodConf(ColorizationMethod,VideoColorizationMethod)),this,
            SLOT(actionSetMethodConf(ColorizationMethod,VideoColorizationMethod)));
}
void Widget::actionSetMethodConf(ColorizationMethod cMethod,VideoColorizationMethod vMethod){
    cm=cMethod;
    vm=vMethod;
}
//pro work for colorizaiton
void Widget::imageColorizationProWork(QString fileName){
    mediaType = IMAGE;
    ImageOperator::LoadImage(imageColor,fileName,GRAY);    
    ui->label_ShowImage->setImage(ImageOperator::mat2QImage(*(imageColor->getGray())));
    ui->scrollArea->update();
    ui->cLinBut_DoColorization->setEnabled(false);
}
void Widget::imageColorizationProWork(Mat mat ){
    imageColor->initSource(mat);
    ui->label_ShowImage->setImage(ImageOperator::mat2QImage(mat));
    ui->scrollArea->update();
    ui->cLinBut_VideoColorization->setEnabled(false);
}
//视频处理的输入为YUV的空间
void Widget::videoColorizationProWork(QString fileName){
    //video operator here
    mediaType = VIDEO;
    Mat firstFrame;
    videoColor->init(fileName,imageColor);
    videoColor->setStartFrame(0);
    firstFrame = videoColor->getFirstFrame();
    imageColorizationProWork(firstFrame);
    ui->cLinBut_VideoColorization->setEnabled(false);
}
