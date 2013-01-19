#include "detectedplate.h"
#include <QDebug>

detectedplate::detectedplate()
{
}

detectedplate::detectedplate(cv::Rect rect, int frame)
{
 plateRect = rect;
 lastFrame=frame;

}

bool detectedplate::match(detectedplate plate)
{
//    qDebug() << plate.plateRect.y <<" "<< plateRect.y+plateRect.height;
    cv::Rect intersection = plate.plateRect & plateRect;
//    qDebug() <<plate.toString()<<" "<<plate.lastFrame <<" " <<toString() << lastFrame << (abs(plate.lastFrame-lastFrame)<5) << (intersection.area() > 0);
    return abs(plate.lastFrame-lastFrame)<5 && intersection.area() > 0;
}

void detectedplate::computeProbab()
{
    double sum=0;
    for (int i=0;i<characters.size();i++)
    {
        sum+=characters[i].second;
    }
    probab=sum/characters.size();
}

double detectedplate::getDivisionProbab()
{
    if (characters.length()>=2) return characters[2].second;
    else return 0;
}

QString detectedplate::toString()
{
    QString result;
    for (int i=0;i<characters.size();i++)
    {
        if (i==division)
        {
            result+=" ";
        }
        result+=characters[i].first;
    }
    return result;
}
