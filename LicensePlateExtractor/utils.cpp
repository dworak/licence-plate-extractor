#include "utils.h"

#include <QDebug>

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

cv::Mat Utils::getMatchFilterKernel(int m, int n, double variance, double A, double B)
{
    double kernel[m][n];
    for(int x=0; x<m; x++){
        double val;
        if(x < m / 3 || x > 2 * m / 3)
            val = A * exp(- x * x / (0.2 * variance));
        else
            val = B * exp(- x * x / (2 * variance));
        qDebug() << "mfKernel";
        for(int y=0; y<n; y++)
            kernel[x][y] = val;
    }
    return cv::Mat(m, n, CV_64F, kernel);
}

int Utils::makeOdd(int number)
{
    return number % 2 == 0 ? number + 1 : number;
}
