#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include "gcodeanalyzator.h"
#include <QVector>
#include <QMouseEvent>
#include "qcustomplot.h"
#include <QDebug>
#include "captureparams.h"

namespace Ui {
class MainWindow;
}

typedef struct
{
    int str;
    float Z;
    QString text;
    QVector<double> Xdata;
    QVector<double> Ydata;
    QVector<double> Edata;
    QVector<double> Fdata;
    QVector<double> FVdata;
    QVector<double> ACdata;
    double Textr;
    double Ttabl;
}oneLayer;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QString fileName;
    QFile srcFile;
    QVector<oneLayer> listLayer;
    int currentLayer;
    gCodeParser * ConfigParser;

    bool clickWas;
    double curX;
    double curY;
    QVector<double> msX;
    QVector<double> msY;

    double deltaMoveZ;

    CaptureParams * paramsDial = 0;
public slots:
    void setMW_MoveZ(double val);


private slots:
    void on_openFileButton_clicked();
    void CorrectSize();
    void resizeEvent(QResizeEvent *event);

    void on_layersList_currentRowChanged(int currentRow);

    void on_InsertButton_clicked();

    void on_SaveButton_clicked();

    void on_pushButton_clicked();

    void mousePressEvent(QMouseEvent* event);

    void keyPressEvent(QKeyEvent *event);

    void on_textToInsert_textChanged();

    void on_insertTextPlot_clicked();

    void on_capParamsButton_clicked();

    void on_captureComboBox_currentIndexChanged(int index);

    void on_saveInsert2fileButton_clicked();

    void on_setScale2TableButton_clicked();

    void on_cutButton_clicked();

private:
    Ui::MainWindow *ui;
    QCPCurve * path;
    double plateX;
    double plateY;
    int EndStartPart;
    int StartEndPart;
};

#endif // MAINWINDOW_H
