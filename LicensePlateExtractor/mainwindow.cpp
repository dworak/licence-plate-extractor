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


//IplImage* skipNFrames(CvCapture* capture, int n)
//{
//    for(int i = 0; i < n; ++i)
//    {
//        if(cvQueryFrame(capture) == NULL)
//        {
//            return NULL;
//        }
//    }

//    return cvQueryFrame(capture);
//}

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
    mfSD = ui->mfSD->value();
    mfA = ui->mfA->value();
    mfB = ui->mfB->value();
    mfThreshold = ui->mfThreshold->value();

    //original = NULL;
    leftView = ORIGINAL;
    rightView = GRAY_SCALE;

    capture = VideoCapture("../Sample/MVI_1022.AVI");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::nextFrame()
{
    capture >> frame;

    if(!frame.empty()){
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
    QImage *img;
    switch(leftView){
    case ORIGINAL:
        img = &original;
        break;
    case GRAY_SCALE:
        img = &grayScale;
        break;
    case SOBEL:
        img = &sobel;
        break;
    case SOBEL_THRESHOLD:
        img = &sobelThreshold;
        break;
    case GAUSS:
        img = &gauss;
        break;
    case MATCH_FILTER:
        img = &matchFilter;
        break;
    case MATCH_FILTER_THRESHOLD:
        img = &matchFilterThreshold;
        break;
    case PLATE_LOCALIZATION:
        img = &combined;
        break;
    default:;
    }
    ui->leftView->setPixmap(QPixmap::fromImage(*img));
    ui->leftView->repaint();
}

void MainWindow::updateRightView()
{
    QImage *img;
    switch(rightView){
    case ORIGINAL:
        img = &original;
        break;
    case GRAY_SCALE:
        img = &grayScale;
        break;
    case SOBEL:
        img = &sobel;
        break;
    case SOBEL_THRESHOLD:
        img = &sobelThreshold;
        break;
    case GAUSS:
        img = &gauss;
        break;
    case MATCH_FILTER:
        img = &matchFilter;
        break;
    case MATCH_FILTER_THRESHOLD:
        img = &matchFilterThreshold;
        break;
    case PLATE_LOCALIZATION:
        img = &combined;
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
    if(!frame.empty()){
        // original
        original = Utils::Mat2QImage(frame);

        // gray scale
        Mat grayScaleMat;
        cvtColor(frame,grayScaleMat,CV_RGB2GRAY);
        grayScale = Utils::Mat2QImage(grayScaleMat);

        // sobel
        Mat sobelMat;
        Sobel(grayScaleMat,sobelMat,CV_16S,sobelXorder,sobelYorder,sobelAperture,1./256);
        convertScaleAbs(sobelMat,sobelMat);
        sobel = Utils::Mat2QImage(sobelMat);

        // sobel threshold
        Mat sobelThresholdMat;
        threshold(sobelMat,sobelThresholdMat,sobelThresholdParam,255,THRESH_BINARY);
        sobelThreshold = Utils::Mat2QImage(sobelThresholdMat);

        // gauss
        Mat gaussMat;
        GaussianBlur(sobelThresholdMat,gaussMat,Size(gaussW,gaussH),0,0);
        gauss = Utils::Mat2QImage(gaussMat);

        // match filter
        Mat matchFilterMat;
        filter2D(gaussMat, matchFilterMat, -1, Utils::getMatchFilterKernel(mfM, mfN, mfSD, mfA, mfB));
        matchFilter = Utils::Mat2QImage(matchFilterMat);

        // match filter threshold
        Mat matchFilterThresholdMat;
        threshold(matchFilterMat,matchFilterThresholdMat,mfThreshold,255,THRESH_BINARY);
        matchFilterThreshold = Utils::Mat2QImage(matchFilterThresholdMat);

        //combined with original
        Mat combinedMat = frame.clone();
        for (int y=0;y<combinedMat.rows;y++)
            for (int x=9;x<combinedMat.cols;x++)
            {
                int G = ((uchar *)(matchFilterThresholdMat.data +y*matchFilterThresholdMat.step))[x*matchFilterThresholdMat.channels() +1];
                if (G==255)
                {
                    ((uchar *)(combinedMat.data +y*combinedMat.step))[x*combinedMat.channels() +0]=0;
                    ((uchar *)(combinedMat.data +y*combinedMat.step))[x*combinedMat.channels() +1]=255;
                    ((uchar *)(combinedMat.data +y*combinedMat.step))[x*combinedMat.channels() +2]=0;
                }
            }
        combined = Utils::Mat2QImage(combinedMat);

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

void MainWindow::on_mfThreshold_valueChanged(int arg1)
{
    mfThreshold = arg1;
    processCurrentFrame();
}

void MainWindow::on_mfSD_valueChanged(double arg1)
{
    mfSD = arg1;
    processCurrentFrame();
}
