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
    this->deltaMoveZ = 5;

    this->ui->captureComboBox->addItem(QString("Нет захвата"));
    this->ui->captureComboBox->addItem(QString("Пол. мыши"));
    this->ui->captureComboBox->addItem(QString("Переставление"));
    this->ui->captureComboBox->addItem(QString("Снятие"));
    this->ui->captureComboBox->setCurrentIndex(0);

    this->ui->postText2saveEdit->setText("drop");

    this->plateX = 200;
    this->plateY = 300;

    /* рисуем выбранную точку */

    this->ui->plotData->addGraph();
    this->ui->plotData->graph(2)->setPen(QColor(100,100,100,255));
    this->ui->plotData->graph(2)->setLineStyle(QCPGraph::lsNone);
    this->ui->plotData->graph(2)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCrossCircle,10));

    QSplitter *splitter = this->ui->splitter_plot;
    QList<int> sizes;
    sizes.append(splitter->widget(0)->minimumSize().width());
    sizes.append(splitter->widget(1)->minimumSize().width());
    splitter->setSizes(sizes);
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->type() == QEvent::KeyPress){
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if ((keyEvent->key() == Qt::Key_Return)||(keyEvent->key() == Qt::Key_Enter)){
            qDebug("Enter Key Pressed...");
            if (msX.size()>1)
            {
                QString res = getTextPathPutTo(msX,msY,this->deltaMoveZ);
                this->ui->textToInsert->append(res);
                res = "";
                for(int i = 0; i < msX.size(); i++)
                {
                    res += "[" + QString::number(msX[i]) + "," + QString::number(msY[i]) + "];";
                }
                this->ui->statusLabel->setText(res);
                msX.clear();
                msY.clear();
            }
            return;
        }
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (this->ui->captureComboBox->currentIndex() == 0)
        return;
    /* Регистрируем отправку событий нажатия клавиши */
    QCustomPlot* customPlot = qobject_cast<QCustomPlot*>(sender());
    /* переводим в координаты */
    if (!customPlot)
        return;

    double x = customPlot->xAxis->pixelToCoord(event->pos().x());
    double y = customPlot->yAxis->pixelToCoord(event->pos().y());
    QVector<double> Xpt, Ypt;
    Xpt.push_back(x); Ypt.push_back(y);
    int count = this->ui->plotData->graphCount();
    if (count)
    {
        this->ui->plotData->graph(2)->setData(Xpt,Ypt);
        this->ui->plotData->replot();
    }

    /* Генерируем строку сохранения */
    QString str = "G1 X" + QString::number(x) + " Y" + QString::number(y);
    this->ui->statusLabel->setText(str);

    /* Сохраняем последние */
    if (!this->clickWas)
        this->clickWas = true;
    else
    {
        if (this->ui->captureComboBox->currentIndex() == 3)
        {
            QString res = getTextRemoveClear(this->curX,this->curY,x,y,this->deltaMoveZ);
            this->ui->textToInsert->append(res);
        }
//        if (this->ui->captureComboBox->currentIndex() == 2)
//        {
//            QString res = getTextPutTo(this->curX, this->curY,x,y,this->deltaMoveZ);
//            this->ui->textToInsert->append(res);
//        }
        this->clickWas = false;
    }
    if (this->ui->captureComboBox->currentIndex() == 2)
    {
        msX.push_back(x);
        msY.push_back(y);
    }

    this->curX = x;
    this->curY = y;
    /* Добавляем данные в поле, если есть галка */
    if (this->ui->captureComboBox->currentIndex() == 1)
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
    double Xval = qQNaN();
    double Yval = qQNaN();
    double Eval = qQNaN();
    double Fval = qQNaN();
    double FVval = qQNaN();
    double ACval = qQNaN();
    double TextrVal = qQNaN();
    double TtablVal = qQNaN();
    QVector<double> Xmass, Ymass, Emass, Fmass, FVmass,ACmass;
    listLayer.clear();
    /* Старт обработки текста */
    for (int i =0; !input.atEnd(); i++)
    {
        QString dataStr = input.readLine();
        isFanChange(dataStr, &FVval);
        isAccelChange(dataStr, &ACval);
        isTempExtrChange(dataStr,&TextrVal);
        isTempTablChange(dataStr,&TtablVal);
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
                Emass.clear();
                Fmass.clear();
                FVmass.clear();
                ACmass.clear();
                this->EndStartPart = i;
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
                    obj.Edata = Emass;
                    obj.Fdata = Fmass;
                    obj.FVdata = FVmass;
                    obj.ACdata = ACmass;
                    obj.Textr = TextrVal;
                    obj.Ttabl = TtablVal;
                    this->listLayer.push_back(obj);
                    this->ui->layersList->addItem(QString::number(layerCount) + " " + QString::number(obj.str) + " " + QString::number(obj.Z));
                    layerCount++;

                    state = 1;
                    str = i;
                    Ztmp = -1;
                    Xmass.clear();
                    Ymass.clear();
                    Emass.clear();
                    Fmass.clear();
                    FVmass.clear();
                    ACmass.clear();
                    if(isEndOfPrint(dataStr,ConfigParser->OnEndTag,ConfigParser->CommentTag))
                    {
                        this->StartEndPart = i;
                    }
                }
            }
            break;
        }
        /* Запись массива траектори */
        if (state != 0)
        {
//            if (isXYmove(dataStr, &Xval, &Yval,ConfigParser->G1Tag))
//            {
//                Xmass.push_back(Xval);
//                Ymass.push_back(Yval);
//            }
            isXYmove2(dataStr,&Xval,&Yval,&Eval,&Fval,ConfigParser->G1Tag);
            isFanChange(dataStr, &FVval);
            isAccelChange(dataStr, &ACval);
            isTempExtrChange(dataStr,&TextrVal);
            isTempTablChange(dataStr,&TtablVal);
            Xmass.push_back(Xval);
            Ymass.push_back(Yval);
            Emass.push_back(Eval);
            Fmass.push_back(Fval);
            FVmass.push_back(FVval);
            ACmass.push_back(ACval);
        }
    }
    this->srcFile.close();
}
void MainWindow::CorrectSize()
{

}
//void MainWindow::resizeEvent(QResizeEvent *event)
//{
//    QRect newSize;
//    /* Установка размера окна рисования */
//    newSize.setX(5);
//    int shiftY  = 150;
//    newSize.setY(shiftY);
//    int width = floor(event->size().width()/2);
//    int heigth = floor(event->size().height() - shiftY);
//    newSize.setWidth(width - 10);
//    newSize.setHeight(width - 10);
//    this->ui->plotData->setGeometry(newSize);
//    //ui->tabWidget->setGeometry(newSize);

