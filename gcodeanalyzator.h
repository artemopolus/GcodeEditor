#ifndef GCODEANALYZATOR_H
#define GCODEANALYZATOR_H

#include <QString>
#include <QStringList>

#include <QFile>
#include <QTextStream>
#include <QDebug>

#include "json.h"
using QtJson::JsonObject;
using QtJson::JsonArray;

//QString const LayerChangeTag = "[on_change_layer]";
//QString const CommentTag = ";";
//QString const G1Tag = "G1";
//QString const EndTag = "[on_end]";

bool isLayerChange(QString data, QString LayerChangeTag, QString EndTag, QString CommentTag);
bool isZChange(QString data, float * val, QString G1Tag);
bool isXYmove(QString data, double * X, double * Y, QString G1Tag);
void getTextDetailUp(QString * data, const double X, const double Y, const double dZ, const int minT, const int maxT, const double E);
void getTextMoveDetail(QString * data, const double X, const double Y);
void getTextDownUP(QString * data, const double dZ, const int Twait);
void getTextDownPut(QString * data, const double dZ, const int Theat);
void getTextDownDetach(QString * data, const double dZ, const int Theat, const float E);
QString getTextRemove(const double X1, const double Y1, const double X2, const double Y2, const double dZ);
QString getTextPutTo(const double X1, const double Y1, const double X2, const double Y2, const double dZ);



class gCodeParser
{
public:
    gCodeParser(QString path);
    ~gCodeParser();
    void readJsonFile();
    QString ChangeLayerTag;
    QString CommentTag;
    QString G1Tag;
    QString OnEndTag;
    QString InsertStartTag;
    QString InsertEndTag;
private:
    QString filename;
};


#endif // GCODEANALYZATOR_H
