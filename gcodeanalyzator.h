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
private:
    QString filename;
};


#endif // GCODEANALYZATOR_H
