#include "gcodeanalyzator.h"


bool isLayerChange(QString data)
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

bool isZChange(QString data, float *val)
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

bool isXYmove(QString data, double *X, double *Y)
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
