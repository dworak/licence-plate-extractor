#ifndef UTILS_H
#define UTILS_H

#include <QImage>

#include <opencv2/core/core.hpp>

class Utils
{
public:
    static QImage* IplImage2QImage(IplImage *iplImg);
    static cv::Mat getMatchFilterKernel(int m, int n, double variance, double A, double B);
    static int makeOdd(int number);
};

#endif // UTILS_H
