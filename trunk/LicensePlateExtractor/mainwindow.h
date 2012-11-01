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
    
private slots:
    void nextFrame();
    void on_actionPlay_triggered();
    void on_actionPause_triggered();

private:
    Ui::MainWindow *ui;
    CvCapture* capture;
    QTimer *timer;
};

#endif // MAINWINDOW_H