//    /* Установка размера списка слоев */
//    width = floor(event->size().width()/4) - 20;
//    newSize.setX(floor(event->size().width()/2) + 5);
//    newSize.setY(shiftY);
//    newSize.setWidth(width);
//    newSize.setHeight(heigth - 50);
//    this->ui->layersList->setGeometry(newSize);

//    /* Установка размера бокса текста для вставки */
//    newSize.setX(floor(event->size().width()*3/4) + 10);
//    newSize.setY(shiftY);
//    newSize.setWidth(width);
//    newSize.setHeight(heigth - 50);
//    this->ui->textToInsert->setGeometry(newSize);
//}

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

void MainWindow::on_capParamsButton_clicked()
{
    paramsDial = new CaptureParams(this);
    connect( paramsDial, SIGNAL(sendMoveZ(double)), this, SLOT(setMW_MoveZ(double)));
    paramsDial->setModal(true);
    paramsDial->putOffsetZ(this->deltaMoveZ);
    paramsDial->exec();
}
void MainWindow::setMW_MoveZ(double val)
{
    //qDebug()<< "text";
    this->deltaMoveZ = val;
    this->ui->statusLabel->setText(QString::number(this->deltaMoveZ));
}

void MainWindow::on_captureComboBox_currentIndexChanged(int index)
{
    this->clickWas = false;
}

void MainWindow::on_saveInsert2fileButton_clicked()
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
    filepath += nameextmass[0] + "_" +this->ui->postText2saveEdit->text() + "." + nameextmass[1];
    this->ui->statusLabel->setText(filepath);
