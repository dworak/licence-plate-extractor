#include "detectedplate.h"
#include <QDebug>

using namespace cv;

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
    int w, h;
    w = plate.plateRect.width;
    h = plate.plateRect.height;
    Rect rect1 = Rect(plate.plateRect.x - w / 2, plate.plateRect.y - h / 2, 2 * w, 2 * h);
    w = plateRect.width;
    h = plateRect.height;
    Rect rect2 = Rect(plateRect.x - w / 2, plateRect.y - h / 2, 2 * w, 2 * h);
    //cv::Rect intersection = plate.plateRect & plateRect;
    cv::Rect intersection = rect1 & rect2;
//    qDebug() <<plate.toString()<<" "<<plate.lastFrame <<" " <<toString() << lastFrame << (abs(plate.lastFrame-lastFrame)<5) << (intersection.area() > 0);
    return abs(plate.lastFrame-lastFrame)<7 && intersection.area() > 0;
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

QString detectedplate::toString() const
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
