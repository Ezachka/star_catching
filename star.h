#ifndef STAR_H
#define STAR_H


#include <QFileDialog>
#include <QMessageBox>
#include <QImage>
#include <QPixmap>
#include <QTableWidget>
#include <QHeaderView>


#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

struct star_pixel {
    int x;
    int y;
    uchar intensity;
};

struct star {
    int id;
    std::vector<star_pixel> pixels;
    cv::Point2d centerOfMass;
};
void draw_star_markers(cv::Mat& image, const std::vector<star>& stars);
std::vector<star> collect_stars(const cv::Mat& imgOriginal,
                                const cv::Mat& labels,
                                const cv::Mat& centroids,
                                cv::Mat& imgWithCenters);
void print_stars_info(const std::vector<star>& stars, int max_pixels_per_star);
void print_stars_info(const std::vector<star>& stars);
void fill_star_table(QTableWidget* table, const std::vector<star>& stars);
void saveTableToCSV(QTableWidget* table, const QString& filePath);

#endif // STAR_H
