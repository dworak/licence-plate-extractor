#ifndef UTILS_H
#define UTILS_H

#include <QImage>
#include <QLinkedList>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

class Utils
{
public:
    static QImage* IplImage2QImage(IplImage *iplImg);
    static QImage Mat2QImage(cv::Mat &matImg);
    static cv::Mat getMatchFilterKernel(int m, int n, double sd, double A, double B);
    static QLinkedList<cv::Rect> getLPRects(cv::Mat &mft, cv::Mat &sobelT, int areaThreshold, double ratioThreshold);
    static int makeOdd(int number);
};

#endif // UTILS_H
