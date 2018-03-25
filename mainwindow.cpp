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
    this->clickWas = false;

    this->ui->captureComboBox->addItem(QString("Нет захвата"));
    this->ui->captureComboBox->addItem(QString("Пол. мыши"));
    this->ui->captureComboBox->addItem(QString("Переставление"));
    this->ui->captureComboBox->addItem(QString("Снятие"));
    this->ui->captureComboBox->setCurrentIndex(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (!((this->ui->captureOnBox->isChecked())||(this->ui->capture4detachBox->isChecked())))
        return;
    /* Регистрируем отправку событий нажатия клавиши */
    QCustomPlot* customPlot = qobject_cast<QCustomPlot*>(sender());
//    int ax = event->pos().x();
//    int ay = event->pos().y();
//    /* Проверяем объект */
//    int x1 = this->ui->plotData->x();
//    int x2 = x1 + this->ui->plotData->width();
//    int y1 = this->ui->plotData->y();
//    int y2 = y1 + this->ui->plotData->height();
    //if (!((ax > x1)&&(ax < x2)&&(ay > y1)&&(ay < y2)))
    //    return;
    /* переводим в координаты */
    if (!customPlot)
        return;
    double x = customPlot->xAxis->pixelToCoord(event->pos().x());
    double y = customPlot->yAxis->pixelToCoord(event->pos().y());
    /* Генерируем строку сохранения */
    QString str = "G1 X" + QString::number(x) + " Y" + QString::number(y);
    this->ui->statusLabel->setText(str);
    /* Сохраняем последние */
    if (!this->clickWas)
        this->clickWas = true;
    else
    {
        if (this->ui->capture4detachBox->isChecked())
        {
            QString res = getTextRemove(this->curX,this->curY,x,y,5);
            this->ui->textToInsert->append(res);
        }
        if (this->ui->capture2putBox->isChecked())
        {
            QString res = getTextPutTo(this->curX, this->curY,x,y,10);
            this->ui->textToInsert->append(res);
        }
    }
    this->curX = x;
    this->curY = y;
    /* Добавляем данные в поле, если есть галка */
    if (this->ui->captureOnBox->isChecked())
    {
        this->ui->textToInsert->append(str);
    }

}

