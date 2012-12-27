#ifndef UTILS_H
#define UTILS_H

#include <QImage>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

class Utils
{
public:
    static QImage* IplImage2QImage(IplImage *iplImg);
    static QImage Mat2QImage(cv::Mat &matImg);
    static cv::Mat getMatchFilterKernel(int m, int n, double sd, double A, double B);
    static int makeOdd(int number);
};

#endif // UTILS_H
