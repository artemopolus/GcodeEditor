#include "gcodeanalyzator.h"


bool isLayerChange(QString data, QString LayerChangeTag, QString EndTag, QString CommentTag)
{
   QStringList dataList = data.split(" ");
   for (int i = 0; i < dataList.length(); i++)
   {
       if ((dataList[i] == LayerChangeTag)||(dataList[i]) == (CommentTag + LayerChangeTag))
           return true;
       if ((dataList[i] == EndTag)||(dataList[i]) == (CommentTag + EndTag))
           return true;
   }
   return false;
}
bool isEndOfPrint(QString data, QString EndTag,QString CommentTag)
{
    QStringList dataList = data.split(" ");
    for (int i = 0; i < dataList.length(); i++)
    {
        if ((dataList[i] == EndTag)||(dataList[i]) == (CommentTag + EndTag))
            return true;
    }
    return false;
}

bool isZChange(QString data, float *val, QString G1Tag)
{
   QStringList dataList = data.split(" ");
   if (dataList[0] == G1Tag)
   {
       for (int i = 1; i < dataList.length(); i++)
       {
           if (dataList[i][0] == 'Z')
           {
               QString zval = dataList[i].remove(0,1);
               *val = zval.toFloat();
               return true;
           }
       }
   }
   return false;
}

