#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTimer>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/opencv.hpp>


#include "utils.h"

using namespace cv;


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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->label->setText("");
    ui->label_2->setText("");
    ui->label_3->setText("");
    ui->label_4->setText("");

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(nextFrame()));

    capture = cvCaptureFromFile("../Sample/mis.mp4");
}

MainWindow::~MainWindow()
{
    cvReleaseCapture(&capture);
    delete ui;
}

void MainWindow::nextFrame()
{
    IplImage* frame = NULL;
    frame = skipNFrames(capture, 0);
    if( frame != NULL ){
        QImage* qImage = Utils::IplImage2QImage(frame);
        ui->label->setPixmap(QPixmap::fromImage(*qImage));
        ui->label->repaint();
        IplImage *grayscale = cvCreateImage(cvGetSize(frame),IPL_DEPTH_8U,1);
        cvCvtColor(frame,grayscale,CV_RGB2GRAY);
        QImage *gsImage = Utils::IplImage2QImage(grayscale);
        ui->label_2->setPixmap(QPixmap::fromImage(*gsImage));
        ui->label_2->repaint();
        IplImage *sobeled = cvCreateImage(cvGetSize(frame),IPL_DEPTH_16S,1);
        IplImage *sobeled2 = cvCreateImage(cvGetSize(frame),IPL_DEPTH_8U,1);
        IplImage *sobeled3 = cvCreateImage(cvGetSize(frame),IPL_DEPTH_8U,1);
        cvSobel(grayscale,sobeled,1,0,3);
        cvConvertScale(sobeled,sobeled2,1./256,0);
        cvThreshold(sobeled2,sobeled3,0.999,255,CV_THRESH_BINARY);
        QImage *sobImage = Utils::IplImage2QImage(sobeled3);
        ui->label_3->setPixmap(QPixmap::fromImage(*sobImage));
        ui->label_3->repaint();
        Mat mtx(sobeled3);
        IplImage *gauss = cvCreateImage(cvGetSize(frame),IPL_DEPTH_8U,1);
        Mat mtx2(gauss);
        GaussianBlur(mtx,mtx2,cvSize(5,5),0,0);
        QImage *gaussImage = Utils::IplImage2QImage(gauss);
        ui->label_4->setPixmap(QPixmap::fromImage(*gaussImage));
        ui->label_4->repaint();
        delete qImage;
        delete gsImage;
        delete sobImage;
        delete gaussImage;


    }
    else
    {
        timer->stop();
        ui->label->clear();
    }
    //cvReleaseImage(&frame);
}

void MainWindow::on_actionPlay_triggered()
{
    timer->start(1000);
}

void MainWindow::on_actionPause_triggered()
{
    timer->stop();
}

