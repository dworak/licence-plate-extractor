#include "utils.h"

#include <QDebug>

using namespace cv;

cv::Mat Utils::QImage2Mat(const QImage &img)
{
    return cv::Mat(img.height(), img.width(), CV_8UC1, (uchar*)img.bits(), img.bytesPerLine()).clone();
}

void Utils::preparePatterns(Patterns &patterns, CharacterSubset characterSubset)
{
    patterns = Patterns();

    if(characterSubset == VEHICLE_CHARACTERISITC || characterSubset == ALL){
        patterns.insert('0', QImage2Mat(QImage(":/ch/0")));
        patterns.insert('1', QImage2Mat(QImage(":/ch/1")));
        patterns.insert('2', QImage2Mat(QImage(":/ch/2")));
        patterns.insert('3', QImage2Mat(QImage(":/ch/3")));
        patterns.insert('4', QImage2Mat(QImage(":/ch/4")));
        patterns.insert('5', QImage2Mat(QImage(":/ch/5")));
        patterns.insert('6', QImage2Mat(QImage(":/ch/6")));
        patterns.insert('7', QImage2Mat(QImage(":/ch/7")));
        patterns.insert('8', QImage2Mat(QImage(":/ch/8")));
        patterns.insert('9', QImage2Mat(QImage(":/ch/9")));
    }

    patterns.insert('A', QImage2Mat(QImage(":/ch/A")));
    patterns.insert('C', QImage2Mat(QImage(":/ch/C")));
    patterns.insert('E', QImage2Mat(QImage(":/ch/E")));
    patterns.insert('F', QImage2Mat(QImage(":/ch/F")));
    patterns.insert('G', QImage2Mat(QImage(":/ch/G")));
    patterns.insert('H', QImage2Mat(QImage(":/ch/H")));
    patterns.insert('J', QImage2Mat(QImage(":/ch/J")));
    patterns.insert('K', QImage2Mat(QImage(":/ch/K")));
    patterns.insert('L', QImage2Mat(QImage(":/ch/L")));
    patterns.insert('M', QImage2Mat(QImage(":/ch/M")));
    patterns.insert('N', QImage2Mat(QImage(":/ch/N")));
    patterns.insert('P', QImage2Mat(QImage(":/ch/P")));
    patterns.insert('R', QImage2Mat(QImage(":/ch/R")));
    patterns.insert('S', QImage2Mat(QImage(":/ch/S")));
    patterns.insert('T', QImage2Mat(QImage(":/ch/T")));
    patterns.insert('U', QImage2Mat(QImage(":/ch/U")));
    patterns.insert('V', QImage2Mat(QImage(":/ch/V")));
    patterns.insert('W', QImage2Mat(QImage(":/ch/W")));
    patterns.insert('X', QImage2Mat(QImage(":/ch/X")));
    patterns.insert('Y', QImage2Mat(QImage(":/ch/Y")));

    if(characterSubset == POWIAT_CHARACTERISTIC || characterSubset == ALL){
        patterns.insert('B', QImage2Mat(QImage(":/ch/B")));
        patterns.insert('D', QImage2Mat(QImage(":/ch/D")));
        patterns.insert('I', QImage2Mat(QImage(":/ch/I")));
        patterns.insert('O', QImage2Mat(QImage(":/ch/O")));
        patterns.insert('Z', QImage2Mat(QImage(":/ch/Z")));
    }
}

cv::Mat Utils::getMatchFilterKernel(int m, int n, double sd, double A, double B)
{
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

    return result;
}

Mat Utils::getHistImage(const Mat &hist, int dim, int size)
{
    if(dim == 0){
        Mat histImg = Mat::zeros(size, hist.cols, CV_8U);
        for(int i=0; i < hist.cols; i++)
            line(histImg, Point(i, size - 1), Point(i, size - 1 - hist.at<uchar>(i) * size / 256), Scalar(255, 255, 255));
        return histImg;
    }
    else{
        Mat histImg = Mat::zeros(hist.rows, size, CV_8U);
        for(int i=0; i < hist.rows; i++)
            line(histImg, Point(0, i), Point(hist.at<uchar>(i) * size / 256, i), Scalar(255, 255, 255));
        return histImg;
    }
}

