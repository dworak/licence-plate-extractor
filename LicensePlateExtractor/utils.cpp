#include "utils.h"

#include <QDebug>

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
        if(stddev[0] < 20 || first.x <= lp.cols / 10)
            sortedRects.removeFirst();
    }

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
    for(int i = -15; i <= 15; i+=5){
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
    int margin = (midBottom - midTop) / 5;
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
    cv::Mat ch;
    cv::resize(character, ch, cv::Size(47, 80));
    cv::equalizeHist(ch, ch);
    int minSum = 47 * 80 * 255 / 3;
    char bestCh = '?';
    for(Patterns::const_iterator it = patterns.begin(); it != patterns.end(); it++){
        cv::Mat diff;
        absdiff(ch, it.value(), diff);
        int sum = cv::sum(diff)[0];
        if(sum < minSum){
            minSum = sum;
            bestCh = it.key();
        }
    }

    return bestCh;
}

int Utils::makeOdd(int number)
{
    return number % 2 == 0 ? number + 1 : number;
}
