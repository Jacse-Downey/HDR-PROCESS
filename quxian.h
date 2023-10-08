#ifndef QUXIAN_H
#define QUXIAN_H
#include <Eigen/Dense>
#include <QWidget>
#include <QMainWindow>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QMouseEvent>
#include <QPoint>
#include <QtCharts/QScatterSeries>
#include <QTimer>
#include <QtCharts/QValueAxis>
#include <QtCharts/QSplineSeries>
#include <unsupported/Eigen/Splines>
#include <QCategoryAxis> // 包含QCategoryAxis头文件



namespace QtCharts {
    class QChartView;
    class QLineSeries;
}

namespace Ui {
class quxian;
}


class quxian : public QWidget
{
    Q_OBJECT

public:
    explicit quxian(QWidget *parent = nullptr);
    ~quxian();

private slots:

    void on_pushButton_3_clicked();

    void on_pushButton_2_clicked();

    //void mousePressEvent(QMouseEvent *event) override;

    void on_pushButton_clicked();

    void on_pushButton_4_clicked();

signals:
    void updateImageRequested(); // 更新图像请求信号（不带参数）



private:
    Ui::quxian *ui;
    bool isDragging; // 是否正在拖动边框
    int startX_p, startY_p; // 拖动开始时的鼠标位置
    QPoint lastMousePos;
    QList<QPoint> points;  // 被拖拽的点的列表
    QLineSeries* series;    // 指向线系列的指针
    QSplineSeries *splineSeries;
    QCategoryAxis *axisX;
    QScatterSeries *markerSeries;
    //QValueAxis *axisX;
    QValueAxis *axisY;
    QChartView* existingChartView;
    QChart* existingChart;
    //QList<QPointF> markedPoints; // 存储标记点的集合
    int markedPointsCount = 0;
    // 假设在类的头文件中已经定义了一个 QScatterSeries 成员变量
    //QScatterSeries *markerSeries;
    QVector<QPointF> mark; // 全局变量用于存储标记点
    int dragStartX_p, dragStartY_p;
    int draggingIndex;
    // 需要在类的成员变量中添加一个用于存储当前标记的点的成员变量
    Eigen::VectorXd coefficients;
};

#endif // QUXIAN_H
