#include "canshu.h"
#include "ui_canshu.h"

int a[6] ={1,2,5,10,20,50};
int b[6] ={0};

Canshu::Canshu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Canshu)
{
    ui->setupUi(this);
}

Canshu::~Canshu()
{
    delete ui;
}

void Canshu::on_pushButton_clicked()
{
    //修改参数
    QString str_a0 = ui->lineEdit_12->text();
    a[0] = str_a0.toInt();

    QString str_a1 = ui->lineEdit_10->text();
    a[1] = str_a1.toInt();

    QString str_a2 = ui->lineEdit_8->text();
    a[2] = str_a2.toInt();

    QString str_a3 = ui->lineEdit_6->text();
    a[3] = str_a3.toInt();

    QString str_a4 = ui->lineEdit_4->text();
    a[4] = str_a4.toInt();

    QString str_a5 = ui->lineEdit_3->text();
    a[5] = str_a5.toInt();

    QString str_b0 = ui->lineEdit_13->text();
    b[0] = str_b0.toInt();

    QString str_b1 = ui->lineEdit_11->text();
    b[1] = str_b1.toInt();

    QString str_b2 = ui->lineEdit_9->text();
    b[2] = str_b2.toInt();

    QString str_b3 = ui->lineEdit_7->text();
    b[3] = str_b3.toInt();

    QString str_b4 = ui->lineEdit_5->text();
    b[4] = str_b4.toInt();

    QString str_b5 = ui->lineEdit_2->text();
    b[5] = str_b5.toInt();


    close();   // 关闭当前页面
}