void MainWindow::on_openFileButton_clicked()
{
    /* Отключаем кнопки, если они активны */
    if (this->ui->InsertButton->isEnabled())
        this->ui->InsertButton->setEnabled(false);
    if(this->ui->SaveButton->isEnabled())
        this->ui->SaveButton->setEnabled(false);
    /* Открываем файл и парсим данные о имени и типе файла */
    this->fileName = QFileDialog::getOpenFileName(this,"Open g-code","",tr("*.gco, *.gcode"));
    QStringList fileNameMass = this->fileName.split("/");
    this->ui->statusLabel->setText(fileNameMass[fileNameMass.length()-1]);
    this->srcFile.setFileName(this->fileName);
    /* Проверка корректности файла */
    if(!this->srcFile.open(QIODevice::ReadOnly))
    {
        this->ui->statusLabel->setText(QString("can't open file"));
        return;
    }
    QTextStream input;
    input.setDevice(&this->srcFile);
    //this->ui->statusLabel->setText(input.readLine());
    /*
     * Инициируем переменные для поиска:
     * Состояние простого автомата
     * Перменная для хранения номера строки
     * Перменная для временного хранения высоты слоя
     * Перменная для хранения высоты слоя
     * Счетчик слоев
     * Переменные хранения
     * Массив траектории печати
     * Объект для хранения данных о слоях
     */
    int state = 0;
    int str = -1;
    float Ztmp = -1;
    float Z;
    int layerCount = 0;
    double Xval, Yval;
    QVector<double> Xmass, Ymass;
    listLayer.clear();
    /* Старт обработки текста */
    for (int i =0; !input.atEnd(); i++)
    {
        QString dataStr = input.readLine();
        /*
         * Список состояний:
         * Инициирование и обнуление
         * Поиск смены высоты
         * Уточнение высоты или, если конец слоя, запись данных
        */
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
        /* Запись массива траектори */
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
    /* Установка размера окна рисования */
    newSize.setX(5);
    int shiftY  = 150;
    newSize.setY(shiftY);
    int width = floor(event->size().width()/2);
    int heigth = floor(event->size().height() - shiftY);
    newSize.setWidth(width - 10);
    newSize.setHeight(width - 10);
    this->ui->plotData->setGeometry(newSize);
    //ui->tabWidget->setGeometry(newSize);

    /* Установка размера списка слоев */
    width = floor(event->size().width()/4) - 20;
    newSize.setX(floor(event->size().width()/2) + 5);
    newSize.setY(shiftY);
    newSize.setWidth(width);
    newSize.setHeight(heigth - 50);
    this->ui->layersList->setGeometry(newSize);

    /* Установка размера бокса текста для вставки */
    newSize.setX(floor(event->size().width()*3/4) + 10);
    newSize.setY(shiftY);
    newSize.setWidth(width);
    newSize.setHeight(heigth - 50);
    this->ui->textToInsert->setGeometry(newSize);
}

void MainWindow::on_layersList_currentRowChanged(int currentRow)
{
    this->ui->statusLabel->setText(QString::number(currentRow));
    /* включение неактивных кнопок */
    if (!this->ui->InsertButton->isEnabled())
        this->ui->InsertButton->setEnabled(true);
    if(!this->ui->SaveButton->isEnabled())
        this->ui->SaveButton->setEnabled(true);
    this->currentLayer = currentRow;
    /* проверка наличия уже вставленного текста */
    if (!this->listLayer[currentRow].text.isNull())
        this->ui->textToInsert->setText(this->listLayer[currentRow].text);
    else
        this->ui->textToInsert->clear();
    /* надо очищать? */
    if (!this->ui->dontUpdateScreenBox->isChecked())
        this->ui->plotData->clearPlottables();
    /* загрузка данных слоя */
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

    /* Рисуем траекторию движения по слою */
    QCPCurve * newCurve = new QCPCurve(ui->plotData->xAxis,ui->plotData->yAxis);
    QVector<double> t;
    for (int i = 0; i < X.length(); i++) t.push_back(i);
    newCurve->setData(t, X, Y);

    /* Надо ли масштабировать?*/
    if (!this->ui->fixScaleBox->isChecked())
    {
        /* Масштабирем по центру без искажений */
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
    /* рисуем все! */
    this->ui->plotData->replot();
}

void MainWindow::on_InsertButton_clicked()
{
    /* Вставка текста в информацию о слое для дальнейшего сохранения */
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

    this->ui->layersList->removeItemWidget(this->ui->layersList->takeItem(this->currentLayer));
    this->ui->layersList->insertItem((this->currentLayer - 1), str);
}

void MainWindow::on_SaveButton_clicked()
{
    /* сохранение данных */
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

    /* вставка всех данных */
    for (int i =0; !input.atEnd(); i++)
    {
        QString str = input.readLine();
        if (layerCounter < listLayer.length())
        {
            if (this->listLayer[layerCounter].str == i)
            {
                if (!this->listLayer[layerCounter].text.isEmpty())
                {
                    output << "; "  << this->ConfigParser->InsertStartTag   <<  " \n";
                    output << this->listLayer[layerCounter].text;
                    output << "\n;" << this->ConfigParser->InsertEndTag     <<  " \n";
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
    //if(this->path)
        //delete this->path;
    this->path = new QCPCurve(ui->plotData->xAxis,ui->plotData->yAxis);

    this->path->setData(tt,tx,ty);
    this->path->setPen(QPen(Qt::red));
    //this->path->setData(tt, tx, ty);
    this->ui->plotData->replot();
}