//    this->srcFile.open(QIODevice::ReadOnly);
//    QTextStream input;
//    input.setDevice(&this->srcFile);

    QFile resFile;
    resFile.setFileName(filepath);
    resFile.open(QIODevice::WriteOnly);
    QTextStream output;
    output.setDevice(&resFile);
    output << this->ui->textToInsert->toPlainText();
    //this->srcFile.close();
    resFile.close();
}

void MainWindow::on_setScale2TableButton_clicked()
{
    this->ui->plotData->xAxis->setRange(0, this->plateX);
    this->ui->plotData->yAxis->setRange(0, this->plateY);
    this->ui->plotData->replot();
}

void MainWindow::on_cutButton_clicked()
{
    int ptr2strend = this->ui->layersList->currentRow();
    if (ptr2strend < 0) return;

    int ptr2end = this->listLayer[ptr2strend].str;
    /* сохранение данных */
    QStringList filenamemass = this->fileName.split("/");
    QString filepath = "";
    for (int i = 0; i < (filenamemass.length()-1); i++)
    {
        filepath += filenamemass[i] + "/";
    }
    QString nameext = filenamemass[filenamemass.length()-1];
    QStringList nameextmass = nameext.split(".");
    QString filepath2 = filepath + nameextmass[0] + "_cut1." + nameextmass[1];
    filepath += nameextmass[0] + "_cut0." + nameextmass[1];

    this->ui->statusLabel->setText(filepath + " " + filepath2);
    this->srcFile.open(QIODevice::ReadOnly);
    QTextStream input;
    input.setDevice(&this->srcFile);
    int layerCounter = 0;

    /* первый файл */
    QFile resFile;
    resFile.setFileName(filepath);
    resFile.open(QIODevice::WriteOnly);
    QTextStream output;
    output.setDevice(&resFile);
    /* второй файл */
    QFile resFile2;
    resFile2.setFileName(filepath2);
    resFile2.open(QIODevice::WriteOnly);
    QTextStream output2;
    output2.setDevice(&resFile2);

    /* вставка всех данных */

    /* начало второго файла генерируется на основе последнего слоя первого */
    int Textr = (int) this->listLayer[ptr2strend].Textr;
    int Ttabl = (int) this->listLayer[ptr2strend].Ttabl;
    int last = this->listLayer[ptr2strend - 1].Xdata.size();
    double X = this->listLayer[ptr2strend - 1].Xdata[last - 1];
    //check size plz
    double Y = this->listLayer[ptr2strend - 1].Ydata[last - 1];
    double Z = this->listLayer[ptr2strend].Z;
    double E = this->listLayer[ptr2strend - 1].Edata[this->listLayer[ptr2strend - 1].Edata.size() - 1];
    double FanV = this->listLayer[ptr2strend - 1].FVdata[this->listLayer[ptr2strend - 1].FVdata.size() - 1];
    output2 << getTextStartNotFrstLayer(Textr,Ttabl,X,Y,Z,E, FanV) << "\n";

    for (int i =0; (!input.atEnd()); i++)
    {
        QString str = input.readLine();
        if (layerCounter < listLayer.length())
        {
            if (this->listLayer[layerCounter].str == i)
            {
                if (!this->listLayer[layerCounter].text.isEmpty())
                {
                    if (i < ptr2end)
                    {
                        output << "; "  << this->ConfigParser->InsertStartTag   <<  " \n";
                        output << this->listLayer[layerCounter].text;
                        output << "\n;" << this->ConfigParser->InsertEndTag     <<  " \n";
                    }
                    if (i >= ptr2end)
                    {
                        output2 << "; "  << this->ConfigParser->InsertStartTag   <<  " \n";
                        output2 << this->listLayer[layerCounter].text;
                        output2 << "\n;" << this->ConfigParser->InsertEndTag     <<  " \n";
                    }
                }
                layerCounter++;
            }
        }
        if ((i < ptr2end)||(i > this->StartEndPart))
        {
            output << str + "\n";
        }
        if (i >= ptr2end)
        {
            output2 << str + "\n";
        }
    }

    this->srcFile.close();
    resFile.close();
    resFile2.close();



}
