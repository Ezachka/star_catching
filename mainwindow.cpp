#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "star.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QImage>
#include <QPixmap>
#include <QTableWidget>
#include <QHeaderView>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

cv::Mat imgOriginal, imgWithCenters;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->light_min->setText("50");

    connect(ui->btnLoad, &QPushButton::clicked, this, &MainWindow::loadImage);
    connect(ui->btnProcess, &QPushButton::clicked, this, &MainWindow::processImage);
    connect(ui->btnSave, &QPushButton::clicked, this, &MainWindow::saveImage);
    connect(ui->sortButton, &QPushButton::clicked, this, &MainWindow::on_sortButton_clicked);

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
    bool ok1;
    double min_value = light_minimum.toDouble(&ok1);

    cv::Mat bin;
    cv::threshold(imgOriginal, bin, min_value, 255, cv::THRESH_BINARY);

    cv::Mat labels, stats, centroids;
    cv::connectedComponentsWithStats(bin, labels, stats, centroids);

    cv::cvtColor(imgOriginal, imgWithCenters, cv::COLOR_GRAY2BGR);
    stars = collect_stars(imgOriginal, labels, centroids, imgWithCenters);
   // print_stars_info(stars, 5);
    print_stars_info(stars);
    fill_star_table(ui->tableWidget, stars);
    draw_star_markers(imgWithCenters, stars);
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





void MainWindow::on_sortButton_clicked()
{
    int criterion = ui->sortComboBox->currentIndex();

    std::sort(stars.begin(), stars.end(), [=](const star& a, const star& b) {
        switch (criterion) {
        case 2: // По координате X
            return a.centerOfMass.x > b.centerOfMass.x;
        case 3: // По координате Y
            return a.centerOfMass.y > b.centerOfMass.y;
        case 0: { // По интенсивности
            double sumA = 0, sumB = 0;
            for (const auto& p : a.pixels) sumA += p.intensity;
            for (const auto& p : b.pixels) sumB += p.intensity;
            double avgA = sumA / a.pixels.size();
            double avgB = sumB / b.pixels.size();
            return avgA > avgB;
        }
        case 1: // По количеству пикселей
            return a.pixels.size() > b.pixels.size();
        default:
            return false;
        }
    });

    // Обновить ID
    for (int i = 0; i < static_cast<int>(stars.size()); ++i) {
        stars[i].id = i + 1;
    }

    // Перерисовать изображение
    cv::cvtColor(imgOriginal, imgWithCenters, cv::COLOR_GRAY2BGR);
    draw_star_markers(imgWithCenters, stars);
    showImage(imgWithCenters);

    // Обновить таблицу
    fill_star_table(ui->tableWidget, stars);
}

