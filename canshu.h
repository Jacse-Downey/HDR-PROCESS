#ifndef CANSHU_H
#define CANSHU_H

#include <QWidget>



namespace Ui {
class Canshu;
}



class Canshu : public QWidget
{
    Q_OBJECT

public:
    explicit Canshu(QWidget *parent = nullptr);
    ~Canshu();

private slots:
    void on_pushButton_clicked();

private:
    Ui::Canshu *ui;
};

#endif // CANSHU_H
