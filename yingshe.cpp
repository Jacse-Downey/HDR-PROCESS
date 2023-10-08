#include "yingshe.h"
#include "ui_yingshe.h"


float gamma1=0.5;
float gamma2=0.5;
int threshold1=30000;//阈值
yingshe::yingshe(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::yingshe)
{
    ui->setupUi(this);
}

yingshe::~yingshe()
{
    delete ui;
}

void yingshe::on_pushButton_clicked()
{
    //修改参数
    QString str_gamma1 = ui->lineEdit->text();
    gamma1 = str_gamma1.toFloat();

    //修改参数
    QString str_gamma2 = ui->lineEdit_2->text();
    gamma2 = str_gamma2.toFloat();

    //修改参数
    QString str_threshold = ui->lineEdit_3->text();
    threshold1 = str_threshold.toInt();



    close();   // 关闭当前页面
}

