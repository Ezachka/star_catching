#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <opencv2/opencv.hpp>  // Добавлено для использования cv::Mat

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void loadImage();
    void processImage();
    void saveImage();

    void on_light_min_cursorPositionChanged(int arg1, int arg2);

    void on_light_min_editingFinished();

private:
    void showImage(const cv::Mat &img);  // Приватный метод для отображения

    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
