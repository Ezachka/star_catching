#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QImage>
#include <QPixmap>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

cv::Mat imgOriginal, imgWithCenters;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->btnLoad, &QPushButton::clicked, this, &MainWindow::loadImage);
    connect(ui->btnProcess, &QPushButton::clicked, this, &MainWindow::processImage);
    connect(ui->btnSave, &QPushButton::clicked, this, &MainWindow::saveImage);
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::loadImage()
{

    QString filename = QFileDialog::getOpenFileName(this, "Выберите изображение", "", "Images (*.jpg *.png *.bmp)");
    if (filename.isEmpty()) return;

    imgOriginal = cv::imread(filename.toStdString(), cv::IMREAD_GRAYSCALE);
    if (imgOriginal.empty()) {
        QMessageBox::warning(this, "Ошибка", "Не удалось загрузить изображение.");
        return;
    }

    showImage(imgOriginal);
}

void MainWindow::processImage()
{
    if (imgOriginal.empty()) return;

    QString light_minimum = ui->light_min->text();
    QString light_maximum = ui->light_max->text();
    bool ok1, ok2;
    double min_value = light_minimum.toDouble(&ok1);
    double max_value = light_maximum.toDouble(&ok2);
    if(max_value>255){max_value=255;}

    cv::Mat bin;
    cv::threshold(imgOriginal, bin, min_value, max_value, cv::THRESH_BINARY);

    cv::Mat labels, stats, centroids;
    int nLabels = cv::connectedComponentsWithStats(bin, labels, stats, centroids);

    cv::cvtColor(imgOriginal, imgWithCenters, cv::COLOR_GRAY2BGR);

    for (int i = 1; i < nLabels; ++i) {
        double cx = centroids.at<double>(i, 0);
        double cy = centroids.at<double>(i, 1);
        cv::circle(imgWithCenters, cv::Point((int)cx, (int)cy), 1, cv::Scalar(0, 0, 255), -1);
    }


    showImage(imgWithCenters);
}

void MainWindow::saveImage()
{
    if (imgWithCenters.empty()) return;

    QString filename = QFileDialog::getSaveFileName(this, "Сохранить изображение", "", "JPEG (*.jpg);;PNG (*.png)");
    if (filename.isEmpty()) return;

    cv::imwrite(filename.toStdString(), imgWithCenters);
}

void MainWindow::showImage(const cv::Mat &img)
{
    QImage qimg(img.data, img.cols, img.rows, img.step, img.channels() == 1 ? QImage::Format_Grayscale8 : QImage::Format_BGR888);
    ui->imageLabel->setPixmap(QPixmap::fromImage(qimg).scaled(ui->imageLabel->size(), Qt::KeepAspectRatio));
}




