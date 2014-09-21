#include "methodselection.h"
#include "ui_methodselection.h"
MethodSelection::MethodSelection(ColorizationMethod cMethod,VideoColorizationMethod vMethod,QWidget* parent):
    QDialog(parent),
    ui(new Ui::MethodSelection)
{
    cm = cMethod;
    vm = vMethod;
    ui->setupUi(this);
    qBG1.addButton(ui->radioButton,0);
    qBG1.addButton(ui->radioButton_2,1);
    qBG1.addButton(ui->radioButton_6,2);
    qBG2.addButton(ui->radioButton_3,0);
    qBG2.addButton(ui->radioButton_4,1);
    qBG2.addButton(ui->radioButton_5,2);

    connect(ui->buttonBox,SIGNAL(accepted()),this,SLOT(actionOnButtonClicked()));
    if(cm==LEVIN)ui->radioButton->setChecked(true);
    else if (cm==GEODESIC)ui->radioButton_2->setChecked(true);
    else ui->radioButton_6->setChecked(true);

    if(vm==FRAMESUB)ui->radioButton_3->setChecked(true);
    else if (vm==MOTIONESTIMATE)ui->radioButton_4->setChecked(true);
    else ui->radioButton_5->setChecked(true);
}
MethodSelection::~MethodSelection()
{
    delete ui;
}
void MethodSelection::actionOnButtonClicked(){
    cm =qBG1.checkedId()==0?LEVIN:qBG1.checkedId()==1? GEODESIC:QUADTREEDECOMP;
    vm=qBG2.checkedId()==0?FRAMESUB:qBG2.checkedId()==1?MOTIONESTIMATE:KEYFRAME;
    emit methodConf(cm,vm);
}
