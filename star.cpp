#include "star.h"
#include <QDebug>
#include <opencv2/imgproc.hpp> // обязательно для putText

std::vector<star> collect_stars(const cv::Mat& imgOriginal,
                                const cv::Mat& labels,
                                const cv::Mat& centroids,
                                cv::Mat& imgWithCenters)
{
    std::vector<star> stars;
    int nLabels = centroids.rows;

    for (int i = 1; i < nLabels; ++i) {
        star current;
        current.id = i;
        current.centerOfMass = cv::Point2d(
            centroids.at<double>(i, 0),
            centroids.at<double>(i, 1)
            );

        for (int y = 0; y < labels.rows; ++y) {
            for (int x = 0; x < labels.cols; ++x) {
                if (labels.at<int>(y, x) == i) {
                    star_pixel px;
                    px.x = x;
                    px.y = y;
                    px.intensity = imgOriginal.at<uchar>(y, x);
                    current.pixels.push_back(px);
                }
            }
        }

        stars.push_back(current);

        // Нарисовать центр масс
        cv::circle(imgWithCenters,
                   cv::Point((int)current.centerOfMass.x, (int)current.centerOfMass.y),
                   3, cv::Scalar(0, 0, 255), -1);
    }

    return stars;
}


void print_stars_info(const std::vector<star>& stars, int max_pixels_per_star){
    for (const star& s : stars) {
        qDebug() << "Звезда #" << s.id;
        qDebug() << "  Центр масс: (" << s.centerOfMass.x << "," << s.centerOfMass.y << ")";
        qDebug() << "  Кол-во пикселей:" << s.pixels.size();

        int count = 0;
        for (const star_pixel& px : s.pixels) {
            if (count++ >= max_pixels_per_star) break;
            qDebug() << "    Пиксель -> X:" << px.x
                     << "Y:" << px.y
                     << "Интенсивность:" << static_cast<int>(px.intensity);
        }

        if (s.pixels.size() > max_pixels_per_star) {
            qDebug() << "    ...и ещё" << (s.pixels.size() - max_pixels_per_star) << "пикселей";
        }

        qDebug() << "---------------------------------------------";
    }
}
void print_stars_info(const std::vector<star>& stars){
    for (const star& s : stars) {
        // Вычисление средней интенсивности
        double total_intensity = 0.0;
        for (const star_pixel& px : s.pixels) {
            total_intensity += static_cast<double>(px.intensity);
        }
        double average_intensity = s.pixels.empty() ? 0.0 : total_intensity / s.pixels.size();

        // Вывод информации
        qDebug() << "Звезда #" << s.id;
        qDebug() << "  Центр масс: (" << s.centerOfMass.x << "," << s.centerOfMass.y << ")";
        qDebug() << "  Кол-во пикселей:" << s.pixels.size();
        qDebug() << "  Средняя интенсивность:" << average_intensity;
        qDebug() << "---------------------------------------------";
    }
}
void fill_star_table(QTableWidget* table, const std::vector<star>& stars){
    table->clear();
    table->setRowCount(static_cast<int>(stars.size()));
    table->setColumnCount(4);
    QStringList headers = {"ID", "Центр масс", "Пикселей", "Сред. интенсивность"};
    table->setHorizontalHeaderLabels(headers);

    for (int i = 0; i < static_cast<int>(stars.size()); ++i) {
        const star& s = stars[i];

        double sumIntensity = 0;
        for (const auto& p : s.pixels)
            sumIntensity += p.intensity;
        double avgIntensity = sumIntensity / s.pixels.size();

        table->setItem(i, 0, new QTableWidgetItem(QString::number(s.id)));
        table->setItem(i, 1, new QTableWidgetItem(QString("(%1, %2)").arg(s.centerOfMass.x, 0, 'f', 1).arg(s.centerOfMass.y, 0, 'f', 1)));
        table->setItem(i, 2, new QTableWidgetItem(QString::number(s.pixels.size())));
        table->setItem(i, 3, new QTableWidgetItem(QString::number(avgIntensity, 'f', 1)));
    }

    table->resizeColumnsToContents();
    table->horizontalHeader()->setStretchLastSection(true);
}
void draw_star_markers(cv::Mat& image, const std::vector<star>& stars){

    cv::Point origin(0, 0); // Нулевая точка (0, 0)
    cv::circle(image, origin, 5, cv::Scalar(255, 255, 255), -1);  // Белый кружок на (0, 0)
    // Стрелка по оси X
    cv::arrowedLine(image, origin, cv::Point(100, 0), cv::Scalar(255, 0, 0), 1, 8, 0, 0.1);
    // Стрелка по оси Y
    cv::arrowedLine(image, origin, cv::Point(0, 100), cv::Scalar(0, 255, 0), 1, 8, 0, 0.1);

    // Рисуем метки для осей
    cv::putText(image, "X", cv::Point(110, 10), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 1);
    cv::putText(image, "Y", cv::Point(10, 110), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);
    for (const auto& s : stars) {
        cv::Point center(static_cast<int>(s.centerOfMass.x), static_cast<int>(s.centerOfMass.y));

        // Рисуем красный кружок
        cv::circle(image, center, 3, cv::Scalar(0, 0, 255), -1);

        // Добавляем номер звезды (ID)
        cv::putText(image,
                    QString::number(s.id).toStdString(),
                    center + cv::Point(5, -5), // небольшое смещение, чтобы не перекрывалось с кружком
                    cv::FONT_HERSHEY_SIMPLEX,
                    0.4,                       // размер шрифта
                    cv::Scalar(0, 255, 0),     // цвет (зелёный)
                    1);                        // толщина
    }
}
