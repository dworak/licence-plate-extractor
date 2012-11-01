#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTimer>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>

#include "utils.h"

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

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(nextFrame()));

    capture = cvCaptureFromFile("../Sample/mis.mp4");
    //
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
        delete qImage;
    }
    else{
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
