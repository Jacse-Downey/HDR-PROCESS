#include "caijian.h"
#include "ui_caijian.h"
#include <process.h>
#include "opencv2/core/mat.hpp"
#include <QMessageBox>
#include <QMouseEvent>
#include <opencv2/imgproc.hpp>

int storedwidth = 0;
int storedheight = 0;
int storedx = 0;
int storedy = 0;
caijian::caijian(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::caijian)
{
    ui->setupUi(this);

    // 初始化裁剪预览边框大小为 256x256
    previewWidth = 256;
    previewHeight = 256;
    previewX = (dstImage[0].cols - previewWidth) / 2+50;
    previewY = (dstImage[0].rows - previewHeight) / 2+50;
    isDragging = false;
    // 初始化存储的裁剪预览边框大小和位置
    storedwidth = previewWidth;
    storedheight = previewHeight;
    storedx = previewX;
    storedy = previewY;

    QImage image1((const unsigned char*)(finalImage[0].data), finalImage[0].cols, finalImage[0].rows, QImage::Format_Grayscale8);
    QPixmap pixmap1 = QPixmap::fromImage(image1).scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label->setPixmap(pixmap1);
    flag_biankaung=1;

    // 绘制裁剪预览边框
    drawPreviewBorder();

}

caijian::~caijian()
{
    delete ui;
}
int flag_biankuang=0;

void caijian::drawPreviewBorder()
{
    // 绘制裁剪预览边框
    cv::Mat tempImage = finalImage[flag_biankuang].clone();
    cv::Rect rect(previewX, previewY, previewWidth, previewHeight);
    cv::rectangle(tempImage, rect, cv::Scalar(255), 2); // 用灰度值为255绘制边框
    QImage img(tempImage.data, tempImage.cols, tempImage.rows, static_cast<int>(tempImage.step), QImage::Format_Grayscale8);
    ui->label->setPixmap(QPixmap::fromImage(img));
}

void caijian::mousePressEvent(QMouseEvent *event)
{
    // 处理鼠标按下事件，记录鼠标按下的位置
    startX = event->x();
    startY = event->y();


    // 判断鼠标是否在裁剪预览边框内，如果是则允许拖动边框
    if (startX >= previewX && startX <= previewX + previewWidth &&
        startY >= previewY && startY <= previewY + previewHeight)
    {
        isDragging = true;
        // 记录边框的起始位置
        dragStartX = startX;
        dragStartY = startY;
        dragStartWidth = previewWidth;
        dragStartHeight = previewHeight;
    }
}

void caijian::mouseMoveEvent(QMouseEvent *event)
{
    // 处理鼠标移动事件，如果正在拖动边框，则根据鼠标移动距离修改边框大小
    if (isDragging)
    {
        int deltaX = event->x() - startX;
        int deltaY = event->y() - startY;

        previewWidth += deltaX;
        previewHeight += deltaY;

        startX = event->x();
        startY = event->y();
        // 根据鼠标移动的距离更新边框的大小和位置
        // 重新绘制裁剪预览边框
        drawPreviewBorder();
    }
}

void caijian::mouseReleaseEvent(QMouseEvent *event)
{
    // 处理鼠标释放事件，结束拖动
    Q_UNUSED(event);
    isDragging = false;
    // 存储调整后的边框大小和位置
    storedwidth = previewWidth;
    storedheight = previewHeight;
    storedx = previewX;
    storedy = previewY;
}

//上一张
void caijian::on_pushButton_clicked()
{
    if(flag_biankuang==0){
        QMessageBox::warning(this, tr("提示！"), tr("当前已经是第一张图像了！"));

        if (this->parent()) {   // 检查父窗口是否存在
            this->close();      // 关闭当前窗口
            QCoreApplication::processEvents(); //处理所有未完成的事件
            this->parentWidget()->show();      // 显示父窗口
        }
        return;
    }
    else{
        flag_biankuang--;
        QImage image1((const unsigned char*)(finalImage[flag_biankuang].data), finalImage[flag_biankuang].cols, finalImage[flag_biankuang].rows, QImage::Format_Grayscale8);
        QPixmap pixmap1 = QPixmap::fromImage(image1).scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->label->setPixmap(pixmap1);
        // 绘制裁剪预览边框
        drawPreviewBorder();
    }
}

//下一张
void caijian::on_pushButton_2_clicked()
{
    if(flag_biankuang==flag_dst){
        QMessageBox::warning(this, tr("提示！"), tr("当前已经是最后一张图像了！"));

        if (this->parent()) {   // 检查父窗口是否存在
            this->close();      // 关闭当前窗口
            QCoreApplication::processEvents(); //处理所有未完成的事件
            this->parentWidget()->show();      // 显示父窗口
        }
        return;
    }
    else{
        flag_biankuang++;
        QImage image1((const unsigned char*)(finalImage[flag_biankuang].data), finalImage[flag_biankuang].cols, finalImage[flag_biankuang].rows, QImage::Format_Grayscale8);
        QPixmap pixmap1 = QPixmap::fromImage(image1).scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->label->setPixmap(pixmap1);
        // 绘制裁剪预览边框
        drawPreviewBorder();
    }
}

//上移
void caijian::on_pushButton_3_clicked()
{
    //storedx += 10 ;
    previewY -= 10;
    storedy = previewY;
    QImage image1((const unsigned char*)(finalImage[flag_biankuang].data), finalImage[flag_biankuang].cols, finalImage[flag_biankuang].rows, QImage::Format_Grayscale8);
    QPixmap pixmap1 = QPixmap::fromImage(image1).scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label->setPixmap(pixmap1);
    // 绘制裁剪预览边框
    drawPreviewBorder();


}

//下移
void caijian::on_pushButton_4_clicked()
{
    previewY += 10 ;
    storedy = previewY;
    QImage image1((const unsigned char*)(finalImage[flag_biankuang].data), finalImage[flag_biankuang].cols, finalImage[flag_biankuang].rows, QImage::Format_Grayscale8);
    QPixmap pixmap1 = QPixmap::fromImage(image1).scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label->setPixmap(pixmap1);
    // 绘制裁剪预览边框
    drawPreviewBorder();

}

//左移
void caijian::on_pushButton_5_clicked()
{
    previewX -= 10 ;
    storedx = previewX;
    QImage image1((const unsigned char*)(finalImage[flag_biankuang].data), finalImage[flag_biankuang].cols, finalImage[flag_biankuang].rows, QImage::Format_Grayscale8);
    QPixmap pixmap1 = QPixmap::fromImage(image1).scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label->setPixmap(pixmap1);
    // 绘制裁剪预览边框
    drawPreviewBorder();

}

//右移
void caijian::on_pushButton_6_clicked()
{
    previewX += 10 ;
    storedx = previewX;
    QImage image1((const unsigned char*)(finalImage[flag_biankuang].data), finalImage[flag_biankuang].cols, finalImage[flag_biankuang].rows, QImage::Format_Grayscale8);
    QPixmap pixmap1 = QPixmap::fromImage(image1).scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label->setPixmap(pixmap1);
    // 绘制裁剪预览边框
    drawPreviewBorder();

}