Mat Utils::getLPInterior(const Mat &lp)
{
    RotatedRect interior;
    if(Utils::getLPRotInteriorRect(lp, interior)){
        Mat M, rotated, cropped;
        float angle = interior.angle;
        Size rect_size = interior.size;
        if (interior.angle < -45.) {
            angle += 90.0;
            int tmp = rect_size.width;
            rect_size.width = rect_size.height;
            rect_size.height = tmp;
        }
        // get the rotation matrix
        M = getRotationMatrix2D(interior.center, angle, 1.0);
        // perform the affine transformation
        warpAffine(lp, rotated, M, lp.size(), INTER_CUBIC);
        // crop the resulting image
        getRectSubPix(rotated, rect_size, interior.center, cropped);

        return cropped;
    }
    else
        return lp;
}

Rect Utils::trimLPInterior(const Mat &lp)
{
    Mat hist;
    reduce(lp, hist, 1, CV_REDUCE_AVG);
    int upper[2], bottom[2];
    minMaxIdx(hist(Range(0, hist.rows / 2), Range::all()), NULL, NULL, upper);
    minMaxIdx(hist(Range(hist.rows / 2, hist.rows), Range::all()), NULL, NULL, bottom);
    bottom[0] += hist.rows / 2;
    return Rect(0, upper[0], lp.cols, bottom[0] - upper[0]);
}

cv::Rect Utils::getLPInteriorRect(const cv::Mat &lp)
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

bool Utils::getLPRotInteriorRect(const cv::Mat &lp, cv::RotatedRect &res)
{
    cv::Mat _lp;
    threshold(lp, _lp, Utils::getLPThreshold(lp, 1./4, 1./4) - 10, 255, cv::THRESH_BINARY);
    //adaptiveThreshold(lp, _lp, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, Utils::makeOdd(lp.rows), -10);

    std::vector< std::vector<cv::Point> > contours;
    findContours(_lp, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
    QList<cv::Rect> rects;
    for(unsigned i=0; i<contours.size(); i++)
        rects.append(boundingRect(contours[i]));

    int innerRectI = -1;
    double minArea = _lp.cols * _lp.rows;
    for(int i=0; i < rects.size(); i++){
        const cv::Rect &rect = rects[i];
        if(rect.width > 0.5 * _lp.cols && rect.height > 0.5 * _lp.rows && rect.area() < minArea){
            minArea = rect.area();
            innerRectI = i;
        }
    }

    if(innerRectI > -1){
        res = cv::minAreaRect(contours[innerRectI]);
        return true;
    }
    else
        return false;
}

// finds rectangles bounding license plates
// mft - image thresholded after match filter
// sobelT - image thresholded after Sobel filter
QList<cv::Rect> Utils::getLPRects(const cv::Mat &mft, const cv::Mat &sobelT, int areaThreshold, double ratioThreshold)
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

    QList<cv::Rect> result;
    foreach(const cv::Rect &rect, foundRects)
        result.append(rect);
    return result;
}

