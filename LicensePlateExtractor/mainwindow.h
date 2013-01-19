#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <opencv2/opencv.hpp>

#include "utils.h"
#include "detectedframe.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    enum View{PREVIOUS, ORIGINAL, GRAY_SCALE, SOBEL, SOBEL_THRESHOLD, GAUSS, MATCH_FILTER, MATCH_FILTER_THRESHOLD, PLATE_LOCALIZATION};
    
private slots:
    void nextFrame();
    void showLicensePlate(int x, int y);

    void on_rightView_customContextMenuRequested(const QPoint &pos);
    void on_leftView_customContextMenuRequested(const QPoint &pos);
    void on_playPause_clicked();
    void on_frameDelay_valueChanged(int arg1);
    void on_sobelAperture_valueChanged(int arg1);
    void on_sobelXorder_valueChanged(int arg1);
    void on_sobelYorder_valueChanged(int arg1);
    void on_sobelThreshold_valueChanged(int arg1);
    void on_gaussW_valueChanged(int arg1);
    void on_gaussH_valueChanged(int arg1);
    void on_mfM_valueChanged(int arg1);
    void on_mfN_valueChanged(int arg1);
    void on_mfA_valueChanged(double arg1);
    void on_mfB_valueChanged(double arg1);
    void on_mfThreshold_valueChanged(int arg1);
    void on_mfSD_valueChanged(double arg1);
    void on_areaThreshold_valueChanged(int arg1);
    void on_ratioThreshold_valueChanged(double arg1);
    void on_forward_clicked();
    void on_backward_clicked();
    void on_currentFrame_valueChanged(int arg1);
    void on_stop_clicked();
    void on_open_clicked();
    void on_playerSlider_valueChanged(int value);

private:
    Ui::MainWindow *ui;
    cv::VideoCapture capture;
    int frameCount;
    cv::Mat frame;
    int currentFrame;
    QList<cv::Rect> lpRects;
    int currentLicensePlate;
    Patterns patternsL, patternsR, patternsLR;
    QTimer *timer;
    detectedframe detection;

    // parameters
    bool playing;
    int frameDelay;

    int sobelXorder;
    int sobelYorder;
    int sobelAperture;
    int sobelThresholdParam;
    int gaussW;
    int gaussH;
    int mfM;
    int mfN;
    double mfSD;
    double mfA;
    double mfB;
    int mfThreshold;
    int rectAreaThreshold;
    double rectRatioThreshold;


    cv::Mat grayScale;
    cv::Mat sobel;
    cv::Mat sobelThreshold;
    cv::Mat gauss;
    cv::Mat matchFilter;
    cv::Mat matchFilterThreshold;
    cv::Mat combined;

    View leftView;
    View rightView;

    View showViewContextMenu(const QPoint &pos);
    void updateLeftView();
    void updateRightView();
    void updateBothViews();

    void drawLPRects();
    void showCurrentLicensePlate();
    void showCurrentLicensePlate2();
    QList<QPair<char,double> > getPlateCharsAndProbabs(cv::Rect plate, int& divisionPoint);
    void goToFrame(int index);
    void processCurrentFrame();
};

#endif // MAINWINDOW_H
