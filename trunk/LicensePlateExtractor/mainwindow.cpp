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
    playing = false;
    frameDelay = ui->frameDelay->value();
    connect(timer, SIGNAL(timeout()), this, SLOT(nextFrame()));

    sobelAperture = Utils::makeOdd(ui->sobelAperture->value());
    ui->sobelXorder->setMaximum(sobelAperture - 1);
    ui->sobelYorder->setMaximum(sobelAperture - 1);
    sobelXorder = ui->sobelXorder->value();
    sobelYorder = ui->sobelYorder->value();
    sobelThresholdParam = ui->sobelThreshold->value();
    gaussW = Utils::makeOdd(ui->gaussW->value());
    gaussH = Utils::makeOdd(ui->gaussH->value());
    mfM = ui->mfM->value();
    mfN = ui->mfN->value();
    mfVariance = ui->mfVariance->value();
    mfA = ui->mfA->value();
    mfB = ui->mfB->value();
    mfThreshold = ui->mfThreshold->value();

    original = NULL;
    leftView = ORIGINAL;
    rightView = GRAY_SCALE;

    capture = cvCaptureFromFile("../Sample/MVI_1022.AVI");
    frame = NULL;
}

MainWindow::~MainWindow()
{
    cvReleaseCapture(&capture);
    delete ui;
}

void MainWindow::nextFrame()
{
    frame = skipNFrames(capture, 0);

    if(frame != NULL){
        processCurrentFrame();
    }
    else
    {
        timer->stop();
        ui->leftView->clear();
        ui->rightView->clear();
        ui->playPause->setIcon(QIcon(":/icon/play"));
    }
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
    menu.addAction("Locate Plates");

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
        else if(selectedItem->text() == "Locate Plates")
            return PLATE_LOCALIZATION;
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
    case PLATE_LOCALIZATION:
        img = combined;
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
    case PLATE_LOCALIZATION:
        img = combined;
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

void MainWindow::processCurrentFrame()
{
    if(frame != NULL){
        if(original != NULL){
            delete original;
            delete grayScale;
            delete sobel;
            delete sobelThreshold;
            delete gauss;
            delete matchFilter;
            delete matchFilterThreshold;
            delete combined;
        }

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
        cvConvertScale(sobeled,sobeled2,1./256,0);
        sobel = Utils::IplImage2QImage(sobeled2);

        // sobel threshold
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

        //combined with original
        IplImage *cmb;
        cmb=cvCloneImage(frame);
        for (int y=0;y<cmb->height;y++)
            for (int x=9;x<cmb->width;x++)
            {
                int G = ((uchar *)(mft->imageData +y*mft->widthStep))[x*mft->nChannels +1];
                if (G==255)
                {
                    ((uchar *)(cmb->imageData +y*cmb->widthStep))[x*cmb->nChannels +0]=0;
                    ((uchar *)(cmb->imageData +y*cmb->widthStep))[x*cmb->nChannels +1]=255;
                    ((uchar *)(cmb->imageData +y*cmb->widthStep))[x*cmb->nChannels +2]=0;
                }
            }
        combined = Utils::IplImage2QImage(cmb);

        cvReleaseImage(&grayscale);
        cvReleaseImage(&sobeled);
        cvReleaseImage(&sobeled2);
        cvReleaseImage(&sobeled3);
        cvReleaseImage(&gaussipl);
        cvReleaseImage(&mf);
        cvReleaseImage(&mft);
        cvReleaseImage(&cmb);

        updateBothViews();
    }
}

void MainWindow::on_playPause_clicked()
{
    if(playing){
        timer->stop();
        ui->playPause->setIcon(QIcon(":/icon/play"));
    }
    else{
        timer->start(frameDelay);
        ui->playPause->setIcon(QIcon(":/icon/pause"));
    }
    playing = !playing;
}

void MainWindow::on_frameDelay_valueChanged(int arg1)
{
    frameDelay = arg1;
    if(playing)
        timer->setInterval(frameDelay);
}

void MainWindow::on_sobelAperture_valueChanged(int arg1)
{
    sobelAperture = arg1;
    ui->sobelXorder->setMaximum(sobelAperture - 1);
    ui->sobelYorder->setMaximum(sobelAperture - 1);
    processCurrentFrame();
}

void MainWindow::on_sobelXorder_valueChanged(int arg1)
{
    if(arg1 == 0)
        ui->sobelYorder->setMinimum(1);
    else{
        sobelXorder = arg1;
        ui->sobelYorder->setMinimum(0);
        processCurrentFrame();
    }
}

void MainWindow::on_sobelYorder_valueChanged(int arg1)
{
    if(arg1 == 0)
        ui->sobelXorder->setMinimum(1);
    else{
        sobelYorder = arg1;
        ui->sobelXorder->setMinimum(0);
        processCurrentFrame();
    }
}

void MainWindow::on_sobelThreshold_valueChanged(int arg1)
{
    sobelThresholdParam = arg1;
    processCurrentFrame();
}

void MainWindow::on_gaussW_valueChanged(int arg1)
{
    gaussW = arg1;
    processCurrentFrame();
}

void MainWindow::on_gaussH_valueChanged(int arg1)
{
    gaussH = arg1;
    processCurrentFrame();
}

void MainWindow::on_mfM_valueChanged(int arg1)
{
    mfM = arg1;
    processCurrentFrame();
}

void MainWindow::on_mfN_valueChanged(int arg1)
{
    mfN = arg1;
    processCurrentFrame();
}

void MainWindow::on_mfA_valueChanged(double arg1)
{
    mfA = arg1;
    processCurrentFrame();
}

void MainWindow::on_mfB_valueChanged(double arg1)
{
    mfB = arg1;
    processCurrentFrame();
}

void MainWindow::on_mfVariance_valueChanged(double arg1)
{
    mfVariance = arg1;
    processCurrentFrame();
}

void MainWindow::on_mfThreshold_valueChanged(int arg1)
{
    mfThreshold = arg1;
    processCurrentFrame();
}
