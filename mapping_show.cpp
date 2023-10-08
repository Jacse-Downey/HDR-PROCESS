#include "mapping_show.h"
#include "ui_mapping_show.h"
#include <Eigen/Dense>
#include <QFileDialog>
#include <opencv2/opencv.hpp>
#include "QLabel"
#include <QMouseEvent>
#include "mainwindow.h"
#include <opencv2/opencv.hpp>
#include <QVariant>
#include <QVariantList>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QAxObject>
#include <QtCharts>
#include <QtWidgets>
#include <QtCharts/QChartView>
#include <fstream>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <sstream>
#include <algorithm>
#include <random>
#include <unsupported/Eigen/Splines>
#include "caijian.h"
#include "quxian.h"
#include "yingshe.h"
#include "process.h"

int flag_mapping=0;
cv::Mat finalImage_show[600];
mapping_show::mapping_show(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::mapping_show)
{
    ui->setupUi(this);
    // 在某个适当的位置进行初始化
    for (int i = 0; i < 600; ++i) {
        // 初始化每个元素，假设图像大小为 width x height，并且每个像素初始化为0（黑色）
        finalImage_show[i] = cv::Mat::zeros(1024, 1360, CV_8UC1); // 初始化为黑色图像
    }
    // 2. 遍历图像中的每个像素
    for(int loop=0;loop<=flag_dst;loop++){
    for (int x = 0; x < finalImage[loop].rows; x++) {
        for (int y = 0; y < finalImage[loop].cols; y++) {
            // 获取当前像素的灰度值作为 x 值
            double xValue = static_cast<double>(finalImage_map[loop].at<float>(x,y));
            int index = (xValue/maxvalue)*10000;
            // 3. 使用拟合曲线的公式计算 y 值作为映射后的像素值
            double yValue = csvfile1[index][1];
            // 4. 创建新的图像并设置像素值
            finalImage_show[loop].at<uchar>(x,y) = (uchar)yValue;
        }
    }
    }
    // 5. 显示映射后的图像
    /*QImage image1((const unsigned char*)(finalImage_show[flag_mapping].data), finalImage_show[flag_mapping].cols, finalImage_show[flag_mapping].rows, QImage::Format_Grayscale8);
    QPixmap pixmap1 = QPixmap::fromImage(image1).scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label->setPixmap(pixmap1);*/

    // 设置初始图像显示大小
    int initialWidth = 1360; // 设置初始宽度
    int initialHeight = 1024; // 设置初始高度

    // 显示初始图像
    QImage initialImage((const unsigned char*)(finalImage_show[flag_mapping].data),
                        finalImage_show[flag_mapping].cols, finalImage_show[flag_mapping].rows, QImage::Format_Grayscale8);

    QPixmap initialPixmap = QPixmap::fromImage(initialImage).scaled(initialWidth, initialHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label->setPixmap(initialPixmap);

}

mapping_show::~mapping_show()
{
    delete ui;
}

//上一张
void mapping_show::on_pushButton_clicked()
{
    if(flag_mapping==0){
        QMessageBox::warning(this, tr("提示！"), tr("当前已经是第一张图像了！"));
        if (this->parent()) {   // 检查父窗口是否存在
            this->close();      // 关闭当前窗口
            QCoreApplication::processEvents(); //处理所有未完成的事件
            this->parentWidget()->show();      // 显示父窗口
        }
        return;
    }
    else{
        flag_mapping--;
        QImage image1((const unsigned char*)(finalImage_show[flag_mapping].data), finalImage_show[flag_mapping].cols, finalImage_show[flag_mapping].rows, QImage::Format_Grayscale8);
        QPixmap pixmap1 = QPixmap::fromImage(image1).scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->label->setPixmap(pixmap1);
    }


}

//下一张
void mapping_show::on_pushButton_2_clicked()
{
    if(flag_mapping==flag_dst){
        QMessageBox::warning(this, tr("提示！"), tr("当前已经是最后一张图像了！"));
        if (this->parent()) {   // 检查父窗口是否存在
            this->close();      // 关闭当前窗口
            QCoreApplication::processEvents(); //处理所有未完成的事件
            this->parentWidget()->show();      // 显示父窗口
        }
        return;
    }
    else{
        flag_mapping++;
        QImage image1((const unsigned char*)(finalImage_show[flag_mapping].data), finalImage_show[flag_mapping].cols, finalImage_show[flag_mapping].rows, QImage::Format_Grayscale8);
        QPixmap pixmap1 = QPixmap::fromImage(image1).scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->label->setPixmap(pixmap1);
    }


}

