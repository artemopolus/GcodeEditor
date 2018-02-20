#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->ui->statusLabel->setText("PLZ, choose fkn file");
    this->ui->plotData->addGraph();
    this->ui->plotData->addGraph();
    this->path = new QCPCurve(ui->plotData->xAxis,ui->plotData->yAxis);
    /*
     * Привязываем событие клика мыши
     */
    connect(this->ui->plotData, &QCustomPlot::mousePress,this,&MainWindow::mousePressEvent);
    /*
    * Загрузка конфигурации для парсинга
    */
    ConfigParser = new gCodeParser("config.json");
    ConfigParser->readJsonFile();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    QCustomPlot* customPlot = qobject_cast<QCustomPlot*>(sender());
        double x = customPlot->xAxis->pixelToCoord(event->pos().x());
        double y = customPlot->yAxis->pixelToCoord(event->pos().y());
    QString str = "G1 X" + QString::number(x) + " Y" + QString::number(y);
    this->ui->statusLabel->setText(str);
    if (this->ui->captureOnBox->isChecked())
    {
        this->ui->textToInsert->append(str);
    }
}

void MainWindow::on_openFileButton_clicked()
{
    this->fileName = QFileDialog::getOpenFileName(this,"Open g-code","",tr("*.gco, *.gcode"));
    QStringList fileNameMass = this->fileName.split("/");
    this->ui->statusLabel->setText(fileNameMass[fileNameMass.length()-1]);
    this->srcFile.setFileName(this->fileName);
    this->srcFile.open(QIODevice::ReadOnly);
    QTextStream input;
    input.setDevice(&this->srcFile);
    //this->ui->statusLabel->setText(input.readLine());
    int state = 0;
    int str = -1;
    float Ztmp = -1;
    float Z;
    int layerCount = 0;
    double Xval, Yval;
    QVector<double> Xmass, Ymass;
    listLayer.clear();
    for (int i =0; !input.atEnd(); i++)
    {
        QString dataStr = input.readLine();
        switch (state)
        {
        case 0:
            if (isLayerChange(dataStr,ConfigParser->ChangeLayerTag,ConfigParser->OnEndTag,ConfigParser->CommentTag))
            {
                state = 1;
                str = i;
                Ztmp = -1;
                Xmass.clear();
                Ymass.clear();
            }
            break;
        case 1:
            if (isZChange(dataStr, &Ztmp,ConfigParser->G1Tag))
            {
                Z = Ztmp;
                state = 2;
            }
            break;
        case 2:
            if (isZChange(dataStr, &Ztmp,ConfigParser->G1Tag))
            {
                if (Z > Ztmp)
                {
                    Z = Ztmp;
                }
            }
            else{
                if (isLayerChange(dataStr,ConfigParser->ChangeLayerTag,ConfigParser->OnEndTag,ConfigParser->CommentTag))
                {
                    state = 0;
                    oneLayer obj;
                    obj.str = str+1;
                    obj.Z = Z;
                    obj.Xdata = Xmass;
                    obj.Ydata = Ymass;
                    this->listLayer.push_back(obj);
                    this->ui->layersList->addItem(QString::number(layerCount) + " " + QString::number(obj.str) + " " + QString::number(obj.Z));
                    layerCount++;

                    state = 1;
                    str = i;
                    Ztmp = -1;
                    Xmass.clear();
                    Ymass.clear();
                }
            }
            break;
        }
        if (state != 0)
        {
            if (isXYmove(dataStr, &Xval, &Yval,ConfigParser->G1Tag))
            {
                Xmass.push_back(Xval);
                Ymass.push_back(Yval);
            }
        }
    }
    this->srcFile.close();
}
void MainWindow::CorrectSize()
{

}
void MainWindow::resizeEvent(QResizeEvent *event)
{
    QRect newSize;
    newSize.setX(5);
    int shiftY  = 150;
    newSize.setY(shiftY);
    int width = floor(event->size().width()/2);
    int heigth = floor(event->size().height() - shiftY);
    newSize.setWidth(width - 10);
    newSize.setHeight(width - 10);
    this->ui->plotData->setGeometry(newSize);
    //ui->tabWidget->setGeometry(newSize);

    width = floor(event->size().width()/4) - 20;
    newSize.setX(floor(event->size().width()/2) + 5);
    newSize.setY(shiftY);
    newSize.setWidth(width);
    newSize.setHeight(heigth - 50);
    this->ui->layersList->setGeometry(newSize);

    newSize.setX(floor(event->size().width()*3/4) + 10);
    newSize.setY(shiftY);
    newSize.setWidth(width);
    newSize.setHeight(heigth - 50);
    this->ui->textToInsert->setGeometry(newSize);
}

