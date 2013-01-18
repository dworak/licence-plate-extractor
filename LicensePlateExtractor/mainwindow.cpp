#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QFontDatabase>
#include <QTimer>

#include <iostream>

#include <QDebug>

using namespace cv;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("License Plate Extractor");

    timer = new QTimer(this);
    playing = false;
    frameDelay = ui->frameDelay->value();
    connect(timer, SIGNAL(timeout()), this, SLOT(nextFrame()));
    connect(ui->leftView, SIGNAL(clicked(int,int)), this, SLOT(showLicensePlate(int,int)));
    connect(ui->rightView, SIGNAL(clicked(int,int)), this, SLOT(showLicensePlate(int,int)));

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
    rectAreaThreshold = ui->areaThreshold->value();
    rectRatioThreshold = ui->ratioThreshold->value();

    QFontDatabase::addApplicationFont(":/font/lpfont");
    ui->recognizedText->setText("");
    ui->recognizedText->setFont(QFont("arklas Tablica samochodowa", 24));

    leftView = PLATE_LOCALIZATION;
    rightView = ORIGINAL;

    Utils::preparePatterns(patternsL, Utils::POWIAT_CHARACTERISTIC);
    Utils::preparePatterns(patternsR, Utils::VEHICLE_CHARACTERISITC);
    Utils::preparePatterns(patternsLR, Utils::ALL);

    capture = VideoCapture("../Sample/MVI_1022.AVI");
    frameCount = capture.get(CV_CAP_PROP_FRAME_COUNT);
    currentFrame = -1;
    currentLicensePlate = -1;

    ui->currentFrame->setMaximum(frameCount);
    ui->playerSlider->setMaximum(frameCount);
    ui->totalFrames->setText(QString::number(frameCount));

    goToFrame(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::nextFrame()
{
    if(currentFrame < frameCount - 1){
        goToFrame(currentFrame + 1);
    }
    else
    {
        timer->stop();
        ui->leftView->clear();
        ui->rightView->clear();
        ui->playPause->setIcon(QIcon(":/icon/play"));
        ui->open->setEnabled(true);
        currentFrame = -1;
        goToFrame(0);
    }
}

void MainWindow::showLicensePlate(int x, int y)
{
    if(lpRects.size() > 0){
        int i = 0;
        while(i < lpRects.size() && !lpRects[i].contains(Point(x, y)))
            i++;
        if(i < lpRects.size()){
            currentLicensePlate = i;
            showCurrentLicensePlate();
            drawLPRects();
        }
    }
    updateBothViews();
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
    menu.addAction("Original with LP locations");

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
        else if(selectedItem->text() == "Original with LP locations")
            return PLATE_LOCALIZATION;
    }

    return PREVIOUS;
}

void MainWindow::updateLeftView()
{
    switch(leftView){
    case ORIGINAL:
        ui->leftView->showImage(frame);
        break;
    case GRAY_SCALE:
        ui->leftView->showImage(grayScale);
        break;
    case SOBEL:
        ui->leftView->showImage(sobel);
        break;
    case SOBEL_THRESHOLD:
        ui->leftView->showImage(sobelThreshold);
        break;
    case GAUSS:
        ui->leftView->showImage(gauss);
        break;
    case MATCH_FILTER:
        ui->leftView->showImage(matchFilter);
        break;
    case MATCH_FILTER_THRESHOLD:
        ui->leftView->showImage(matchFilterThreshold);
        break;
    case PLATE_LOCALIZATION:
        ui->leftView->showImage(combined);
        break;
    default:;
    }
}

void MainWindow::updateRightView()
{
    switch(rightView){
    case ORIGINAL:
        ui->rightView->showImage(frame);
        break;
    case GRAY_SCALE:
        ui->rightView->showImage(grayScale);
        break;
    case SOBEL:
        ui->rightView->showImage(sobel);
        break;
    case SOBEL_THRESHOLD:
        ui->rightView->showImage(sobelThreshold);
        break;
    case GAUSS:
        ui->rightView->showImage(gauss);
        break;
    case MATCH_FILTER:
        ui->rightView->showImage(matchFilter);
        break;
    case MATCH_FILTER_THRESHOLD:
        ui->rightView->showImage(matchFilterThreshold);
        break;
    case PLATE_LOCALIZATION:
        ui->rightView->showImage(combined);
        break;
    default:;
    }
}

void MainWindow::updateBothViews()
{
    updateLeftView();
    updateRightView();
}

void MainWindow::drawLPRects()
{
    for(int i=0; i < lpRects.size(); i++){
        if(i == currentLicensePlate)
            rectangle(combined, lpRects[i], Scalar(0, 255, 255));
        else
            rectangle(combined, lpRects[i], Scalar(0, 0, 255));
    }
}

