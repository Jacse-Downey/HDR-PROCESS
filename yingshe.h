#ifndef YINGSHE_H
#define YINGSHE_H

#include <QWidget>

namespace Ui {
class yingshe;
}

class yingshe : public QWidget
{
    Q_OBJECT

public:
    explicit yingshe(QWidget *parent = nullptr);
    ~yingshe();

private slots:
    void on_pushButton_clicked();

private:
    Ui::yingshe *ui;
};

#endif // YINGSHE_H
