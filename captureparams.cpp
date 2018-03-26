#include "captureparams.h"
#include "ui_captureparams.h"

CaptureParams::CaptureParams(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CaptureParams)
{
    ui->setupUi(this);
    //this->ui->offsetZspinBox->setValue(parent->deltaMoveZ);
}

CaptureParams::~CaptureParams()
{
    delete ui;
}

void CaptureParams::on_OKButton_clicked()
{
    emit sendMoveZ(this->ui->offsetZspinBox->value());
    this->close();
}

void CaptureParams::putOffsetZ(double dZ)
{
    this->ui->offsetZspinBox->setValue(dZ);
}