void MainWindow::on_layersList_currentRowChanged(int currentRow)
{
    this->ui->statusLabel->setText(QString::number(currentRow));
    if (!this->ui->InsertButton->isEnabled())
        this->ui->InsertButton->setEnabled(true);
    if(!this->ui->SaveButton->isEnabled())
        this->ui->SaveButton->setEnabled(true);
    this->currentLayer = currentRow;
    if (!this->listLayer[currentRow].text.isNull())
        this->ui->textToInsert->setText(this->listLayer[currentRow].text);
    else
        this->ui->textToInsert->clear();
    if (!this->ui->dontUpdateScreenBox->isChecked())
        this->ui->plotData->clearPlottables();
    QVector<double> X = this->listLayer[currentRow].Xdata;
    QVector<double> Y = this->listLayer[currentRow].Ydata;
    /*
     * QCPCurve *newCurve = new QCPCurve(customPlot->xAxis, customPlot->yAxis);
which registers it with the QCustomPlot instance of the passed axes.
Note that this QCustomPlot instance takes ownership of the plottable,
so do not delete it manually but use QCustomPlot::removePlottable() instead.
The newly created plottable can be modified, e.g.:

  newCurve->setName("Fermat's Spiral");
  newCurve->setData(tData, xData, yData);
     * */
    //this->ui->plotData->addGraph();
    //this->ui->plotData->graph(0)->setData(X,Y);
    QCPCurve * newCurve = new QCPCurve(ui->plotData->xAxis,ui->plotData->yAxis);
    QVector<double> t;
    for (int i = 0; i < X.length(); i++) t.push_back(i);
    newCurve->setData(t, X, Y);
    //this->ui->plotData->rescaleAxes();
    //ui->Acc1widget->yAxis->setRange(minY, maxY);
    if (!this->ui->fixScaleBox->isChecked())
    {
        double minX = X[0]; double maxX = X[0];
        double minY = Y[0]; double maxY = Y[0];
        for (int i = 0; i < X.length(); i++)
        {
            if (X[i]<minX)  minX = X[i];
            if (X[i]>maxX)  maxX = X[i];
            if (Y[i]<minY)  minY = Y[i];
            if (Y[i]>maxY)  maxY = Y[i];
        }
        int shift = 10;
        if ((maxX - minX)<(maxY - minY))
        {

            int meanMaxMinY = floor((maxY + minY)/2);
            int halfMax_Min = floor((maxY - minY)/2);
            int meanMaxMinX = floor((maxX + minX)/2);
            this->ui->plotData->xAxis->setRange((meanMaxMinX - halfMax_Min - shift),(meanMaxMinX + halfMax_Min + shift));
            this->ui->plotData->yAxis->setRange((meanMaxMinY - halfMax_Min - shift),(meanMaxMinY + halfMax_Min + shift));
        }
        else
        {
            int meanMaxMinX = floor((minX + maxX)/2);
            int halfMax_Min = floor((maxX - minX)/2);
            int meanMaxMinY = floor((minY + maxY)/2);
            this->ui->plotData->xAxis->setRange((meanMaxMinX - halfMax_Min - shift),(meanMaxMinX + halfMax_Min + shift));
            this->ui->plotData->yAxis->setRange((meanMaxMinY - halfMax_Min - shift),(meanMaxMinY + halfMax_Min + shift));
        }
    }
    this->ui->plotData->replot();
}

void MainWindow::on_InsertButton_clicked()
{
    QString text = this->ui->textToInsert->toPlainText();
    this->listLayer[this->currentLayer].text = text;
    QString str = this->ui->layersList->item(this->currentLayer)->text();
    QStringList strlst = str.split(" ");
    if (strlst.length() > 3)
    {
        if (text.isEmpty())
        {
            str = "";
            for(unsigned int i = 0; i < 3; i++)
                str += strlst[i] + " ";
        }
    }
    else
    {
        if (!text.isEmpty())
        {
            str += " edit";
        }
    }
    //QString str = this->ui->layersList->item(this->currentLayer)->text() + " edit";

    this->ui->layersList->removeItemWidget(this->ui->layersList->takeItem(this->currentLayer));
    this->ui->layersList->insertItem((this->currentLayer - 1), str);
}

void MainWindow::on_SaveButton_clicked()
{
    QStringList filenamemass = this->fileName.split("/");
    QString filepath = "";
    for (int i = 0; i < (filenamemass.length()-1); i++)
    {
        filepath += filenamemass[i] + "/";
    }
    QString nameext = filenamemass[filenamemass.length()-1];
    QStringList nameextmass = nameext.split(".");
    filepath += nameextmass[0] + "_changed." + nameextmass[1];
    this->ui->statusLabel->setText(filepath);
    this->srcFile.open(QIODevice::ReadOnly);
    QTextStream input;
    input.setDevice(&this->srcFile);
    int layerCounter = 0;

    QFile resFile;
    resFile.setFileName(filepath);
    resFile.open(QIODevice::WriteOnly);
    QTextStream output;
    output.setDevice(&resFile);

    for (int i =0; !input.atEnd(); i++)
    {
        QString str = input.readLine();
        if (layerCounter < listLayer.length())
        {
            if (this->listLayer[layerCounter].str == i)
            {
                if (!this->listLayer[layerCounter].text.isEmpty())
                {
                    output << "; [start_insert] \n";
                    output << this->listLayer[layerCounter].text;
                    output << "\n; [end_insert] \n";
                }
                layerCounter++;
            }
        }
        output << str + "\n";
    }

    this->srcFile.close();
    resFile.close();
}

void MainWindow::on_pushButton_clicked()
{
    QString str = "G91\nG1 Z5\nG1 E-3 F1000\nM300 S5000 P280\nG4 S1\nM300 S5000 P280\nG90\n G1 Y10 X10 F1000\nM117 Push Me\nM0\nG91\nG1 E3 F1000\nG92 E0\nG90\n";
    this->ui->textToInsert->setText(str);
}

void MainWindow::on_textToInsert_textChanged()
{

}



void MainWindow::on_insertTextPlot_clicked()
{
    QStringList rowList = this->ui->textToInsert->toPlainText().split("\n");
    double x,y,t=0;
    QVector<double> tx,ty,tt;
    for(int i = 0; i < rowList.length(); i++)
    {
        if (isXYmove(rowList[i],&x,&y,ConfigParser->G1Tag))
        {
            tx.push_back(x);
            ty.push_back(y);
            tt.push_back(t);
            t++;
        }
    }
    QCPCurve * m = new QCPCurve(ui->plotData->xAxis,ui->plotData->yAxis);
    m->setData(tt,tx,ty);
    m->setPen(QPen(Qt::red));
    //this->path->setData(tt, tx, ty);
    this->ui->plotData->replot();
}
