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
gCodeParser::gCodeParser(QString path)
{
    this->filename = path;
}
gCodeParser::~gCodeParser()
{

}
void gCodeParser::readJsonFile()
{
    //qDebug() << "path:" << this->filename;
    QFile file(this->filename);
    //QFile file("D:/SyncFolder/Workspace/Projects/GcodeEditor/build-GCodeEditor-Desktop_Qt_5_10_1_MinGW_32bit-Debug/debug/config.json");
    //QFile file("config.json");
    if (!file.exists())
    {
        QString str = "File " + this->filename + " is not exist";
        qFatal(str.toLatin1().data());
        file.close();
        return;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        QString str = "Could not read " + this->filename + "!";
        qFatal(str.toLatin1().data());
        file.close();
        return;
    } else {
        QTextStream in(&file);
        QString str = in.readAll();
        bool ok;
        JsonObject json = QtJson::parse(str, ok).toMap();
        if (!ok) {
            qFatal("An error occurred during parsing");
            return;
        }
        else
            qDebug() << "Config data:";
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
        file.close();
    }
}
