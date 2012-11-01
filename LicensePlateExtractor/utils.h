#ifndef UTILS_H
#define UTILS_H

#include <QImage>

#include <opencv2/core/core.hpp>

class Utils
{
public:
    static QImage* IplImage2QImage(IplImage *iplImg);
};

#endif // UTILS_H
