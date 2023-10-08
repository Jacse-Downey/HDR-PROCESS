#ifndef CAIJIAN_H
#define CAIJIAN_H

#include <QWidget>
#include <opencv2/core/core.hpp>


namespace Ui {
class caijian;
}

class caijian : public QWidget
{
    Q_OBJECT

public:
    explicit caijian(QWidget *parent = nullptr);
    ~caijian();

private slots:
    void on_pushButton_clicked();//上一张图像

    void on_pushButton_2_clicked();//下一张图像

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

private:
    Ui::caijian *ui;
    bool isDragging; // 是否正在拖动边框
    int startX, startY; // 拖动开始时的鼠标位置
    int previewX, previewY; // 裁剪预览边框的左上角坐标
    int previewWidth, previewHeight; // 裁剪预览边框的宽度和高度
    // 存储边框信息
    int storedPreviewX, storedPreviewY;
    int storedPreviewWidth, storedPreviewHeight;
    // 添加用于拖动的新成员变量
    int dragStartX, dragStartY;
    int dragStartWidth, dragStartHeight;
    void drawPreviewBorder(); // 绘制裁剪预览边框
};

#endif // CAIJIAN_H
