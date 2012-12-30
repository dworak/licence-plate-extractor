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
    }

//    double min, total;
//    minMaxLoc(result, &min);
//    result -= min;
//    total = sum(result)[0];
//    result /= total;

    return result;
}

cv::Rect Utils::getLPInterior(const cv::Mat &lp)
{
    cv::Mat _lp;
    threshold(lp, _lp, Utils::getLPThreshold(lp, 1./4, 1./4) - 10, 255, cv::THRESH_BINARY);

    std::vector< std::vector<cv::Point> > contours;
    findContours(_lp, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
    QList<cv::Rect> rects;
    for(unsigned i=0; i<contours.size(); i++)
        rects.append(boundingRect(contours[i]));

    const cv::Rect *innerRect = NULL;
    double minArea = _lp.cols * _lp.rows;
    foreach(const cv::Rect& rect, rects){
        if(rect.width > 0.5 * _lp.cols && rect.height > 0.5 * _lp.rows && rect.area() < minArea){
            minArea = rect.area();
            innerRect = &rect;
        }
    }

    if(innerRect != NULL)
        return *innerRect;
    else
        return cv::Rect();
}

// finds rectangles bounding license plates
// mft - image thresholded after match filter
// sobelT - image thresholded after Sobel filter
QLinkedList<cv::Rect> Utils::getLPRects(const cv::Mat &mft, const cv::Mat &sobelT, int areaThreshold, double ratioThreshold)
{
    QLinkedList<cv::Rect> foundRects;
    std::vector< std::vector<cv::Point> > contours;
    cv::Mat _mft = mft.clone();
    findContours(_mft, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
    for(unsigned i=0; i<contours.size(); i++){
        cv::Rect r = boundingRect(contours[i]);
        int x1 = r.x - r.width / 2 >= 0 ? r.x - r.width / 2 : 0;
        int x2 = r.x + 3 * r.width / 2 < mft.cols ? r.x + 3 * r.width / 2 : mft.cols;
        int y1 = r.y - r.height >= 0 ? r.y - r.height : 0;
        int y2 = r.y + 2 * r.height < mft.rows ? r.y + 2 * r.height : mft.rows;
        cv::Mat roi = sobelT(cv::Range(y1, y2), cv::Range(x1, x2)).clone();
        morphologyEx(roi, roi, cv::MORPH_CLOSE, cv::Mat(), cv::Point(-1, -1), 2);
        std::vector< std::vector<cv::Point> > roiContours;
        std::vector<cv::Rect> rects;    // candidates in roi
        findContours(roi, roiContours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
        for(unsigned i=0; i < roiContours.size(); i++)
            rects.push_back(boundingRect(roiContours[i]));
        double maxArea = 0;
        cv::Rect *maxRect = NULL;      // best candidate
        for(unsigned i=0; i < rects.size(); i++){
            double area = rects[i].area();
            if(area > maxArea){
                maxArea = area;
                maxRect = &rects[i];
            }
        }
        if(maxArea > 0)
            foundRects.append(cv::Rect(maxRect->x + x1, maxRect->y + y1, maxRect->width, maxRect->height));
    }

    // merge overlapping rectangles
    for(QLinkedList<cv::Rect>::iterator rect1 = foundRects.begin(); rect1 != foundRects.end();){
        QList<cv::Rect> overlapping;
        for(QLinkedList<cv::Rect>::iterator rect2 = rect1 + 1; rect2 != foundRects.end(); rect2++)
            if(rect1 != rect2){
                cv::Rect intersection = *rect1 & *rect2;
                if(intersection.area() > 0)
                    overlapping.append(*rect2);
            }
        cv::Rect sum = *rect1;
        foreach(const cv::Rect &r, overlapping){
            sum |= r;
            foundRects.removeOne(r);
        }
        if(!overlapping.empty()){
            rect1 = foundRects.erase(rect1);
            foundRects.append(sum);
        }
        else
            rect1++;
    }

    // remove false rectangles
    for(QLinkedList<cv::Rect>::iterator rect = foundRects.begin(); rect != foundRects.end();){
        if((*rect).area() < areaThreshold || (double)(*rect).width / (*rect).height < ratioThreshold)
            rect = foundRects.erase(rect);
        else
            rect++;
    }

    return foundRects;
}

QList<cv::Rect> Utils::getLPSignsRects(const cv::Mat &lp, int C)
{
    cv::Mat _lp;
    adaptiveThreshold(lp, _lp, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, Utils::makeOdd(lp.rows), C);

    // find rectangles around contours
    std::vector< std::vector<cv::Point> > contours;
    findContours(_lp, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
    QLinkedList<cv::Rect> rects;
    for(unsigned i=0; i < contours.size(); i++)
        rects.append(boundingRect(contours[i]));

    // remove false rectangles
    for(QLinkedList<cv::Rect>::iterator rect = rects.begin(); rect != rects.end();){
        double ratio = (double)(*rect).width / (*rect).height;
        double area = (*rect).area();
        if(ratio < 0.4 || ratio > 0.9 || area < 1000 || area > 6000)
            rect = rects.erase(rect);
        else
            rect++;
    }

    // remove rectangles contained in another rectangles
    for(QLinkedList<cv::Rect>::iterator rect1 = rects.begin(); rect1 != rects.end();rect1++){
        for(QLinkedList<cv::Rect>::iterator rect2 = rects.begin(); rect2 != rects.end();){
            cv::Rect r1 = *rect1;
            cv::Rect r2 = *rect2;
            if(r1.contains(r2.tl()) && r1.contains(r2.br()))
                rect2 = rects.erase(rect2);
            else
                rect2++;
        }
    }

    // sort rectangles
    QMap<int, cv::Rect> map;
    foreach(const cv::Rect &rect, rects)
        map.insert(rect.x, rect);
    QList<cv::Rect> sortedRects = map.values();

    // remove false left outermost rectangle if any
    if(sortedRects.size() > 0){
        cv::Rect &first = sortedRects.first();
        cv::Mat roi = lp(first);
        roi = roi(cv::Range(0, roi.rows / 2), cv::Range::all());
        cv::Scalar mean, stddev;
        cv::meanStdDev(roi, mean, stddev);
        if(stddev[0] < 20 || first.x <= 2)
            sortedRects.removeFirst();
    }

    return sortedRects;
}

QList<cv::Rect> Utils::getLPSignsRects(const cv::Mat &lp)
{
    // find best set of rectangles
    QMap< double, QList<cv::Rect> > candidates;
    for(int i = -10; i <= 10; i+=10){
        QList<cv::Rect> candidate = getLPSignsRects(lp, i);
        double totalArea = 0;
        foreach(const cv::Rect &rect, candidate)
            totalArea += rect.area();
        candidates.insert(totalArea, candidate);
    }
    QList<cv::Rect> best = candidates.values().last();

    // align heights
    if(best.size() >= 4){
        QList<int> tops, bottoms;
        foreach(const cv::Rect &rect, best){
            tops.append(rect.y);
            bottoms.append(rect.br().y);
        }
        qSort(tops);
        qSort(bottoms);
        int height;
        height = bottoms[bottoms.size() - 2] - tops[1];
        for(int i=0; i<best.size(); i++){
            best[i].y -= (height - best[i].height) / 2;
            best[i].height = height;
        }
    }

    return best;
}

double Utils::getLPThreshold(const cv::Mat &lp, double leftMargin, double rightMargin)
{
    cv::Mat roi = lp(cv::Range(lp.rows / 3, 2 * lp.rows / 3), cv::Range(leftMargin * lp.cols, (1 - rightMargin) * lp.cols)).clone();
    cv::Mat _roi;
    equalizeHist(roi, _roi);
    cv::Mat mask1, mask2;
    threshold(_roi, mask1, 143, 255, cv::THRESH_BINARY);
    double mean1 = mean(roi, mask1)[0];
    threshold(_roi, mask2, 111, 255, cv::THRESH_BINARY_INV);
    double mean2 = mean(roi, mask2)[0];
    return (mean1 + mean2) / 2;
}

int Utils::makeOdd(int number)
{
    return number % 2 == 0 ? number + 1 : number;
}