QList<cv::Rect> Utils::getLPCharactersRects(const cv::Mat &lp, int C, cv::Mat &lpAfterAT)
{
    adaptiveThreshold(lp, lpAfterAT, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, Utils::makeOdd(lp.rows), C);

    // find rectangles around contours
    cv::Mat _lp = lpAfterAT.clone();
    std::vector< std::vector<cv::Point> > contours;
    findContours(_lp, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
    QLinkedList<cv::Rect> rects;
    for(unsigned i=0; i < contours.size(); i++)
        rects.append(boundingRect(contours[i]));

    // remove false rectangles
    for(QLinkedList<cv::Rect>::iterator rect = rects.begin(); rect != rects.end();){
        double ratio = (double)(*rect).width / (*rect).height;
        double area = (*rect).area();
        if(ratio < 0.3 || ratio > 0.9 || area < 700 || area > 6000)
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
        if(stddev[0] < 20 || first.x <= 3)
            sortedRects.removeFirst();
    }

    // remove false right outermost rectangle if any
    if(sortedRects.size() > 0)
        if(sortedRects.last().x >= lp.cols - 3)
            sortedRects.removeLast();

    // remove rectangles with very low top y or very high bottom y
    QList<int> tops, bottoms;
    foreach(const cv::Rect &rect, sortedRects){
        tops.append(rect.y);
        bottoms.append(rect.br().y);
    }
    qSort(tops);
    qSort(bottoms);
    int midTop, midBottom;
    midTop = tops[(tops.size() - 1) / 2];
    midBottom = bottoms[bottoms.size() / 2];
    int margin = (midBottom - midTop) / 5;
    for(int i=0; i < sortedRects.size(); i++)
        if(sortedRects[i].y < midTop - margin || sortedRects[i].br().y > midBottom + margin)
            sortedRects.removeAt(i);

    return sortedRects;
}

QList<cv::Rect> Utils::getLPCharactersRects(const cv::Mat &lp, cv::Mat &lpAfterAT)
{
    // find best set of rectangles
    QMap< double, QList<cv::Rect> > candidates;
    QMap<double, cv::Mat> lpsAfterAT;
    for(int i = 10; i <= 10; i+=5){
        cv::Mat lpAfterAT;
        QList<cv::Rect> candidate = getLPCharactersRects(lp, i, lpAfterAT);
        double totalArea = 0;
        foreach(const cv::Rect &rect, candidate)
            totalArea += rect.area();
        candidates.insert(totalArea, candidate);
        lpsAfterAT.insert(totalArea, lpAfterAT);
    }
    QList<cv::Rect> best = candidates.values().last();
    lpAfterAT = lpsAfterAT.values().last();

    // extend small heights
    QList<int> tops, bottoms;
    foreach(const cv::Rect &rect, best){
        tops.append(rect.y);
        bottoms.append(rect.br().y);
    }
    qSort(tops);
    qSort(bottoms);
    int midTop, midBottom;
    midTop = tops[(tops.size() - 1) / 2];
    midBottom = bottoms[bottoms.size() / 2];
    int margin = (midBottom - midTop) / 10;
    for(int i=0; i < best.size(); i++){
        if(best[i].y > midTop + margin){
            best[i].height += best[i].y - midTop;
            best[i].y = midTop;
        }
        if(best[i].br().y < midBottom - margin)
            best[i].height += midBottom - best[i].br().y;
    }

    // extend small widths
    for(int i=0; i < best.size(); i++){
        if((double)best[i].width / best[i].height < 0.5){
            if (i==best.size()-1 || best[i+1].x-(best[i].x+best[i].width)>2 || (double)best[i].width / best[i].height >=0.55)
            {
                int newWidth = 0.5875 * best[i].height;
                best[i].x = best[i].x - (newWidth - best[i].width) / 2;
                best[i].width = newWidth;
                if(best[i].x < 0){
                    best[i].width += best[i].x;
                    best[i].x = 0;
                }
                if(best[i].br().x > lp.cols)
                    best[i].width -= best[i].br().x - lp.cols;
            }
            else
            {
                best[i].width = best[i+1].x+best[i+1].width - best[i].x;
                if (best[i+1].y<best[i].y)
                {
                    best[i].height = best[i].height + best[i].y - best[i+1].y;
                    best[i].y =  best[i+1].y;
                }
                if (best[i+1].y+best[i+1].height>best[i].y+best[i].height)
                    best[i].height = best[i+1].y+best[i+1].height-best[i].y;
            }
        }
    }


    // delete overlapping rectangles
    for(QList<cv::Rect>::iterator rect1 = best.begin(); rect1 != best.end();){
        QList<cv::Rect> overlapping;
        for(QList<cv::Rect>::iterator rect2 = rect1 + 1; rect2 != best.end(); rect2++)
            if(rect1 != rect2){
                cv::Rect intersection = *rect1 & *rect2;
                if(intersection.area() > 0)
                    overlapping.append(*rect2);
            }
        foreach(const cv::Rect &r, overlapping){
            best.removeOne(r);
        }
        rect1++;
    }





    // estimate missing rectangles
    int midWidth = 0.5875 * (midBottom - midTop);
    QList<cv::Rect> newRects;
    for(int i=0; i < best.size() - 1; i++){
        int diff = best[i + 1].x - best[i].br().x;
        if(diff > 0.9 * midWidth){
            int num = diff / (1.2 * midWidth);
            for(int j=0; j < num; j++){
                int lx = best[i].br().x + (j * diff) / num;
                int rx = lx + diff / num;
                newRects.append(cv::Rect(lx + (rx - lx - midWidth) / 2, midTop, midWidth, midBottom - midTop));
            }
        }
    }
    best.append(newRects);

    // once again sort rectangles
//    QMap<int, cv::Rect> map;
//    foreach(const cv::Rect &rect, best)
//        map.insert(rect.x, rect);
//    best = map.values();


    return best;
}

QList<Rect> Utils::getLPCharactersRectsByHist(Mat &lp, Mat &lpAfterAT, Mat &hHist)
{
    // preprocess
    adaptiveThreshold(lp, lpAfterAT, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, Utils::makeOdd(lp.rows), 10);
    Rect trimmed = Utils::trimLPInterior(lpAfterAT);
    lpAfterAT = lpAfterAT(trimmed);
    lp = lp(trimmed);

    // get horizontal histogram
    reduce(lpAfterAT, hHist, 0, CV_REDUCE_AVG);

    // find rects horizontally
    QList<Rect> rects = getLPChHRects(hHist, 20, lp.rows);

    if(rects.size() > 1){
        // find rects vertically
        for(int i=0; i < rects.size(); i++)
            rects[i] = getLPChVRect(lpAfterAT(rects[i]), rects[i]);

        // remove rectangles with very low top y or very high bottom y
        QList<int> topsHigh, bottomsHigh;
            foreach(const cv::Rect &rect, rects){
                topsHigh.append(rect.y);
                bottomsHigh.append(rect.br().y);
            }
            qSort(topsHigh);
            qSort(bottomsHigh);
            int midTopHigh, midBottomHigh;
            midTopHigh = topsHigh[(topsHigh.size() - 1) / 2];
            midBottomHigh = bottomsHigh[bottomsHigh.size() / 2];
            int marginHigh = (midBottomHigh - midTopHigh) / 10;
            for(int i=0; i < rects.size(); i++)
                if(rects[i].y < midTopHigh - marginHigh || rects[i].br().y > midBottomHigh + marginHigh)
                    rects.removeAt(i);

        // extend small heights
        QList<int> tops, bottoms;
        foreach(const cv::Rect &rect, rects){
            tops.append(rect.y);
            bottoms.append(rect.br().y);
        }
        qSort(tops);
        qSort(bottoms);
        int midTop, midBottom;
        midTop = tops[(tops.size() - 1) / 2];
        midBottom = bottoms[bottoms.size() / 2];
        int margin = (midBottom - midTop) / 10;
        for(int i=0; i < rects.size(); i++){
            if(rects[i].y > midTop + margin){
                rects[i].height += rects[i].y - midTop;
                rects[i].y = midTop;
            }
            if(rects[i].br().y < midBottom - margin)
                rects[i].height += midBottom - rects[i].br().y;
        }

        // split too wide rects
        QList<Rect> afterSplit;
        QList<int> widths;
        for(int i=0; i < rects.size(); i++)
            widths.append(rects[i].width);
        qSort(widths);
        int midWidth = widths[widths.size() / 2];
        for(int i=0; i < rects.size(); i++)
            if(rects[i].width > 1.3 * midWidth){
    //            Mat hist = hHist(Range::all(), Range(rects[i].x, rects[i].x + rects[i].width));
    //            QList<Rect> newRects;
    //            int shift = 20;
    //            do{
    //                shift += 1;
    //                newRects = getLPChHRects(hist, shift, lp.rows);
    //            }while(newRects.size() <= 1 && shift < 50);

    //            // shift new rects
    //            for(int j=0; j < newRects.size(); j++)
    //                newRects[j].x += rects[i].x;
    //            afterSplit.append(newRects);
    //            rects.removeAt(i);

                int n = rects[i].width / (0.85 * midWidth);
                int newWidth = rects[i].width / n - 1;
                for(int j=0; j < n; j++)
                    afterSplit.append(Rect(rects[i].x + j * newWidth, rects[i].y, newWidth, rects[i].height));
                rects.removeAt(i);
            }
        rects.append(afterSplit);

        // remove too narrow rects
        for(int i=0; i < rects.size(); i++)
            if(rects[i].width < 0.5 * midWidth)
                rects.removeAt(i);

        // sort rects
        QMap<int, cv::Rect> map;
        foreach(const cv::Rect &rect, rects)
            map.insert(rect.x, rect);
        rects = map.values();

        // extend small widths
        for(int i=0; i < rects.size(); i++){
            if((double)rects[i].width / rects[i].height < 0.5){
                if (i==rects.size()-1 || rects[i+1].x-(rects[i].x+rects[i].width)>2 || (double)rects[i].width / rects[i].height >=0.55)
                {
                    int newWidth = 0.5875 * rects[i].height;
                    rects[i].x = rects[i].x - (newWidth - rects[i].width) / 2;
                    rects[i].width = newWidth;
                    if(rects[i].x < 0){
                        rects[i].width += rects[i].x;
                        rects[i].x = 0;
                    }
                    if(rects[i].br().x > lp.cols)
                        rects[i].width -= rects[i].br().x - lp.cols;
                }
                else
                {
                    rects[i].width = rects[i+1].x+rects[i+1].width - rects[i].x;
                    if (rects[i+1].y<rects[i].y)
                    {
                        rects[i].height = rects[i].height + rects[i].y - rects[i+1].y;
                        rects[i].y =  rects[i+1].y;
                    }
                    if (rects[i+1].y+rects[i+1].height>rects[i].y+rects[i].height)
                        rects[i].height = rects[i+1].y+rects[i+1].height-rects[i].y;
                }
            }
        }

        // shift all 1 pixel right (tmp!)
        for(int i=0; i < rects.size(); i++)
            rects[i].x += 1;
        if(rects.last().br().x > lp.cols)
            rects.last().width -= 1;

        // shift all 1 pixel down (tmp!)
        for(int i=0; i < rects.size(); i++){
            if(rects[i].height > 1){
                rects[i].y += 1;
                if(rects[i].br().y > lp.rows)
                    rects[i].height -= 1;
            }
        }

        // remove false left outermost rectangle if any
        if(rects.size() > 0){
            Rect &first = rects.first();
            Mat roi = lp(first);
            roi = roi(cv::Range(0, roi.rows / 2), cv::Range::all());
            Scalar roiMean, lpMean, sd;
            meanStdDev(roi, roiMean, sd);
            meanStdDev(lp, lpMean, sd);
            if(roiMean[0] < 0.8 * lpMean[0])
                rects.removeFirst();
        }

        // find widest space
        int spaceWidth = 0, spacePos;
        for(int i=0; i < rects.size() - 1; i++){
            int width = rects[i + 1].x - rects[i].br().x;
            if(width > spaceWidth){
                spaceWidth = width;
                spacePos = i;
            }
        }

        // once again remove false left outermost rectangle if any
        if(spacePos == 3)
            rects.removeFirst();

        // remove false rects on the right
        while(rects.size() > spacePos + 6)
            rects.removeLast();

        return rects;
    }
    else    // rects number < 2
        return QList<Rect>();
}

QList<Rect> Utils::getLPChHRects(const Mat &hHist, int thres, int lpHeight)
{
    double th[2];
    minMaxIdx(hHist(Range::all(), Range(hHist.cols * 0.2, hHist.cols * 0.8)), th, NULL);
    Mat _hHist;
    threshold(hHist, _hHist, th[0] + thres, 255, THRESH_BINARY);
    morphologyEx(_hHist, _hHist, cv::MORPH_CLOSE, Mat::ones(1, 2, CV_8U), cv::Point(-1, -1), 1);

    // find rects horizontally
    QList<Rect> rects;
    int beginning, width;
    int prev = 0;
    for(int i=0; i < _hHist.cols; i++){
        if(_hHist.at<int>(i)){
            if(prev)        // ww
                width++;
            else{           // bw
                beginning = i;
                width = 1;
            }
        }
        else{
            if(prev && width > _hHist.cols / 20)        // wb
                rects.append(Rect(beginning, 0, width, lpHeight));
            else{           // bb
            }
        }
        prev = _hHist.at<int>(i);
    }
    if(prev && width > _hHist.cols / 20)        // wb
        rects.append(Rect(beginning, 0, width, lpHeight));

    return rects;
}

Rect Utils::getLPChVRect(const Mat &ch, const cv::Rect &rect)
{
    Mat vHist;
    reduce(ch, vHist, 1, CV_REDUCE_AVG);

    // find rects vertically
    QList<Rect> rects;
    int beginning, height;
    int prev = 0;
    for(int i=0; i < vHist.rows; i++){
        if(vHist.at<int>(i)){
            if(prev)        // ww
                height++;
            else{           // bw
                beginning = i;
                height = 1;
            }
        }
        else{
            if(prev)        // wb
                rects.append(Rect(rect.x, beginning, rect.width, height));
            else{           // bb
            }
        }
        prev = vHist.at<int>(i);
    }
    if(prev)        // wb
        rects.append(Rect(rect.x, beginning, rect.width, height));

    // find rect with largest height
    int val = 0;
    Rect best;
    for(int i=0; i < rects.size(); i++)
        if(rects[i].height > val){
            val = rects[i].height;
            best = rects[i];
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

char Utils::recognizeCharacter(const cv::Mat &character, const Patterns &patterns)
{
//    cv::Mat ch;
//    cv::resize(character, ch, cv::Size(47, 80));
//    cv::equalizeHist(ch, ch);
//    int minSum = 47 * 80 * 255 / 3;
//    char bestCh = '?';
//    for(Patterns::const_iterator it = patterns.begin(); it != patterns.end(); it++){
//        cv::Mat diff;
//        absdiff(ch, it.value(), diff);
//        int sum = cv::sum(diff)[0];
//        if(sum < minSum){
//            minSum = sum;
//            bestCh = it.key();
//        }
//    }

//    return bestCh;
    return (recognizeCharacterWithProbab(character, patterns).end() - 1).value();
}

CharRecognitionResult Utils::recognizeCharacterWithProbab(const cv::Mat &character, const Patterns &patterns)
{
    cv::Mat ch;
    CharRecognitionResult result;
    cv::resize(character, ch, cv::Size(47, 80));
    cv::equalizeHist(ch, ch);
    for(Patterns::const_iterator it = patterns.begin(); it != patterns.end(); it++){
        cv::Mat diff;
        absdiff(ch, it.value(), diff);
        int sum = cv::sum(diff)[0];
        double probab = 1 - (double)sum/(47*80*255);

        // special cases
        switch(it.key()){
        case '0':
            probab += 0.01; break;
        case '8':
            probab -= 0.01; break;

        case '1':
            probab += 0.01; break;

        case '5':
            probab += 0.006; break;
        case 'S':
            probab -= 0.008; break;

        case 'W':
            probab -= 0.02; break;

        case 'A':
            probab -= 0.01; break;

        case 'L':
            probab += 0.01; break;

        case '7':
            probab += 0.01; break;
        case 'Z':
            probab -= 0.01; break;

        case 'J':
            probab += 0.03; break;

        case 'C':
            probab += 0.02; break;

        case  'B':
            probab -= 0.01; break;

        case 'K':
            probab -= 0.01; break;

        case 'X':
            probab -= 0.01; break;
        }

        if(probab < 0)
            probab = 0;
        else if(probab > 1)
            probab = 1;

        result.insert(probab, it.key());
    }
    return result;
}

QList<QPair<char, double> > Utils::recognizePowiat(const QList<CharRecognitionResult> &crr, const Powiaty &powiaty)
{
    int size = crr.size();
    QList<int> ind;
    QList<double> probs0, probs;
    QString label;

    // initialize
    for(int i=0; i < size; i++){
        ind += 1;
        label += (crr[i].end() - 1).value();
        probs0.append((crr[i].end() - 1).key());
        probs.append(probs0[i]);
    }

    // find best
    if(size == 2){
        int counter = 0;
        int counterMaxVal = 20;
        while(!powiaty.existsInTwo(label) && counter < counterMaxVal){
            // find char to replace
            double diff0, diff1;
            diff0 = probs0[0] - (crr[0].end() - ind[0] - 1).key();
            diff1 = probs0[1] - (crr[1].end() - ind[1] - 1).key();
            int rep = diff0 < diff1 ? 0 : 1;

            // replace
            ind[rep]++;
            probs[rep] = (crr[rep].end() - ind[rep]).key();
            label[rep] = (crr[rep].end() - ind[rep]).value();
        }
        if(counter == counterMaxVal)
            label = "??";
    }
    else if(size == 3){
        if(probs0[0] < 0.5 * (probs0[1] + probs0[2]) * 0.5)
            return recognizePowiat(crr.mid(1), powiaty);

        int counter = 0;
        int counterMaxVal = 20;
        while(!powiaty.existsInThree(label) && counter < counterMaxVal){
            // find char to replace
            QMultiMap<double, int> diffs;
            for(int i=0; i<3; i++)
                diffs.insert(probs0[i] - (crr[i].end() - ind[i] - 1).key(), i);
            int rep = diffs.values().first();

            // replace
            ind[rep]++;
            probs[rep] = (crr[rep].end() - ind[rep]).key();
            label[rep] = (crr[rep].end() - ind[rep]).value();

            counter++;
        }

        if(counter == counterMaxVal)
            return recognizePowiat(crr.mid(1), powiaty);
    }

    // prepare result
    QList<QPair<char, double> > result;
    for(int i=0; i < size; i++)
        result.append(QPair<char, double>(label.at(i).toAscii(), probs[i]));

    return result;
}

int Utils::makeOdd(int number)
{
    return number % 2 == 0 ? number + 1 : number;
}
