#include "utils.h"

#include <QDebug>
#include <opencv2/opencv.hpp>

QImage *Utils::IplImage2QImage(IplImage *iplImg)
{
    int h = iplImg->height;
    int w = iplImg->width;
    int channels = iplImg->nChannels;
    QImage *qimg = new QImage(w, h, QImage::Format_ARGB32);
    char *data = iplImg->imageData;

    for (int y = 0; y < h; y++, data += iplImg->widthStep){
        for (int x = 0; x < w; x++){
            char r = 0, g = 0, b = 0, a = 0;
            if (channels == 1){
                r = data[x * channels];
                g = data[x * channels];
                b = data[x * channels];
            }
            else if (channels == 3 || channels == 4){
                r = data[x * channels + 2];
                g = data[x * channels + 1];
                b = data[x * channels];
            }

            if (channels == 4){
                a = data[x * channels + 3];
                qimg->setPixel(x, y, qRgba(r, g, b, a));
            }
            else{
                qimg->setPixel(x, y, qRgb(r, g, b));
            }
        }
    }
    return qimg;
}

QImage Utils::Mat2QImage(cv::Mat &matImg)
{
    int h = matImg.rows;
    int w = matImg.cols;
    int channels = matImg.channels();
    QImage qimg = QImage(w, h, QImage::Format_ARGB32);
    uchar *data = matImg.data;

    for (int y = 0; y < h; y++, data += matImg.step){
        for (int x = 0; x < w; x++){
            char r = 0, g = 0, b = 0, a = 0;
            if (channels == 1){
                r = data[x * channels];
                g = data[x * channels];
                b = data[x * channels];
            }
            else if (channels == 3 || channels == 4){
                r = data[x * channels + 2];
                g = data[x * channels + 1];
                b = data[x * channels];
            }

            if (channels == 4){
                a = data[x * channels + 3];
                qimg.setPixel(x, y, qRgba(r, g, b, a));
            }
            else{
                qimg.setPixel(x, y, qRgb(r, g, b));
            }
        }
    }
    return qimg;

//    cv::Mat dest(matImg.rows, matImg.cols, matImg.type());
//    cvtColor(matImg, dest, CV_BGR2RGB);
//    return QImage((uchar*) dest.data, dest.cols, dest.rows, QImage::Format_RGB888);

//    IplImage* iplImg = new IplImage(matImg);
//    QImage *qi = IplImage2QImage(iplImg);
//    return *qi;
}

cv::Mat Utils::getMatchFilterKernel(int m, int n, double sd, double A, double B)
{
//    double kernel[m][n];
//    for(int x=0; x<m; x++){
//        double val;
//        if(x < m / 3 || x > 2 * m / 3)
//            val = A * exp(- x * x / (0.2 * variance));
//        else
//            val = B * exp(- x * x / (2 * variance));
//        qDebug() << "mfKernel";
//        for(int y=0; y<n; y++)
//            kernel[x][y] = val;
//    }
//    return cv::Mat(m, n, CV_64F, kernel);

    cv::Mat result(m, n, CV_64F);
    double variance = sd * sd;
    double mm = m - 1;

    for(int x=0; x<m; x++){
        double val;
        if(x < mm / 3 || x > 2 * mm / 3){
            double trans = x < mm / 3 ? mm / 4 : mm / 4 * 3;
            double xTrans = x - trans;
            val = A * exp(- xTrans * xTrans / (0.2 * variance));
        }
        else{
            double xTrans = x - mm / 2;
            val = B * exp(- xTrans * xTrans / (2 * variance));
        }
        for(int y=0; y<n; y++)
            result.at<double>(x, y) = val;
        qDebug() << val;
    }
    qDebug() << "";

//    double min, total;
//    minMaxLoc(result, &min);
//    result -= min;
//    total = sum(result)[0];
//    result /= total;

    return result;
}

int Utils::makeOdd(int number)
{
    return number % 2 == 0 ? number + 1 : number;
}
