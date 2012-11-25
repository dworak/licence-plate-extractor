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

    enum View{PREVIOUS, ORIGINAL, GRAY_SCALE, SOBEL, SOBEL_THRESHOLD, GAUSS, MATCH_FILTER, MATCH_FILTER_THRESHOLD};
    
private slots:
    void nextFrame();
    void on_actionPlay_triggered();
    void on_actionPause_triggered();

    void on_rightView_customContextMenuRequested(const QPoint &pos);

    void on_leftView_customContextMenuRequested(const QPoint &pos);

private:
    Ui::MainWindow *ui;
    CvCapture* capture;
    QTimer *timer;

    QImage *original;
    QImage *grayScale;
    QImage *sobel;
    QImage *sobelThreshold;
    QImage *gauss;
    QImage *matchFilter;
    QImage *matchFilterThreshold;

    View leftView;
    View rightView;

    View showViewContextMenu(const QPoint &pos);
    void updateLeftView();
    void updateRightView();
    void updateBothViews();
};

#endif // MAINWINDOW_H
