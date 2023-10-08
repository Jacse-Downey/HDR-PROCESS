#include "quxian.h"
#include "ui_quxian.h"
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
#include <gsl/gsl_spline.h>
#include <gsl/gsl_interp.h>
#include <unsupported/Eigen/Splines>
#include "mapping_show.h"
#include "process.h"

Eigen::VectorXd coefficients;
double a_map;
double b_map;
double c_map;
double click_x=0;
double click_y=0;


//多项式拟合函数
Eigen::VectorXd cubicFitWithoutConstant(const std::vector<double>& x, const std::vector<double>& y) {
    int n = x.size();
    Eigen::MatrixXd A(n, 3); // 使用三个系数来表示三次多项式（去除常数项）
    Eigen::VectorXd b(n);
    Eigen::VectorXd result(3);

    for (int i = 0; i < n; ++i) {
        double xi = x[i];
        A(i, 0) = xi * xi * xi;
        A(i, 1) = xi * xi;
        A(i, 2) = xi;
        b(i) = y[i];
    }

    result = A.fullPivLu().solve(b);
    return result;
}


quxian::quxian(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::quxian)
{
    ui->setupUi(this);
    this->setMouseTracking(true); // 启用鼠标追踪

    if(flag_quxian==0){
               // 获取已经存在的graphicsView中的QChart
               existingChartView = ui->graphicsView;
               existingChart = existingChartView->chart();
               existingChartView->setRenderHint(QPainter::Antialiasing);

               // 创建线系列
               series = new QLineSeries;
               for (int i = 0; i <= 255; ++i) {
                   series->append(i, i);
                   points.append(QPoint(i, i)); // 使用 QPoint 初始化点
               }

               // 添加拟合曲线到已经存在的QChart中
               existingChart->addSeries(series);
               existingChart->legend()->hide();

               // 创建坐标轴
               /*axisX = new QValueAxis;
               axisX->setRange(0, 255);

               existingChart->addAxis(axisX, Qt::AlignBottom);
               series->attachAxis(axisX);*/
               //添加部分
               //QCategoryAxis *axisX=new QCategoryAxis;
               axisX=new QCategoryAxis;
               axisX->setMin(0.0);
               axisX->setMax(255.0);
               axisX->setStartValue(0.0);
               //append要按照大小顺序依次添加
               axisX->append("0.0",0.0);
               axisX->append("818900.0",63.75);
               axisX->append("1637800.0",127.5);
               axisX->append("2456700.0",191.25);
               axisX->append("3275600",255.0);
              //下边这一句代码的作用是让刻度线和刻度的label对齐
              axisX->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
              existingChart->setAxisX(axisX,series);
              series->attachAxis(axisX);

                //原始部分
               axisY = new QValueAxis;
               // 创建坐标轴
               axisY->setRange(0, 255);
               existingChart->addAxis(axisY, Qt::AlignLeft);
               series->attachAxis(axisY);

               // 重新设置chartView的chart
               existingChartView->setChart(existingChart);

               // 设置窗口标题
               setWindowTitle("像素映射曲线图");

        // 初始化 lastMousePos
        lastMousePos = QPoint(0, 0);
    }
    if(flag_quxian==1){
        // 选择CSV文件
        QString fileName = QFileDialog::getOpenFileName(this, "选择CSV文件", "", "CSV 文件 (*.csv)");
        if (!fileName.isEmpty()) {
            // 读取CSV文件
            QFile file(fileName);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                int i=0;
                while (!in.atEnd()) {
                    QString line = in.readLine();
                    QStringList parts = line.split(',');
                    if (parts.size() >= 2) {
                        double xValue = parts[0].toDouble();
                        double yValue = parts[1].toDouble();
                        csvfile1[i][0]=(xValue/maxvalue)*255.0;
                        csvfile1[i][1]=yValue;
                        i++;
                    }
                }
                file.close();
            }
        }



        // 获取已经存在的graphicsView中的QChart
        existingChartView = ui->graphicsView;
        existingChart = existingChartView->chart();
        existingChartView->setRenderHint(QPainter::Antialiasing);


        // 创建线系列
        splineSeries = new QSplineSeries;
        // 生成拟合曲线上的点
        for (int xi = 0; xi < 10001; ++xi) {
            splineSeries->append(csvfile1[xi][0], csvfile1[xi][1]);
        }


        // 添加拟合曲线到已经存在的QChart中
        existingChart->addSeries(splineSeries);

        // 创建坐标轴
        //axisX = new QValueAxis;
        /*axisX=new QCategoryAxis;
        axisX->setRange(0, 255);
        existingChart->addAxis(axisX, Qt::AlignBottom);*/

        axisX=new QCategoryAxis;
        axisX->setMin(0.0);
        axisX->setMax(255.0);
        axisX->setStartValue(0.0);
        //append要按照大小顺序依次添加
        axisX->append("0.0",0.0);
        axisX->append("818900.0",63.75);
        axisX->append("1637800.0",127.5);
        axisX->append("2456700.0",191.25);
        axisX->append("3275600",255.0);
       //下边这一句代码的作用是让刻度线和刻度的label对齐
       axisX->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
       existingChart->setAxisX(axisX,splineSeries);
       //series->attachAxis(axisX);
        splineSeries->attachAxis(axisX);

        axisY = new QValueAxis;
        axisY->setRange(0, 255);
        existingChart->addAxis(axisY, Qt::AlignLeft);
        splineSeries->attachAxis(axisY);

        // 重新设置chartView的chart
        existingChartView->setChart(existingChart);

        // 设置窗口标题
        setWindowTitle("像素映射曲线图");
    }
}

