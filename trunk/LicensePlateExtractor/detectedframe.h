#ifndef DETECTEDFRAME_H
#define DETECTEDFRAME_H

#include <QList>
#include <detectedplate.h>

class detectedframe
{
public:
    detectedframe();
    void AddPlate(detectedplate plate);
    detectedplate GetPlate(cv::Rect rect, int frame);
    QList<detectedplate> plates;
};

#endif // DETECTEDFRAME_H