void MainWindow::showCurrentLicensePlate()
{
    Mat lp = grayScale(lpRects[currentLicensePlate]).clone();
    cv::resize(lp, lp, cv::Size(100. * lp.cols / lp.rows, 100));
    Mat lpAfterAT;
    lp = Utils::getLPInterior(lp);
    QList<Rect> lpChRects = Utils::getLPCharactersRects(lp, lpAfterAT);
    QList<Rect> lpChRectsCopy = QList<Rect>(lpChRects);
    //lpChRectsCopy.append(lpChRects);

    QMap<int, cv::Rect> map;
        foreach(const cv::Rect &rect, lpChRects)
            map.insert(rect.x, rect);
        lpChRects = map.values();

    if(lpChRects.size() > 0){
        QString recognizedCharacters;
        if(lpChRects.size() < 7 || lpChRects.size() > 8){
            foreach(Rect r, lpChRects){
                Mat ch = lp(r);
                recognizedCharacters += Utils::recognizeCharacter(ch, patternsLR);
            }
        }
        else if(lpChRects.size() == 7){
            int divisionPoint = lpChRects[2].x - lpChRects[1].br().x > lpChRects[3].x - lpChRects[2].br().x ? 2 : 3;
            for(int i=0; i<divisionPoint; i++){
                Mat ch = lp(lpChRects[i]);
                recognizedCharacters += Utils::recognizeCharacter(ch, patternsL);
            }
            recognizedCharacters += " ";
            for(int i=divisionPoint; i<7; i++){
                Mat ch = lp(lpChRects[i]);
                recognizedCharacters += Utils::recognizeCharacter(ch, patternsR);
            }
        }
        else if(lpChRects.size() == 8){
            for(int i=0; i<3; i++){
                Mat ch = lp(lpChRects[i]);
                recognizedCharacters += Utils::recognizeCharacter(ch, patternsL);
            }
            recognizedCharacters += " ";
            for(int i=3; i<8; i++){
                Mat ch = lp(lpChRects[i]);
                recognizedCharacters += Utils::recognizeCharacter(ch, patternsR);
            }
        }

        ui->recognizedText->setText(recognizedCharacters);
    }
    else
        ui->recognizedText->setText("");



    cvtColor(lp, lp, CV_GRAY2BGR);
    int lastX = 0;
    Scalar color = Scalar(0, 255, 0);
    foreach(const Rect &rect, lpChRectsCopy){
        if(rect.x < lastX)
            color = Scalar(255, 0, 255);
        rectangle(lp, rect, color);
        lastX = rect.x;
    }

    ui->lpView->showImage(lp);
    ui->lpBWView->showImage(lpAfterAT);
}

void MainWindow::goToFrame(int index)
{
    if(index != currentFrame){
        currentFrame = index;
        ui->backward->setEnabled(currentFrame >= 1);
        ui->forward->setEnabled(currentFrame <= frameCount - 2);
        capture.set(CV_CAP_PROP_POS_FRAMES, index);
        capture >> frame;
        if(!frame.empty())
            processCurrentFrame();
        ui->currentFrame->setValue(index + 1);
        ui->playerSlider->setValue(index + 1);
    }
}

void MainWindow::processCurrentFrame()
{
    if(!frame.empty()){
        // gray scale
        cvtColor(frame,grayScale,CV_RGB2GRAY);

        // sobel
        Sobel(grayScale,sobel,CV_16S,sobelXorder,sobelYorder,sobelAperture,1./256);
        convertScaleAbs(sobel,sobel);

        // sobel threshold
        threshold(sobel,sobelThreshold,sobelThresholdParam,255,THRESH_BINARY);

        // gauss
        GaussianBlur(sobelThreshold,gauss,Size(gaussW,gaussH),0,0);

        // match filter
        filter2D(gauss, matchFilter, -1, Utils::getMatchFilterKernel(mfM, mfN, mfSD, mfA, mfB));

        // match filter threshold
        threshold(matchFilter,matchFilterThreshold,mfThreshold,255,THRESH_BINARY);

        // combined with original
        combined = frame.clone();
        for (int y=0;y<combined.rows;y++)
            for (int x=0;x<combined.cols;x++)
            {
                int G = ((uchar *)(matchFilterThreshold.data +y*matchFilterThreshold.step))[x*matchFilterThreshold.channels() +1];
                if (G==255)
                {
                    ((uchar *)(combined.data +y*combined.step))[x*combined.channels() +0]=0;
                    ((uchar *)(combined.data +y*combined.step))[x*combined.channels() +1]=255;
                    ((uchar *)(combined.data +y*combined.step))[x*combined.channels() +2]=0;
                }
            }

        // precise license plates rectangles
        lpRects = Utils::getLPRects(matchFilterThreshold, sobelThreshold, rectAreaThreshold, rectRatioThreshold);

        // license plate characters
        if(lpRects.size() > 0){
            currentLicensePlate = 0;
            showCurrentLicensePlate();
        }
        else{
            currentLicensePlate = -1;
            ui->lpView->clear();
            ui->lpBWView->clear();
            ui->recognizedText->setText("");
        }

        drawLPRects();
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
    ui->open->setEnabled(!playing);
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

void MainWindow::on_areaThreshold_valueChanged(int arg1)
{
    rectAreaThreshold = arg1;
    processCurrentFrame();
}

void MainWindow::on_ratioThreshold_valueChanged(double arg1)
{
    rectRatioThreshold = arg1;
    processCurrentFrame();
}

void MainWindow::on_forward_clicked()
{
    goToFrame(currentFrame + 1);
}

void MainWindow::on_backward_clicked()
{
    goToFrame(currentFrame - 1);
}

void MainWindow::on_currentFrame_valueChanged(int arg1)
{
    goToFrame(arg1 - 1);
}

void MainWindow::on_stop_clicked()
{
    timer->stop();
    ui->playPause->setIcon(QIcon(":/icon/play"));
    ui->open->setEnabled(true);
    goToFrame(0);
}

void MainWindow::on_open_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    if(!fileName.isEmpty()){
        capture = VideoCapture(fileName.toStdString());
        frameCount = capture.get(CV_CAP_PROP_FRAME_COUNT);
        currentFrame = -1;
        currentLicensePlate = -1;

        ui->currentFrame->setMaximum(frameCount);
        ui->playerSlider->setMaximum(frameCount);
        ui->totalFrames->setText(QString::number(frameCount));

        goToFrame(0);
    }
}

void MainWindow::on_playerSlider_valueChanged(int value)
{
    goToFrame(value - 1);
}
