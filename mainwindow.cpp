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

cv::Mat imgOriginal,imgBase, imgWithCenters;

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
    connect(ui->btnFilter, &QPushButton::clicked, this, &MainWindow::applyFilter);
    connect(ui->btnBack, &QPushButton::clicked, this, &MainWindow::resetAll);
    connect(ui->btnSave_table, &QPushButton::clicked, this, &MainWindow::save_table);

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
    imgBase = imgOriginal;
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
void MainWindow::save_table()
{
    // QString fileName = QFileDialog::getSaveFileName(this, "Сохранить таблицу", "", "CSV файлы (*.csv);;Все файлы (*.*)");
    // if (!fileName.isEmpty()) {
    //     saveTableToCSV(ui->tableWidget, fileName);
    // }

    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить данные звёзд", "", "Text Files (*.txt);;All Files (*)");
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл для записи.");
        return;
    }

    QTextStream out(&file);

    // Предположим, у вас есть вектор звёзд — например, this->stars
    for (const star& s : this->stars) {
        out << "Star ID: " << s.id << "\n";
        out << "Center of Mass: (" << s.centerOfMass.x << ", " << s.centerOfMass.y << ")\n";
        out << "Pixels (" << s.pixels.size() << "):\n";
        for (const star_pixel& p : s.pixels) {
            out << "  (" << p.x << ", " << p.y << ") Intensity: " << static_cast<int>(p.intensity) << "\n";
        }
        out << "\n";
    }

    file.close();
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
    if (imgOriginal.empty()) {
        QMessageBox::warning(this, "Ошибка", "Сначала загрузите изображение.");
        return;
    }

    int criterion = ui->sortComboBox->currentIndex();

    std::sort(stars.begin(), stars.end(), [=](const star& a, const star& b) {
        switch (criterion) {
        case 2: // По координате X
            return a.centerOfMass.x < b.centerOfMass.x;
        case 3: // По координате Y
            return a.centerOfMass.y < b.centerOfMass.y;
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
void MainWindow::applyFilter()
{
    if (imgOriginal.empty()) return;


    int selectedIndex = ui->filterComboBox->currentIndex();

    cv::Mat result = imgOriginal.clone();
    cv::Mat kernel, kern;

    switch (selectedIndex) {
    case 0: // Нет — оставить без изменений
        result = imgOriginal.clone();
        break;
    case 1: // Gaussian Blur
        cv::GaussianBlur(imgOriginal, result, cv::Size(5, 5), 1.5);
        break;
    case 2: // Median Blur
        cv::medianBlur(imgOriginal, result, 3);
        break;
    case 3: // Bilateral Filter
        cv::bilateralFilter(imgOriginal, result, 9, 75, 75);
        break;
    case 4: // Increase contrast
        imgOriginal.convertTo(result, -1, 1.1, 0); // 1.5 - коэффициент контраста
        break;
    case 5: // Decrease contrast
        imgOriginal.convertTo(result, -1, 0.9, 0);
        break;
    case 6: // Horizontal filter
        kernel = cv::Mat::ones(1, 4, CV_32F) / 5;
        cv::filter2D(imgOriginal, result, -1, kernel);
        break;
    case 7: // Vertical filter
        kern = cv::Mat::ones(4, 1, CV_32F) / 5;
        cv::filter2D(imgOriginal, result, -1, kern);
        break;
    case 8:
    {
        cv::GaussianBlur(imgOriginal, result, cv::Size(5, 5), 1.5);

        int threshold = 10;
        double alpha_high = 2;  // Коэффициент увеличения яркости для ярких пикселей
        double alpha_low = 0.1;   // Коэффициент уменьшения яркости для темных пикселей

        result = imgOriginal.clone(); // Клонируем изображение, чтобы работать с ним

        // Преобразуем изображение в формат, пригодный для обработки (если оно 8 бит)
        cv::Mat img_float;
        imgOriginal.convertTo(img_float, CV_32F);

        // Для каждого пикселя
        for (int y = 0; y < img_float.rows; ++y) {
            for (int x = 0; x < img_float.cols; ++x) {
                float& pixel = img_float.at<float>(y, x); // Яркость пикселя

                if (pixel > threshold) {
                    // Увеличиваем яркость для светлых пикселей
                    pixel = cv::saturate_cast<float>(pixel * alpha_high);
                } else {
                    // Уменьшаем яркость для темных пикселей
                    pixel = cv::saturate_cast<float>(pixel * alpha_low);
                }
            }
        }

        // Преобразуем обратно в 8-битное изображение
        img_float.convertTo(result, CV_8U);

    }
    case 9:
    {
        QString light_minimum = ui->light_min->text();
        bool ok1;
        int threshold = light_minimum.toInt(&ok1);

        if (!ok1) {
            qWarning() << "Ошибка преобразования порога яркости!";
            return;
        }

        double alpha_high = 1.5;  // Коэффициент увеличения яркости для ярких пикселей
        double alpha_low = 0.5;   // Коэффициент уменьшения яркости для темных пикселей

        result = imgOriginal.clone(); // Клонируем изображение, чтобы работать с ним

        // Преобразуем изображение в формат, пригодный для обработки (если оно 8 бит)
        cv::Mat img_float;
        imgOriginal.convertTo(img_float, CV_32F);

        // Для каждого пикселя
        for (int y = 0; y < img_float.rows; ++y) {
            for (int x = 0; x < img_float.cols; ++x) {
                float& pixel = img_float.at<float>(y, x); // Яркость пикселя

                if (pixel > threshold) {
                    // Увеличиваем яркость для светлых пикселей
                    pixel = cv::saturate_cast<float>(pixel * alpha_high);
                } else {
                    // Уменьшаем яркость для темных пикселей
                    pixel = cv::saturate_cast<float>(pixel * alpha_low);
                }
            }
        }

        // Преобразуем обратно в 8-битное изображение
        img_float.convertTo(result, CV_8U);

        break;
    }


    default:
        result = imgOriginal.clone();
        break;
    }

    imgOriginal = result.clone(); // Обновляем оригинал
    showImage(result);
}
void MainWindow::resetAll()
{
     imgOriginal.release();
     imgWithCenters.release();
     ui->imageLabel->clear();
     ui->tableWidget->clearContents();
     ui->tableWidget->setRowCount(0);
      stars.clear();
     ui->light_min->setText("50");
    imgOriginal=imgBase;
    showImage(imgOriginal);
}

void MainWindow::on_btnBack_clicked(){
}
void MainWindow::on_light_min_editingFinished(){

}

void MainWindow::on_light_min_cursorPositionChanged(int oldPos, int newPos){

}

