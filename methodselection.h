#ifndef METHODSELECTION_H
#define METHODSELECTION_H

#include <QDialog>
#include <QButtonGroup>
#include "global.h"
namespace Ui {
class MethodSelection;
}

class MethodSelection : public QDialog
{
    Q_OBJECT

public:
    MethodSelection(ColorizationMethod cMethod = GEODESIC,VideoColorizationMethod vMethod = FRAMESUB,QWidget* parent =0);
    ~MethodSelection();
    ColorizationMethod cm;
    VideoColorizationMethod vm;

signals:
    void methodConf(ColorizationMethod,VideoColorizationMethod);
private slots:
    void actionOnButtonClicked();
private:
    Ui::MethodSelection *ui;
    QButtonGroup qBG1;
    QButtonGroup qBG2;
};

#endif // METHODSELECTION_H
