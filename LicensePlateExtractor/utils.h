#ifndef UTILS_H
#define UTILS_H

#include <QImage>
#include <QLinkedList>
#include <QMap>

//#include <opencv2/core/core.hpp>
//#include <opencv2/highgui/highgui.hpp>
#include "cvimagewidget.h"
#include <opencv2/opencv.hpp>

typedef QMap<char, cv::Mat> Patterns;

class Utils
{
public:
    static QImage* IplImage2QImage(IplImage *iplImg);
    static QImage Mat2QImage(cv::Mat &matImg);
    static cv::Mat QImage2Mat(const QImage &img);
    static void preparePatterns(Patterns &patterns);
    static cv::Mat getMatchFilterKernel(int m, int n, double sd, double A, double B);
    static cv::Rect getLPInterior(const cv::Mat &lp);
    static QList<cv::Rect> getLPRects(const cv::Mat &mft, const cv::Mat &sobelT, int areaThreshold, double ratioThreshold);
    static QList<cv::Rect> getLPCharactersRects(const cv::Mat &lp, int C);
    static QList<cv::Rect> getLPCharactersRects(const cv::Mat &lp);
    static double getLPThreshold(const cv::Mat &lp, double leftMargin, double rightMargin);
    static char recognizeCharacter(const cv::Mat &character, const Patterns &patterns);
    static int makeOdd(int number);
};

#endif // UTILS_H
