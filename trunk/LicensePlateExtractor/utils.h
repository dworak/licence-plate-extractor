#ifndef UTILS_H
#define UTILS_H

#include <QImage>
#include <QLinkedList>

//#include <opencv2/core/core.hpp>
//#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

class Utils
{
public:
    static QImage* IplImage2QImage(IplImage *iplImg);
    static QImage Mat2QImage(cv::Mat &matImg);
    static cv::Mat getMatchFilterKernel(int m, int n, double sd, double A, double B);
    static cv::Rect getLPInterior(const cv::Mat &lp);
    static QLinkedList<cv::Rect> getLPRects(const cv::Mat &mft, const cv::Mat &sobelT, int areaThreshold, double ratioThreshold);
    static QLinkedList<cv::Rect> getLPSignsRects(const cv::Mat &lp);
    static double getLPThreshold(const cv::Mat &lp, double leftMargin, double rightMargin);
    static int makeOdd(int number);
};

#endif // UTILS_H