//鼠标点击
/*void quxian::mousePressEvent(QMouseEvent *event)
{
    // 获取鼠标点击的位置
    QPoint currentMousePos = event->pos();

    // 将鼠标点击的点从窗口坐标转换为数据坐标
    QPointF clickedPoint = existingChartView->chart()->mapToValue(currentMousePos);
    // 在点击的点处添加标记（例如，红色圆圈）
    QScatterSeries *markerSeries = new QScatterSeries;
    markerSeries->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    markerSeries->setMarkerSize(10);
    markerSeries->setColor(Qt::red);
    markerSeries->append(clickedPoint);
    if (series) {
            // 清空原始曲线
            series->clear();
            // 进行三次多项式拟合
            std::vector<double> xValues = {0, clickedPoint.x(), 255};
            std::vector<double> yValues = {0, clickedPoint.y(), 255};
            Eigen::VectorXd coefficients = cubicFitWithoutConstant(xValues, yValues);
            a_map=(double)(clickedPoint.y()/clickedPoint.x());
            b_map=(double)((255-clickedPoint.y())/(255-clickedPoint.x()));
            c_map=(double)(255-b_map*255);


            a_map=(double)coefficients(0);
            b_map=(double)coefficients(1);
            c_map=(double)coefficients(2);
            // 生成拟合曲线上的点
            for (int x = 0; x <= (int)clickedPoint.x(); ++x) {
                double y = a_map * x ;
                series->append(x, y);
            }
            for (int x = (int)clickedPoint.x()+1; x <= 255; ++x) {
                double y = b_map * x +c_map;
                series->append(x, y);
            }
            existingChart->removeSeries(series); // 删除已存在的拟合曲线（如果有的话）
            existingChart->addSeries(series); // 添加拟合曲线到图表

    }

}*/



quxian::~quxian()
{
    delete ui;
}

//保存曲线数据
void quxian::on_pushButton_3_clicked()
{

    // 打开文件对话框以选择保存路径和文件名
    QString filePath = QFileDialog::getSaveFileName(nullptr, "保存CSV文件", QDir::homePath(), "CSV文件 (*.csv)");

    // 计算并保存数据
    std::ofstream csvFile(filePath.toStdString()); // 使用选择的文件路径
    for (int j = 0; j < 10001; ++j) {
            csvFile << csvfile1[j][0] << "," << csvfile1[j][1] << std::endl;
    }
    csvFile.close();
}

//显示预览效果按钮，包括将调整后的曲线映射数据保存到全局变量里、将窗口关闭
void quxian::on_pushButton_2_clicked()
{
    mapping_show *configWindow = new mapping_show;
    configWindow->setWindowTitle("曲线映射预览");
    configWindow->show();
}

//应用当前曲线
void quxian::on_pushButton_clicked()
{
    flag_change=1;
    if(flag_cut==1){
        // 2. 遍历图像中的每个像素
        for(int loop=0;loop<=flag_dst;loop++){
            cv::Mat temp=finalImage_map[loop];
        for (int x = 0; x < cutImage_map[loop].rows; x++) {
            for (int y = 0; y < cutImage_map[loop].cols; y++) {
                // 获取当前像素的灰度值作为 x 值
                double xValue = static_cast<double>(cutImage_map[loop].at<float>(x,y));
                int index = (xValue/maxvalue)*10000;
                // 3. 使用拟合曲线的公式计算 y 值作为映射后的像素值
                double yValue = csvfile1[index][1];
                // 4. 创建新的图像并设置像素值
                cutImage[loop].at<uchar>(x,y) = (uchar)yValue;
            }
        }
        }
        emit updateImageRequested();

    }
    else{
        // 2. 遍历图像中的每个像素
        for(int loop=0;loop<=flag_dst;loop++){
        for (int x = 0; x < finalImage_map[loop].rows; x++) {
            for (int y = 0; y < finalImage_map[loop].cols; y++) {
                // 获取当前像素的灰度值作为 x 值
                double xValue = static_cast<double>(finalImage_map[loop].at<float>(x,y));
                int index = (xValue/maxvalue)*10000;
                // 3. 使用拟合曲线的公式计算 y 值作为映射后的像素值
                double yValue = csvfile1[index][1];
                // 4. 创建新的图像并设置像素值
                finalImage[loop].at<uchar>(x,y) = (uchar)yValue;
            }
        }
        }
        emit updateImageRequested();

    }
    // 关闭当前的quxian窗口
    this->close();
}