bool isXYmove(QString data, double *X, double *Y, QString G1Tag)
{
    QStringList datamass = data.split(" ");
    if (datamass.length() > 2){
        if ((datamass[0] == G1Tag)&(datamass[1][0] == 'X')&(datamass[2][0] == 'Y'))
        {
            QString xval = datamass[1].remove(0,1);
            QString yval = datamass[2].remove(0,1);
            *X = xval.toDouble();
            *Y = yval.toDouble();
            return true;
        }
    }
    return false;
}
bool isXYmove2(QString data, double *X, double *Y, double *E, double *F, QString G1Tag)
{
    QStringList datamass = data.split(" ");
    if (datamass[0] == G1Tag)
    {
        for (int i = 1; i < datamass.length(); i++)
        {
            if (datamass[i][0] == 'X')
                *X = datamass[i].remove(0,1).toDouble();
            if (datamass[i][0] == 'Y')
                *Y = datamass[i].remove(0,1).toDouble();
            if (datamass[i][0] == 'E')
                *E = datamass[i].remove(0,1).toDouble();
            if (datamass[i][0] == 'F')
                *F = datamass[i].remove(0,1).toDouble();

        }
        return true;
    }
    return false;
}
bool isFanChange(QString data, double *S)
{
    QStringList datamass = data.split(" ");
    if (datamass[0] == "M106")
    {
        if (datamass[1][0] == 'S')
            *S = datamass[1].remove(0,1).toDouble();
        return true;
    }
    if (datamass[0] == "M107")
    {
        * S = 0;
        return true;
    }
    return false;
}
bool isTempExtrChange(QString data, double *T)
{
    QStringList datamass = data.split(" ");
    if (datamass[0] == "M104")
    {
        if (datamass[1][0] == 'S')
            *T = datamass[1].remove(0,1).toDouble();
        return true;
    }
    if (datamass[0] == "M109")
    {
        if (datamass[1][0] == 'S')
            *T = datamass[1].remove(0,1).toDouble();
        return true;
    }
    return false;
}
bool isTempTablChange(QString data, double *T)
{
    QStringList datamass = data.split(" ");
    if (datamass[0] == "M140")
    {
        if (datamass[1][0] == 'S')
            *T = datamass[1].remove(0,1).toDouble();
        return true;
    }
    if (datamass[0] == "M190")
    {
        if (datamass[1][0] == 'S')
            *T = datamass[1].remove(0,1).toDouble();
        return true;
    }
    return false;
}
bool isAccelChange(QString data, double *S)
{
    QStringList datamass = data.split(" ");
    if (datamass[0] == "M204")
    {
        if (datamass[1][0] == 'S')
            *S = datamass[1].remove(0,1).toDouble();
        return true;
    }
    return false;
}
gCodeParser::gCodeParser(QString path)
{
    this->filename = path;
}
gCodeParser::~gCodeParser()
{

}
void gCodeParser::readJsonFile()
{
//    //qDebug() << "path:" << this->filename;
//    QFile file(this->filename);
//    //QFile file("D:/SyncFolder/Workspace/Projects/GcodeEditor/build-GCodeEditor-Desktop_Qt_5_10_1_MinGW_32bit-Debug/debug/config.json");
//    //QFile file("config.json");
//    if (!file.exists())
//    {
//        QString str = "File " + this->filename + " is not exist";
//        qFatal(str.toLatin1().data());
//        file.close();
//        return;
//    }
//    if (!file.open(QIODevice::ReadOnly)) {
//        QString str = "Could not read " + this->filename + "!";
//        qFatal(str.toLatin1().data());
//        file.close();
//        return;
//    } else {
//        QTextStream in(&file);
//        QString str = in.readAll();
//        bool ok;
//        JsonObject json = QtJson::parse(str, ok).toMap();
//        if (!ok) {
//            qFatal("An error occurred during parsing");
//            return;
//        }
//        else
//            qDebug() << "Config data:";
//        this->ChangeLayerTag = json["change layer tag"].toString();
//        this->OnEndTag = json["end of printing"].toString();
//        this->CommentTag = json["comment indicator"].toString();
//        this->G1Tag = json["move indicator"].toString();
//        this->InsertStartTag = json["marker on start insert"].toString();
//        this->InsertEndTag = json["marker on end insert"].toString();
//        qDebug() << "change layer tag" << (this->ChangeLayerTag);
//        qDebug() << "end of printing" << (this->OnEndTag);
//        qDebug() << "comment indicator" << (this->CommentTag);
//        qDebug() << "move indicator" << (this->G1Tag);
//        file.close();
//    }

    QFile src;
    src.setFileName(this->filename);
    if (!src.open(QIODevice::ReadOnly))
    {
        return;
    }
    QByteArray saveData = src.readAll();
    QJsonDocument  loadDoc(QJsonDocument::fromJson(saveData));
    QJsonObject json = loadDoc.object();
    this->ChangeLayerTag = json["change layer tag"].toString();
    this->OnEndTag = json["end of printing"].toString();
    this->CommentTag = json["comment indicator"].toString();
    this->G1Tag = json["move indicator"].toString();
    this->InsertStartTag = json["marker on start insert"].toString();
    this->InsertEndTag = json["marker on end insert"].toString();
    qDebug() << "change layer tag" << (this->ChangeLayerTag);
    qDebug() << "end of printing" << (this->OnEndTag);
    qDebug() << "comment indicator" << (this->CommentTag);
    qDebug() << "move indicator" << (this->G1Tag);
    src.close();

}
void getTextDetailUp(QString * data, const double X, const double Y, const double dZ, const int minT, const int maxT, const double E)
{
    //    M107		; fan off
    * data = ";Move detail up start\n";
    * data += "M107\n";
    //    M109 R200 	; set extruder temperature and wait
    * data += "M109 R" + QString::number(maxT) + "\n";
    //    G91 		; relative
    * data += "G91\nG1 Z" + QString::number(dZ) +"\n";
    //    G1 Z5 		; up
    //    G90 		; absolute
    //    G28 X Y 	; home to X Y
    * data += "G90\nG28 X Y\n";

//    G1 X99 Y128	; go to first point
    * data += "G1 X" + QString::number(X) + " Y" + QString::number(Y) + "\n";
//    G91 			; relative
//    G1 Z-5 			; down
//    G90 			; absolute
    * data += "G91\nG1 Z" + QString::number(-dZ) +"\nG90\n";
//    G1 E15 F700		; connecting head to detail
//    M106			; fan on
//    M109 R50 		; wait untill colling
//    M107			; fan off
    * data += "G1 E" + QString::number(E);
    * data += " F700\nM106\nM109 R" + QString::number(minT) +"\nM107\n";
//    G91 			; relative
//    G1 Z5 			; turn up detail
//    G90 			; absolute
    * data += "G91\nG1 Z" + QString::number(dZ) +"\nG90\n";
    * data += ";Move detail up stop\n";
}
void getTextMoveDetail(QString *data, const double X, const double Y)
{
    * data = "G1";
    * data += " X" + QString::number(X);
    * data += " Y" + QString::number(Y);
    * data += "\n";
}
void getTextDownUP(QString *data, const double dZ, const int Twait)
{
//    M109 R200;		; heat head
//    G1 E45 F700		; disconnect detail with plastic
//    G1 Y100			; move bed to center
//    G1 Y250
//    M104 S0			; cool down

//    G91 			; relative
//    G1 Z-5 		; back to init z
//    G90 			; absolute
    * data = ";Detail down and up start\n";
    * data += "G91\n";
    * data += "G1 Z" + QString::number(-dZ) + "\n";
    * data += "G4 P" + QString::number(Twait) + "\n";
    * data += "G1 Z" + QString::number(dZ) + "\n";
    * data += "G90\n";
    * data += ";Detail down and up stop\n";
}
void getTextDownPut(QString *data, const double dZ, const int Theat)
{
    * data = ";Detail down and put start\n";
    * data += "G91\n";
    * data += "G1 Z" + QString::number(-dZ) + "\n";
    * data += "M109 R" + QString::number(Theat) + "\n";
    * data += "G1 Z" + QString::number(dZ) + "\n";
    * data += "G90\n";
    * data += ";Detail down and put stop\n";
}
void getTextDownDetach(QString *data, const double dZ, const int Theat, const float E)
{
    * data = ";Detail down and detach\n";
    * data += "G91\n";
    * data += "G1 Z" + QString::number(-dZ) + "\n";
    * data += "M109 R" + QString::number(Theat) + "\n";
    * data += "G1 E" + QString::number(E) + " F700\n";
    * data += "G1 Z" + QString::number(dZ) + "\n";
    * data += "G90\n";
    * data += ";Detail down and detach\n";
}
void getTextClear(QString *data, const int Theat, const float E)
{
    * data = ";Clear\n";
    * data += "M109 R" + QString::number(Theat) + "\n";
    * data += "G1 E" + QString::number(E) + " F700\n";
    * data += ";Clear\n";
}
QString getTextRemove(const double X1, const double Y1, const double X2, const double Y2, const double dZ)
{
    QString res;
    QString tmp;
    getTextDetailUp(&tmp, X1, Y1, dZ, 40, 200, 15);
    res += tmp;
    getTextMoveDetail(&tmp, X2, Y2);
    res +=tmp;
    getTextDownDetach(&tmp, dZ, 200, -10);
    res += tmp;
    return res;
}
QString getTextRemoveClear(const double X1, const double Y1, const double X2, const double Y2, const double dZ)
{
    QString res;
    QString tmp;
    getTextDetailUp(&tmp, X1, Y1, dZ, 40, 200, 15);
    res += tmp;
    getTextMoveDetail(&tmp, X2, Y2);
    res +=tmp;
    getTextClear(&tmp, 200, -10);
    res += tmp;
    res += "M104 S0\n";
    return res;
}
QString getTextPutTo(const double X1, const double Y1, const double X2, const double Y2, const double dZ)
{
    QString res;
    QString tmp;
    getTextDetailUp(&tmp, X1, Y1, dZ, 40, 200, 15);
    res += tmp;
    getTextMoveDetail(&tmp, X2, Y2);
    res += tmp;
    getTextDownPut(&tmp, dZ, 200);
    res += tmp;
    return res;
}
QString getTextPathPutTo(QVector<double> X, QVector<double> Y, const double dZ)
{
    if (X.size() < 2)
        return NULL;
    QString res;
    QString tmp;
    getTextDetailUp(&tmp,X[0],Y[0], dZ,40,200,15);
    res = tmp;
    for (int i = 1; i < (X.size()); i++)
    {
        getTextMoveDetail(&tmp,X[i], Y[i]);
        res += tmp;
    }
    getTextDownPut(&tmp, dZ, 200);
    res += tmp;
    return res;
}

/* сначала температура, выставить значение Е, в нуль, вернуть х у и z обязательно, вентилятор, высота, в последнюю точку */
QString getTextStartNotFrstLayer(const int Textr, const int Ttabl, const double X, const double Y, const double Z, const double E, const int FanV)
{
    QString n = "\n";
    QString t = "\t;";
    QString res = "M109 R" + QString::number(Textr) + "\t ;set extruder temperature" + n;
    res += "M190 R" + QString::number(Ttabl) + "\t ;set table temperature" + n;
    res += "G92 E" + QString::number(E) + "\t;set start E" + n;
    res += "G28 Y\t;go to null X" + n;
    res += "G28 X\t;go to null Y" + n;
    res += "G28 Z\t;go to null Z" + n;
    res += "M106 S" + QString::number(FanV) + t + "set fan speed" + n;
    res += "G1 Z" + QString::number(Z) + t + "go to start Z" + n;
    res += "G1 X" + QString::number(X) + " Y" + QString::number(Y) + t + "go to start point" + n;
    return res;
}
