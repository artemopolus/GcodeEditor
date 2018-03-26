#ifndef CAPTUREPARAMS_H
#define CAPTUREPARAMS_H

#include <QDialog>
//#include "mainwindow.h"

namespace Ui {
class CaptureParams;
}

class CaptureParams : public QDialog
{
    Q_OBJECT

public:
    explicit CaptureParams(QWidget *parent = 0);
    ~CaptureParams();

private:
    Ui::CaptureParams *ui;

signals:
    void sendMoveZ(double);
private slots:
    void on_OKButton_clicked();
};

#endif // CAPTUREPARAMS_H
