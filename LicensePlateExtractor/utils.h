#ifndef UTILS_H
#define UTILS_H

#include <QImage>
#include <QLinkedList>
#include <QMap>

#include <opencv2/opencv.hpp>

#include "cvimagewidget.h"

typedef QMap<char, cv::Mat> Patterns;

class Utils
{
public:
    static cv::Mat QImage2Mat(const QImage &img);
    enum CharacterSubset{POWIAT_CHARACTERISTIC, VEHICLE_CHARACTERISITC, ALL};
    static void preparePatterns(Patterns &patterns, CharacterSubset characterSubset);
    static cv::Mat getMatchFilterKernel(int m, int n, double sd, double A, double B);
    static cv::Mat getLPInterior(const cv::Mat &lp);
    static cv::Rect getLPInteriorRect(const cv::Mat &lp);   // deprecated
    static bool getLPRotInteriorRect(const cv::Mat &lp, cv::RotatedRect &res);
    static QList<cv::Rect> getLPRects(const cv::Mat &mft, const cv::Mat &sobelT, int areaThreshold, double ratioThreshold);
    static QList<cv::Rect> getLPCharactersRects(const cv::Mat &lp, int C, cv::Mat &lpAfterAT);
    static QList<cv::Rect> getLPCharactersRects(const cv::Mat &lp, cv::Mat &lpAfterAT);
    static double getLPThreshold(const cv::Mat &lp, double leftMargin, double rightMargin);
    static char recognizeCharacter(const cv::Mat &character, const Patterns &patterns);
    static int makeOdd(int number);
};

#endif // UTILS_H
