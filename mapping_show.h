#ifndef MAPPING_SHOW_H
#define MAPPING_SHOW_H

#include <QWidget>
#include <opencv2/opencv.hpp>

namespace Ui {
class mapping_show;
}

extern cv::Mat finalImage_show[600];

class mapping_show : public QWidget
{
    Q_OBJECT

public:
    explicit mapping_show(QWidget *parent = nullptr);
    ~mapping_show();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::mapping_show *ui;
};

#endif // MAPPING_SHOW_H
