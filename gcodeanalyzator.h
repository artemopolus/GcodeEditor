#ifndef GCODEANALYZATOR_H
#define GCODEANALYZATOR_H

#include <QString>
#include <QStringList>

QString const LayerChangeTag = "[on_change_layer]";
QString const CommentTag = ";";
QString const G1Tag = "G1";
QString const EndTag = "[on_end]";

bool isLayerChange(QString data);
bool isZChange(QString data, float * val);
bool isXYmove(QString data, double * X, double * Y);


#endif // GCODEANALYZATOR_H