//通过输入的点坐标绘制曲线
void quxian::on_pushButton_4_clicked()
{
    if(existingChart->series().contains(series)){
    // 删除线性系列
    existingChart->removeSeries(series);
    delete series;
    }

    if(existingChart->series().contains(splineSeries)){
        // 清除数据点
        splineSeries->clear();
    }
    if(existingChart->series().contains(markerSeries)){
        // 清除数据点
        markerSeries->clear();
    }

    // 获取已经存在的graphicsView中的QChart
    existingChartView = ui->graphicsView;
    existingChart = existingChartView->chart();
    existingChartView->setRenderHint(QPainter::Antialiasing);

    //
    QString str_a0 = ui->lineEdit->text();
    double x1 = str_a0.toDouble();

    QString str_a1 = ui->lineEdit_2->text();
    double y1 = str_a1.toDouble();

    QString str_a2 = ui->lineEdit_3->text();
    double x2 = str_a2.toDouble();

    QString str_a3 = ui->lineEdit_4->text();
    double y2 = str_a3.toDouble();

    QString str_a4 = ui->lineEdit_5->text();
    double x3 = str_a4.toDouble();

    QString str_a5 = ui->lineEdit_6->text();
    double y3 = str_a5.toDouble();
    splineSeries = new QSplineSeries;

    if(x1>0.0&&x1<x2&&x2<x3&&x3<maxvalue&&y1>0.0&&y1<y2&&y2<y3&&y3<255.0){

    // 定义控制点
       double x[] = {0.0, (x1/3275600)*255, (x2/3275600)*255, (x3/3275600)*255, 255.0};
       double y[] = {0.0, y1, y2, y3, 255.0};
       int n = sizeof(x) / sizeof(double); // 控制点数量


       // 创建GSL样条插值对象
       gsl_interp_accel *acc = gsl_interp_accel_alloc();
       gsl_spline *spline = gsl_spline_alloc(gsl_interp_cspline, n);
       // 初始化样条插值
       gsl_spline_init(spline, x, y, n);
       // 计算样条曲线上的点并存储它们
       double xStep = 0.0255; // 参数 x 的步长 (可以根据需要调整)
       int i=0;
       for (double xi = 0.0; xi <= 255; xi += xStep) {
            csvfile1[i][0]=(xi/255)*maxvalue;
           double yi = gsl_spline_eval(spline, xi, acc);
            csvfile1[i][1]=yi;
           splineSeries->append(xi, yi);
           i++;
       }
       // 释放资源
       gsl_spline_free(spline);
       gsl_interp_accel_free(acc);
    // 创建坐标点
    QVector<QPointF> points;
    points << QPointF(0, 0) << QPointF(x[1], y1) << QPointF(x[2], y2) << QPointF(x[3], y3) << QPointF(255, 255);

    // 添加插值系列到图表
    existingChart->addSeries(splineSeries);

    // 将插值系列附加到轴
    splineSeries->attachAxis(axisX);
    splineSeries->attachAxis(axisY);

    // 在点击的点处添加标记（例如，红色圆圈）
    markerSeries = new QScatterSeries;
    markerSeries->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    markerSeries->setMarkerSize(10);
    markerSeries->setColor(Qt::red);
    for(int i=1;i<4;i++){
    markerSeries->append(points[i]);
    }
    existingChart->addSeries(markerSeries);
    markerSeries->attachAxis(existingChart->axisX());
    markerSeries->attachAxis(existingChart->axisY());

    // 更新图表视图
    existingChartView->setChart(existingChart);
    }
    else {
        QMessageBox::warning(this, tr("提示！"), tr("请确保输入点的横纵坐标均按照单调增的顺序！请检查输入坐标点横纵坐标值的范围！"));

        if (this->parent()) {   // 检查父窗口是否存在
            this->close();      // 关闭当前窗口
            QCoreApplication::processEvents(); //处理所有未完成的事件
            this->parentWidget()->show();      // 显示父窗口
        }
        return;

    }

}

