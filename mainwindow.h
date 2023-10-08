#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMainWindow>
#include <QApplication>
#include <qdebug.h>
#include <qlabel.h>
#include <canshu.h>
#include<ui_canshu.h>
#include <vector>

extern int storedx,storedy,storedwidth,storedheight;
extern int a[6];
extern int b[6];
extern int flag_change;
extern float gamma1;
extern float gamma2;
extern int threshold1;//阈值
extern QVector<double> xValues;
extern QVector<double> yValues;
extern double suofangxishu[600];

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_action_input_triggered();

    void on_action_output_triggered();

    void updateLabelImage();

    void on_pushButton_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_8_clicked();

    void on_pushButton_9_clicked();

    void on_pushButton_10_clicked();

    void on_pushButton_11_clicked();

    void on_pushButton_13_clicked();

    void on_pushButton_14_clicked();

    void on_pushButton_12_clicked();

    void on_pushButton_17_clicked();

    void on_pushButton_15_clicked();

    void on_pushButton_16_clicked();

    void on_pushButton_18_clicked();

    void on_action_yulan_triggered();

    void on_action_inputdata_triggered();

    void on_action_new_map_triggered();

    //void on_action_xianshicanshu_triggered();

private:
    Ui::MainWindow *ui;
    QImage img;                  //将用到的定义全部声明
    QString img_name;
};
#endif // MAINWINDOW_H
