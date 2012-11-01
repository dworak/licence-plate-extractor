#include <QtGui/QApplication>
#include "mainwindow.h"

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>

IplImage* skipNFrames(CvCapture* capture, int n)
{
    for(int i = 0; i < n; ++i)
    {
        if(cvQueryFrame(capture) == NULL)
        {
            return NULL;
        }
    }

    return cvQueryFrame(capture);
}

int main(int argc, char *argv[])
{
//    zakomentowane tymczasowo - nie usuwać!
//    QApplication a(argc, argv);
//    MainWindow w;
//    w.show();
    
//    return a.exec();

    CvCapture* capture = cvCaptureFromFile("../Sample/mis.mp4");

    IplImage* frame = NULL;
    do
    {
        frame = skipNFrames(capture, 0);
        cvNamedWindow("filmik", CV_WINDOW_AUTOSIZE);
        cvShowImage("filmik", frame);
        cvWaitKey(1000);
    } while( frame != NULL );

    cvReleaseCapture(&capture);
    cvDestroyWindow("filmik");
    cvReleaseImage(&frame);

    return 0;
}
