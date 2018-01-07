#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include "gcodeanalyzator.h"
#include <QVector>

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

private slots:
    void on_openFileButton_clicked();
    void CorrectSize();
    void resizeEvent(QResizeEvent *event);

    void on_layersList_currentRowChanged(int currentRow);

    void on_InsertButton_clicked();

    void on_SaveButton_clicked();

    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
