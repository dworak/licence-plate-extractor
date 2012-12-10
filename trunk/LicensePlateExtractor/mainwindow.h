#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <opencv2/highgui/highgui.hpp>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    enum View{PREVIOUS, ORIGINAL, GRAY_SCALE, SOBEL, SOBEL_THRESHOLD, GAUSS, MATCH_FILTER, MATCH_FILTER_THRESHOLD,PLATE_LOCALIZATION};
    
private slots:
    void nextFrame();

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

    void on_mfVariance_valueChanged(double arg1);

    void on_mfThreshold_valueChanged(int arg1);

private:
    Ui::MainWindow *ui;
    CvCapture* capture;
    IplImage* frame;
    QTimer *timer;

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
    double mfVariance;
    double mfA;
    double mfB;
    int mfThreshold;


    QImage *original;
    QImage *grayScale;
    QImage *sobel;
    QImage *sobelThreshold;
    QImage *gauss;
    QImage *matchFilter;
    QImage *matchFilterThreshold;
    QImage *combined;

    View leftView;
    View rightView;

    View showViewContextMenu(const QPoint &pos);
    void updateLeftView();
    void updateRightView();
    void updateBothViews();

    void processCurrentFrame();
};

#endif // MAINWINDOW_H
