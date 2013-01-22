#ifndef DETECTEDPLATE_H
#define DETECTEDPLATE_H

#include <QList>
#include <QPair>
#include <QString>
#include <opencv2/opencv.hpp>

class detectedplate
{
public:
    detectedplate();
    detectedplate(cv::Rect rect,int frame);
    QString toString() const;
    bool match(detectedplate plate);
    double getDivisionProbab();
    cv::Rect plateRect;
    QList<QPair<char,double> > characters;
    int lastFrame;
    int division;
    void computeProbab();
    double probab;
};

#endif // DETECTEDPLATE_H
