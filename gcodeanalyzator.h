#ifndef GCODEANALYZATOR_H
#define GCODEANALYZATOR_H

#include <QString>
#include <QStringList>

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QVector>

//#include "json.h"
//using QtJson::JsonObject;
//using QtJson::JsonArray;

//QString const LayerChangeTag = "[on_change_layer]";
//QString const CommentTag = ";";
//QString const G1Tag = "G1";
//QString const EndTag = "[on_end]";

#include <QJsonObject>
#include <QJsonDocument>

bool isLayerChange(QString data, QString LayerChangeTag, QString EndTag, QString CommentTag);
bool isEndOfPrint(QString data, QString EndTag, QString CommentTag);
bool isTempExtrChange(QString data, double * T);
bool isTempTablChange(QString data, double * T);

bool isZChange(QString data, float * val, QString G1Tag);
bool isXYmove(QString data, double * X, double * Y, QString G1Tag);
bool isXYmove2(QString data, double *X, double * Y, double * E, double * F, QString G1Tag);

bool isFanChange(QString data, double * S);
bool isAccelChange(QString data, double * S);

void getTextDetailUp(QString * data, const double X, const double Y, const double dZ, const int minT, const int maxT, const double E);
void getTextMoveDetail(QString * data, const double X, const double Y);
void getTextDownUP(QString * data, const double dZ, const int Twait);
void getTextDownPut(QString * data, const double dZ, const int Theat);
void getTextDownDetach(QString * data, const double dZ, const int Theat, const float E);
void getTextClear(QString * data, const int Theat, const float E);

QString getTextRemove(const double X1, const double Y1, const double X2, const double Y2, const double dZ);
QString getTextPutTo(const double X1, const double Y1, const double X2, const double Y2, const double dZ);
QString getTextPathPutTo(QVector<double> X, QVector<double> Y, const double dZ);

QString getTextStartNotFrstLayer(const int Textr, const int Ttabl, const double X, const double Y, const double Z, const double E, const int FanV);

QString getTextRemoveClear(const double X1, const double Y1, const double X2, const double Y2, const double dZ);


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
