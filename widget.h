#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QFileDialog>
#include <QColorDialog>
#include <QMessageBox>
#include "imageoperator.h"
#include "stillimagecolorization.h"
#include "videocolorization.h"


namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private:
    Ui::Widget *ui;
    StillImageColorization* imageColor;
    VideoColorization* videoColor;
    MediaType mediaType;
    ColorizationMethod cm;
    VideoColorizationMethod vm;
private slots:
    void actionOpenGrayFile();
    void actionOpenScribbleFile();
    void actionSaveFile();
    void actionSelectColor();
    void actionSelectLineWidth(int lineWidth);
    void actionMethodSelection();
    void actionSetMethodConf(ColorizationMethod cMethod,VideoColorizationMethod vMethod);
    //
    void actionDoImageColorization();
    void acitonDoVideoColorization();
    void actionClearScribble();

    void actionUpdateArea(Mat m);
    void setDoColorization();
    //function
private :
    void connectActions();
    void windowSetting();
    void imageColorizationProWork(QString fileName);//预处理
    void imageColorizationProWork(Mat mat );//预处理
    void videoColorizationProWork(QString fileName);//预处理
};

#endif // WIDGET_H
