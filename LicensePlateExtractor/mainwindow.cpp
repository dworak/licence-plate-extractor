#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTimer>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/opencv.hpp>

#include "utils.h"

#include <QDebug>

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

    ui->leftView->setText("");
    ui->rightView->setText("");

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(nextFrame()));

    original = NULL;
    leftView = ORIGINAL;
    rightView = GRAY_SCALE;

    capture = cvCaptureFromFile("../Sample/MVI_1022.AVI");
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

    if(original != NULL){
        delete original;
        delete grayScale;
        delete sobel;
        delete sobelThreshold;
        delete gauss;
        delete matchFilter;
        delete matchFilterThreshold;
    }

    if( frame != NULL ){
        // parameters
        int sobelAperture = Utils::makeOdd(ui->sobelAperture->value());
        ui->sobelXorder->setMaximum(sobelAperture - 1);
        ui->sobelYorder->setMaximum(sobelAperture - 1);
        int sobelXorder = ui->sobelXorder->value();
        int sobelYorder = ui->sobelYorder->value();
        int sobelThresholdParam = ui->sobelThreshold->value();

        int gaussW = Utils::makeOdd(ui->gaussW->value());
        int gaussH = Utils::makeOdd(ui->gaussH->value());

        int mfM = ui->mfM->value();
        int mfN = ui->mfN->value();
        double mfVariance = ui->mfVariance->value();
        double mfA = ui->mfA->value();
        double mfB = ui->mfB->value();
        int mfThreshold = ui->mfThreshold->value();

        // original
        original = Utils::IplImage2QImage(frame);

        // gray scale
        IplImage *grayscale = cvCreateImage(cvGetSize(frame),IPL_DEPTH_8U,1);
        cvCvtColor(frame,grayscale,CV_RGB2GRAY);
        grayScale = Utils::IplImage2QImage(grayscale);

        // sobel
        IplImage *sobeled = cvCreateImage(cvGetSize(frame),IPL_DEPTH_16S,1);
        IplImage *sobeled2 = cvCreateImage(cvGetSize(frame),IPL_DEPTH_8U,1);
        IplImage *sobeled3 = cvCreateImage(cvGetSize(frame),IPL_DEPTH_8U,1);
        cvSobel(grayscale,sobeled,sobelXorder,sobelYorder,sobelAperture);
        sobel = Utils::IplImage2QImage(sobeled);

        // sobel threshold
        cvConvertScale(sobeled,sobeled2,1./256,0);
        cvThreshold(sobeled2,sobeled3,sobelThresholdParam,255,CV_THRESH_BINARY);
        sobelThreshold = Utils::IplImage2QImage(sobeled3);

        // gauss
        Mat mtx(sobeled3);
        IplImage *gaussipl = cvCreateImage(cvGetSize(frame),IPL_DEPTH_8U,1);
        Mat mtx2(gaussipl);
        GaussianBlur(mtx,mtx2,cvSize(gaussW,gaussH),0,0);
        gauss = Utils::IplImage2QImage(gaussipl);

        // match filter
        Mat mf1(gaussipl);
        IplImage *mf = cvCreateImage(cvGetSize(frame),IPL_DEPTH_8U,1);
        Mat mf2(mf);
        filter2D(mf1, mf2, -1, Utils::getMatchFilterKernel(mfM, mfN, mfVariance, mfA, mfB));
        matchFilter = Utils::IplImage2QImage(mf);

        // match filter threshold
        IplImage *mft = cvCreateImage(cvGetSize(frame),IPL_DEPTH_8U,1);
        cvThreshold(mf,mft,mfThreshold,255,CV_THRESH_BINARY);
        matchFilterThreshold = Utils::IplImage2QImage(mft);

        cvReleaseImage(&grayscale);
        cvReleaseImage(&sobeled);
        cvReleaseImage(&sobeled2);
        cvReleaseImage(&sobeled3);
        cvReleaseImage(&gaussipl);
        cvReleaseImage(&mf);
        cvReleaseImage(&mft);
    }
    else
    {
        timer->stop();
        ui->leftView->clear();
        ui->rightView->clear();
    }

    updateBothViews();

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

void MainWindow::on_leftView_customContextMenuRequested(const QPoint &pos)
{
    View view = showViewContextMenu(ui->leftView->mapToGlobal(pos));
    if(view != PREVIOUS){
        leftView = view;
        updateLeftView();
    }
}

void MainWindow::on_rightView_customContextMenuRequested(const QPoint &pos)
{
    View view = showViewContextMenu(ui->rightView->mapToGlobal(pos));
    if(view != PREVIOUS){
        rightView = view;
        updateRightView();
    }
}

MainWindow::View MainWindow::showViewContextMenu(const QPoint &pos)
{
    QMenu menu;
    menu.addAction("Original");
    menu.addAction("Gray scale");
    menu.addAction("Sobel filter");
    menu.addAction("Threshold after Sobel");
    menu.addAction("Gaussian blur");
    menu.addAction("Match Filter");
    menu.addAction("Threshold after MF");

    QAction* selectedItem = menu.exec(pos);
    if(selectedItem){
        if(selectedItem->text() == "Original")
            return ORIGINAL;
        else if(selectedItem->text() == "Gray scale")
            return GRAY_SCALE;
        else if(selectedItem->text() == "Sobel filter")
            return SOBEL;
        else if(selectedItem->text() == "Threshold after Sobel")
            return SOBEL_THRESHOLD;
        else if(selectedItem->text() == "Gaussian blur")
            return GAUSS;
        else if(selectedItem->text() == "Match Filter")
            return MATCH_FILTER;
        else if(selectedItem->text() == "Threshold after MF")
            return MATCH_FILTER_THRESHOLD;
    }

    return PREVIOUS;
}

void MainWindow::updateLeftView()
{
    QImage *img = NULL;
    switch(leftView){
    case ORIGINAL:
        img = original;
        break;
    case GRAY_SCALE:
        img = grayScale;
        break;
    case SOBEL:
        img = sobel;
        break;
    case SOBEL_THRESHOLD:
        img = sobelThreshold;
        break;
    case GAUSS:
        img = gauss;
        break;
    case MATCH_FILTER:
        img = matchFilter;
        break;
    case MATCH_FILTER_THRESHOLD:
        img = matchFilterThreshold;
        break;
    default:;
    }
    ui->leftView->setPixmap(QPixmap::fromImage(*img));
    ui->leftView->repaint();
}

void MainWindow::updateRightView()
{
    QImage *img = NULL;
    switch(rightView){
    case ORIGINAL:
        img = original;
        break;
    case GRAY_SCALE:
        img = grayScale;
        break;
    case SOBEL:
        img = sobel;
        break;
    case SOBEL_THRESHOLD:
        img = sobelThreshold;
        break;
    case GAUSS:
        img = gauss;
        break;
    case MATCH_FILTER:
        img = matchFilter;
        break;
    case MATCH_FILTER_THRESHOLD:
        img = matchFilterThreshold;
        break;
    default:;
    }
    ui->rightView->setPixmap(QPixmap::fromImage(*img));
    ui->rightView->repaint();
}

void MainWindow::updateBothViews()
{
    updateLeftView();
    updateRightView();
}
