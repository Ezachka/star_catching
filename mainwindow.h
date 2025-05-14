#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <opencv2/opencv.hpp>  // Добавлено для использования cv::Mat
#include "star.h"
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

    void on_sortButton_clicked();

    void on_btnBack_clicked();

private:
    void showImage(const cv::Mat &img);  // Приватный метод для отображения
    std::vector<star> stars;
    Ui::MainWindow *ui;
    void applyFilter();
    void resetAll();
    void save_table();
};

#endif // MAINWINDOW_H
