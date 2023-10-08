#include "mainwindow.h"
#include "QVBoxLayout"
#include "QLabel"
#include "ui_mainwindow.h"
#include <opencv2/opencv.hpp>
#include <string.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <QDir>
#include <QtDebug>
#include <QApplication>
#include <QFileDialog>
#include <imageprocess.h>
#include <opencv2/features2d.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/features2d.hpp>
#include <iostream>
#include <QString>
#include <string>
#include <fstream>
#include <stdint.h>
#include <QRegularExpression>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <math.h>
#include <QPalette>
#include <QMessageBox>
#include <QPainter>
#include "caijian.h"
#include "quxian.h"
#include <QVector>
#include <QPixmap>
#include <QPushButton>
#include <QApplication>
#include "QMutex"
#include <QVariant>
#include <Eigen/Dense>
#include <cmath>
#include "yingshe.h"



using namespace cv;
using namespace std;
int flag_src=0;
int flag_dst=0;
int flag_cut=0;
int flag_quxian=2;
int flag_change=0;
int currentPhotoIndex=0;
int currentPhotoIndex_dst=0;
double suofangxishu[600]={0};
std::string tempfilenames[600];
std::string tempfilenames_dst[600];
std::string tempfilenames_src[600][6];
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

//连续的0的个数
int count_continuous_zeros(const std::vector<int>& arr, int index)
{
    int count_zeros = 1;
    while (index > 0 && arr[index - 1] == 0)
    {
        count_zeros++;
        index--;
    }
    return count_zeros;
}

//双重gamma校正算法
cv::Mat highlight_compression(cv::Mat image, float gamma1 , float gamma2 ,int threshold) {
    // 将图像转换为浮点数
    cv::Mat image_float;
    image.convertTo(image_float, CV_32F);

    // 计算伽马校正后的图像
    cv::Mat compressed_image = image_float.clone();  // 克隆图像以存储结果

    // 对图像进行遍历以进行伽马校正和高亮度区域的处理
    for (int x = 0; x < image.rows; x++) {
        for (int y = 0; y < image.cols; y++) {
            float pixel_value = image_float.at<float>(x, y);
            float compressed_value = std::pow(pixel_value / 3275600.0, gamma1) * 3275600.0;
            // 限制值在0到3275600之间
            compressed_image.at<float>(x, y) = std::max(0.0f, std::min(3275600.0f, compressed_value));
        }
    }

    // 对图像进行遍历以进行伽马校正和高亮度区域的处理
    for (int x = 0; x < image.rows; x++) {
        for (int y = 0; y < image.cols; y++) {
            float pixel_value = image_float.at<float>(x, y);
            float compressed_value;
            if (pixel_value > threshold) {
                compressed_value = std::pow(pixel_value / 3275600.0, gamma2) * 255.0;
            }
            else{
                compressed_value = (pixel_value / 3275600.0)* 255.0;
            }
            // 限制值在0到255之间
            compressed_image.at<float>(x, y) = std::max(0.0f, std::min(255.0f, compressed_value));
        }
    }

    // 将图像值映射到0-255范围
    cv::Mat mapped_image = compressed_image.clone();
    mapped_image.convertTo(mapped_image, CV_8U);

    return mapped_image;
}

Mat contrastStretch(cv::Mat Image)
{
    cv::Mat resultImage = Image.clone();
    int nRows = resultImage.rows;
    int nCols = resultImage.cols;
    // 图像连续性判断
    if (resultImage.isContinuous())
    {
        nCols = nCols * nRows;
        nRows = 1;
    }
    // 图像指针操作
    float* pDataMat;
    float pixMax = 0, pixMin =0;
    // 计算图像的最大最小值
    for (int j = 0; j < nRows; j++)
    {
        pDataMat = resultImage.ptr<float>(j);
        for (int i = 0; i < nCols; i++)
        {
            if (pDataMat[i] > pixMax)
                pixMax = pDataMat[i];
            if (pDataMat[i] < pixMin)
                pixMin = pDataMat[i];
        }
    }
    // 对比度拉伸映射
    for (int j = 0; j < nRows; j++)
    {
        pDataMat = resultImage.ptr<float>(j);
        for (int i = 0; i < nCols; i++)
        {
            pDataMat[i] = (pDataMat[i] - pixMin) * 255 / (pixMax - pixMin);
        }
    }
    return resultImage;
}

// 曝光融合函数
cv::Mat exposureFusion(const std::vector<cv::Mat>& images, const std::vector<float>& exposures, float gamma)
{
    // 调整图像曝光，使其具有相同的平均曝光
    std::vector<cv::Mat> exposuresAdjusted;
    for (const auto& image : images)
    {
        cv::Mat adjustedImg;
        cv::Mat floatingImg;
        image.convertTo(floatingImg, CV_32FC1);
        cv::pow(floatingImg, gamma, adjustedImg);
        exposuresAdjusted.push_back(adjustedImg);
    }
    // 计算权重图像
    std::vector<cv::Mat> weights;
    for (const auto& exposure : exposuresAdjusted)
    {
        cv::Mat weight;
        cv::normalize(exposure, weight, 0, 1, cv::NORM_MINMAX);
        weights.push_back(weight);
    }
    // 执行曝光融合
    cv::Mat fusion = cv::Mat::zeros(images[0].size(), CV_32FC1);
    for (size_t i = 0; i < images.size(); i++)
    {
        cv::Mat weightedImg;
        cv::multiply(exposuresAdjusted[i], weights[i], weightedImg);
        cv::add(fusion, weightedImg, fusion);
    }
    cv::normalize(fusion, fusion, 0, 255, cv::NORM_MINMAX);
    fusion.convertTo(fusion, CV_8UC1);
    return fusion;
}


//导入文件
void MainWindow::on_action_input_triggered()
{
    //读取图片
    // 打开文件选择对话框以获取文件夹路径

    QString folderPath = QFileDialog::getExistingDirectory(this, tr("Select Folder"));
    if (folderPath.isEmpty()) {
        return; // 用户取消选择，直接退出函数
    }
    QDir dir(folderPath);
    QStringList filters;
    filters << "*.raw";
    QStringList files = dir.entryList(filters);
    // 排序文件名
    std::sort(files.begin(), files.end(), [](const QString& a, const QString& b){
        QRegularExpression re("\\d+"); // 匹配数字
        auto matchA = re.globalMatch(a);
        auto matchB = re.globalMatch(b);

        while (matchA.hasNext()) {
            int num1 = matchA.next().captured().toInt(); // 获取数字值
            if (matchB.hasNext()) {
                int num2 = matchB.next().captured().toInt();
                if (num1 != num2) {
                    return num1 < num2; // 数字不相同，按照数字大小排序
                }
            } else {
                return false; // b 中没有数字，a 排在 b 前面
            }
        }

        return a < b; // 比较剩余字符
    });
    std::vector<cv::Mat> matList;
    matList.reserve(files.size()); // 预先分配足够的内存空间
    for (int i = 0; i < files.size(); ++i) {
        QString filePath = folderPath + "/" + files[i];
        tempfilenames[i]=files[i].toStdString();
        tempfilenames_src[i/6][i%6]=files[i].toStdString();

        ifstream in(filePath.toStdString(), ios::binary);
        if (!in.is_open()) {
            cerr << "Failed to open file: " << filePath.toStdString() << endl;
        }
        in.seekg(36);
        vector<uint16_t> data(1360 * 1024);
        in.read(reinterpret_cast<char*>(data.data()), data.size() * sizeof(uint16_t));
        in.close();
        Mat img(1024, 1360, CV_16UC1, data.data());
        srcImage[i/6][i%6]=img.clone();

        // 指定要保存的文件路径和文件名
        //std::string outputPath = "C:/Users/65366/Desktop/data/group5"+ std::to_string(i) +".png";

        // 将图像保存到指定位置
        //cv::imwrite(outputPath,img);
        flag_src++;
    }
    if (flag_src == 0) {
        return; // 没有有效的图片，直接退出函数
    }
    flag_dst = (flag_src-1)/6;

    // 将 std::string 转换为 QString
    QString qstr5 = "1."+QString::fromStdString(tempfilenames_src[0][0]);

    // 将 QString 设置为 QLabel 的文本
    ui->label_5->setText(qstr5);

    QImage image(srcImage[0][0].data, srcImage[0][0].cols, srcImage[0][0].rows, QImage::Format_Grayscale16);
    QPixmap pixmap = QPixmap::fromImage(image).scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label->setPixmap(pixmap);
}

//导出结果图
void MainWindow::on_action_output_triggered()
{
    if (finalImage[0].empty()) {
            QMessageBox::warning(this, tr("无已合成图片"), tr("请导入一个文件夹并点击HDR图片合成重试！"));
            if (this->parent()) {   // 检查父窗口是否存在
                this->close();      // 关闭当前窗口
                QCoreApplication::processEvents(); //处理所有未完成的事件
                this->parentWidget()->show();      // 显示父窗口
            }
            return;
    }
    if(flag_cut==1){
        QString filename_floder=QFileDialog::getExistingDirectory(nullptr,"选择文件夹","");
        if (!filename_floder.isEmpty()){
            for (int i = 0; i < (flag_dst+1); i++)
                {
                    // 构造文件名
                    // 使用格式化将 double 转换为带两位小数的字符串
                    std::ostringstream stream;
                    stream << std::fixed << std::setprecision(2) << suofangxishu[i];
                    std::string suffix = stream.str();
                    QString suffix_result = QString::fromStdString(suffix);
                    QString qstr = QString::fromStdString(tempfilenames[i*6].substr(0, tempfilenames[i*6].find("-1db.raw")));
                    QString fileName = filename_floder + "/"+ qstr +"_" + suffix_result + ".bmp";

                    // 保存图像
                    cv::imwrite(fileName.toStdString(), cutImage[i]);
                }
            }


    }
    else{
    //选择文件
    QString filename_floder=QFileDialog::getExistingDirectory(nullptr,"选择文件夹","");
    if (!filename_floder.isEmpty()){
        for (int i = 0; i < (flag_dst+1); i++)
            {
                QString qstr = QString::fromStdString(tempfilenames[i*6].substr(0, tempfilenames[i*6].find("-1db.raw")));
                QString fileName = filename_floder + "/"+qstr + ".bmp";

                // 保存图像
                cv::imwrite(fileName.toStdString(), finalImage[i]);
            }
        }
    }
}

//HDR图像合成
void MainWindow::on_pushButton_clicked()
{
    if (srcImage[0][0].empty()) {
            QMessageBox::warning(this, tr("未导入文件"), tr("请导入一个文件夹并重试！"));

            if (this->parent()) {   // 检查父窗口是否存在
                this->close();      // 关闭当前窗口
                QCoreApplication::processEvents(); //处理所有未完成的事件
                this->parentWidget()->show();      // 显示父窗口
            }
            return;
    }
    else{
    for (int loop = 0; loop < (flag_dst+1); loop++) {
        vector<Mat> result(6);
        //int a[6] = { 1,2,5,10,20,50 }, b[6] = { 0 };//存储每个图像的系数
        int flag_t = 0;
        Mat final(1024, 1360, CV_32FC1);
           for (int row = 0; row < srcImage[loop][flag_t].rows; row++) {
               for (int col = 0; col < srcImage[loop][flag_t].cols; col++) {
                   bool found = false;
                   for (int i = 0; i < 6; i++) {
                       ushort value = srcImage[loop][i].at<ushort>(row, col);
                       if (value < 58960) {
                           final.at<float>(row, col) = (float)value * a[flag_t]+b[flag_t];
                           found = true;
                           flag_t = 0;
                           break;
                       }
                       else {
                           flag_t++;
                       }
                   }
                   if (!found) {
                       final.at<float>(row, col) = (float)srcImage[loop][5].at<ushort>(row, col) * a[5]+b[5];
                       flag_t = 0;
                   }

               }
           }
        double maxvalue_ini; // 声明 double 类型的变量来存储最大值
        cv::minMaxLoc(final, NULL, &maxvalue_ini); // 传递正确类型的指针参数
        maxvalue = (int)maxvalue_ini;
        //highlight_compression(final,gamma1,gamma2,threshold1);
        finalImage_map[loop]=final.clone();
        cv::normalize(final, finalImage[loop], 0, 255, cv::NORM_MINMAX, CV_8U);
        tempfilenames_dst[loop]=tempfilenames[loop*6].substr(0, tempfilenames[loop*6].find("-1db.raw"));
        // 曝光时间（以秒为单位）
        std::vector<float> exposures = { 1.0f, 0.5f, 0.2f,0.1f,0.05f,0.02f };
        std::vector<Mat> srcImages(6);
        for (int i = 0; i < 6; i++) {
            srcImages[i] = srcImage[loop][i].clone();
        }
        // 曝光融合
        cv::Mat fusedImage = exposureFusion(srcImages, exposures, 5.5f); // gamma设为2.2进行gamma矫正
        dstImage[loop]=fusedImage.clone();
        showImage[loop]=finalImage[loop].clone();
    }
    // 将 std::string 转换为 QString
    QString qstr6 = "1." + QString::fromStdString(tempfilenames_dst[0]);

    // 将 QString 设置为 QLabel 的文本
    ui->label_6->setText(qstr6);
    QImage image((const unsigned char*)(finalImage[0].data), finalImage[0].cols, finalImage[0].rows, QImage::Format_Grayscale8);
    QPixmap pixmap = QPixmap::fromImage(image).scaled(ui->label_2->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_2->setPixmap(pixmap);
    qApp->processEvents();
    }
}


//参数调整
void MainWindow::on_pushButton_3_clicked()
{
    Canshu *configWindow = new Canshu;
    configWindow->setWindowTitle("合成参数设置");
    configWindow->show();
}


//分割目标物体
void MainWindow::on_pushButton_2_clicked()
{
    if (finalImage[0].empty()) {
            QMessageBox::warning(this, tr("无已合成图片"), tr("请导入一个文件夹并点击HDR图片合成重试！"));
            if (this->parent()) {   // 检查父窗口是否存在
                this->close();      // 关闭当前窗口
                QCoreApplication::processEvents(); //处理所有未完成的事件
                this->parentWidget()->show();      // 显示父窗口
            }
            return;
    }
    if(flag_biankaung==0){
        QMessageBox::warning(this, tr("边框没有预设"), tr("请先预览并预设裁剪边框大小！"));
        if (this->parent()) {   // 检查父窗口是否存在
            this->close();      // 关闭当前窗口
            QCoreApplication::processEvents(); //处理所有未完成的事件
            this->parentWidget()->show();      // 显示父窗口
        }
        return;
    }
    else{
        //cv::Mat blackBackground = cv::Mat::zeros(512, 512, CV_8UC1);
    for(int loop=0;loop<(flag_dst+1);loop++){
        cv::Mat blackBackground = cv::Mat::zeros(512, 512, CV_8UC1);
        cv::Mat blackBackground1 = cv::Mat::zeros(512, 512, CV_8UC1);
        int newwidth, newheight;
        cv::Rect roi(storedx, storedy, storedwidth, storedheight); // 以左上角点为(200, 200)，宽为300，高为200的矩形作为ROI
        cutImage[loop] = finalImage[loop](roi);
        cutImage_map[loop]= finalImage_map[loop](roi);
        if(storedwidth>storedheight){
            newwidth = 512;
            newheight = static_cast<int>((static_cast<double>(storedheight) / storedwidth) * newwidth);
            suofangxishu[loop]=(static_cast<double>(newwidth) / storedwidth);
        }
        else{
            newheight = 512;
            newwidth = static_cast<int>((static_cast<double>(storedwidth) / storedheight) * newheight);
            suofangxishu[loop]=(static_cast<double>(newheight) / storedheight);
        }
        // 计算在黑色背景中的放置位置
        cv::resize(cutImage[loop], cutImage[loop], Size(newwidth, newheight));
        cv::resize(cutImage_map[loop], cutImage_map[loop], Size(newwidth, newheight));
        int x = (512 - cutImage[loop].cols) / 2;
        int y = (512 - cutImage[loop].rows) / 2;
        // 将分割图像放置在黑色背景中
        cv::Mat roiInBlack = blackBackground(cv::Rect(x, y, cutImage[loop].cols, cutImage[loop].rows));
        cv::Mat roiInBlack1 = blackBackground(cv::Rect(x, y, cutImage_map[loop].cols, cutImage_map[loop].rows));
        cutImage_map[loop].copyTo(roiInBlack1);
        cutImage[loop].copyTo(roiInBlack);
        cutImage_map[loop]=blackBackground1.clone();
        cutImage[loop]=blackBackground.clone();
        showImage_cut[loop] = cutImage[loop].clone();
    }
    flag_cut=1;

    // 将 std::string 转换为 QString
    QString qstr6 = "1." + QString::fromStdString(tempfilenames_dst[0]);

    // 将 QString 设置为 QLabel 的文本
    ui->label_6->setText(qstr6);
    QImage image((const unsigned char*)(cutImage[0].data), cutImage[0].cols, cutImage[0].rows, QImage::Format_Grayscale8);
    QPixmap pixmap = QPixmap::fromImage(image).scaled(ui->label_2->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_2->setPixmap(pixmap);
    qApp->processEvents();
    }
}



//锐化
void MainWindow::on_pushButton_4_clicked()
{
    if (finalImage[0].empty()) {
            QMessageBox::warning(this, tr("无已合成图片"), tr("请导入一个文件夹并点击HDR图片合成重试！"));
            if (this->parent()) {   // 检查父窗口是否存在
                this->close();      // 关闭当前窗口
                QCoreApplication::processEvents(); //处理所有未完成的事件
                this->parentWidget()->show();      // 显示父窗口
            }
            return;
    }
    if(flag_cut==1)
    {
        //图像锐化
        // 定义锐化核
        cv::Mat kernel = (cv::Mat_<float>(3,3) <<
                          0, -1, 0,
                         -1,  5, -1,
                          0, -1, 0);

        // 对图像进行锐化操作
        //cv::Mat sharpened;
        cv::filter2D(cutImage[currentPhotoIndex/6], cutImage[currentPhotoIndex/6], -1, kernel);
        QImage image((const unsigned char*)(cutImage[currentPhotoIndex/6].data), cutImage[currentPhotoIndex/6].cols, cutImage[currentPhotoIndex/6].rows, QImage::Format_Grayscale8);
        QPixmap pixmap = QPixmap::fromImage(image).scaled(ui->label_2->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->label_2->setPixmap(pixmap);
        qApp->processEvents();
    }
    else{
    //图像锐化
    // 定义锐化核
    cv::Mat kernel = (cv::Mat_<float>(3,3) <<
                      0, -1, 0,
                     -1,  5, -1,
                      0, -1, 0);
    // 对图像进行锐化操作
    //cv::Mat sharpened;
    cv::filter2D(finalImage[currentPhotoIndex/6], finalImage[currentPhotoIndex/6], -1, kernel);
    QImage image((const unsigned char*)(finalImage[currentPhotoIndex/6].data), finalImage[currentPhotoIndex/6].cols, finalImage[currentPhotoIndex/6].rows, QImage::Format_Grayscale8);
    QPixmap pixmap = QPixmap::fromImage(image).scaled(ui->label_2->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_2->setPixmap(pixmap);
    qApp->processEvents();
    }
}


//均值滤波
void MainWindow::on_pushButton_5_clicked()
{
    if (finalImage[0].empty()) {
            QMessageBox::warning(this, tr("无已合成图片"), tr("请导入一个文件夹并点击HDR图片合成重试！"));
            if (this->parent()) {   // 检查父窗口是否存在
                this->close();      // 关闭当前窗口
                QCoreApplication::processEvents(); //处理所有未完成的事件
                this->parentWidget()->show();      // 显示父窗口
            }
            return;
    }
    if(flag_cut==1){
        blur(cutImage[currentPhotoIndex/6], cutImage[currentPhotoIndex/6], Size(3, 3));
        QImage image((const unsigned char*)(cutImage[currentPhotoIndex/6].data), cutImage[currentPhotoIndex/6].cols, cutImage[currentPhotoIndex/6].rows, QImage::Format_Grayscale8);
        QPixmap pixmap = QPixmap::fromImage(image).scaled(ui->label_2->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->label_2->setPixmap(pixmap);
        qApp->processEvents();
    }
    else{
    //均值滤波
    blur(finalImage[currentPhotoIndex/6], finalImage[currentPhotoIndex/6], Size(3, 3));
    QImage image((const unsigned char*)(finalImage[currentPhotoIndex/6].data), finalImage[currentPhotoIndex/6].cols, finalImage[currentPhotoIndex/6].rows, QImage::Format_Grayscale8);
    QPixmap pixmap = QPixmap::fromImage(image).scaled(ui->label_2->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_2->setPixmap(pixmap);
    qApp->processEvents();
    }
}



//高斯滤波
void MainWindow::on_pushButton_6_clicked()
{
    if (finalImage[0].empty()) {
            QMessageBox::warning(this, tr("无已合成图片"), tr("请导入一个文件夹并点击HDR图片合成重试！"));
            if (this->parent()) {   // 检查父窗口是否存在
                this->close();      // 关闭当前窗口
                QCoreApplication::processEvents(); //处理所有未完成的事件
                this->parentWidget()->show();      // 显示父窗口
            }
            return;
    }
    if(flag_cut==1){
        GaussianBlur(cutImage[currentPhotoIndex/6], cutImage[currentPhotoIndex/6], Size(5, 5), 100);
        QImage image((const unsigned char*)(cutImage[currentPhotoIndex/6].data), cutImage[currentPhotoIndex/6].cols, cutImage[currentPhotoIndex/6].rows, QImage::Format_Grayscale8);
        QPixmap pixmap = QPixmap::fromImage(image).scaled(ui->label_2->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->label_2->setPixmap(pixmap);
        qApp->processEvents();
    }
    else{
    //高斯滤波
    GaussianBlur(finalImage[currentPhotoIndex/6], finalImage[currentPhotoIndex/6], Size(5, 5), 100);
    QImage image((const unsigned char*)(finalImage[currentPhotoIndex/6].data), finalImage[currentPhotoIndex/6].cols, finalImage[currentPhotoIndex/6].rows, QImage::Format_Grayscale8);
    QPixmap pixmap = QPixmap::fromImage(image).scaled(ui->label_2->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_2->setPixmap(pixmap);
    qApp->processEvents();
    }
}



//中值滤波
void MainWindow::on_pushButton_7_clicked()
{
    if (finalImage[0].empty()) {
            QMessageBox::warning(this, tr("无已合成图片"), tr("请导入一个文件夹并点击HDR图片合成重试！"));
            if (this->parent()) {   // 检查父窗口是否存在
                this->close();      // 关闭当前窗口
                QCoreApplication::processEvents(); //处理所有未完成的事件
                this->parentWidget()->show();      // 显示父窗口
            }
            return;
    }
    if(flag_cut==1){
        //中值滤波
        medianBlur(cutImage[currentPhotoIndex/6], cutImage[currentPhotoIndex/6], 3);
        QImage image((const unsigned char*)(cutImage[currentPhotoIndex/6].data), cutImage[currentPhotoIndex/6].cols, cutImage[currentPhotoIndex/6].rows, QImage::Format_Grayscale8);
        QPixmap pixmap = QPixmap::fromImage(image).scaled(ui->label_2->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->label_2->setPixmap(pixmap);
        qApp->processEvents();

    }
    else{
    //中值滤波
    medianBlur(finalImage[currentPhotoIndex/6], finalImage[currentPhotoIndex/6], 3);
    QImage image((const unsigned char*)(finalImage[currentPhotoIndex/6].data), finalImage[currentPhotoIndex/6].cols, finalImage[currentPhotoIndex/6].rows, QImage::Format_Grayscale8);
    QPixmap pixmap = QPixmap::fromImage(image).scaled(ui->label_2->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_2->setPixmap(pixmap);
    qApp->processEvents();
    }
}


//双边滤波
void MainWindow::on_pushButton_8_clicked()
{
    if (finalImage[0].empty()) {
            QMessageBox::warning(this, tr("无已合成图片"), tr("请导入一个文件夹并点击HDR图片合成重试！"));
            if (this->parent()) {   // 检查父窗口是否存在
                this->close();      // 关闭当前窗口
                QCoreApplication::processEvents(); //处理所有未完成的事件
                this->parentWidget()->show();      // 显示父窗口
            }
            return;
    }
    if(flag_cut==1){
        Mat shuangbian=cutImage[currentPhotoIndex/6].clone();
        //双边滤波
        bilateralFilter(shuangbian, cutImage[currentPhotoIndex/6], 50, 100, 100);
        QImage image((const unsigned char*)(cutImage[currentPhotoIndex/6].data), cutImage[currentPhotoIndex/6].cols, cutImage[currentPhotoIndex/6].rows, QImage::Format_Grayscale8);
        QPixmap pixmap = QPixmap::fromImage(image).scaled(ui->label_2->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->label_2->setPixmap(pixmap);
        qApp->processEvents();
    }
    else{
    Mat shuangbian=finalImage[currentPhotoIndex/6].clone();
    //双边滤波
    bilateralFilter(shuangbian, finalImage[currentPhotoIndex/6], 50, 100, 100);
    QImage image((const unsigned char*)(finalImage[currentPhotoIndex/6].data), finalImage[currentPhotoIndex/6].cols, finalImage[currentPhotoIndex/6].rows, QImage::Format_Grayscale8);
    QPixmap pixmap = QPixmap::fromImage(image).scaled(ui->label_2->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_2->setPixmap(pixmap);
    qApp->processEvents();
    }
}


//拉普拉斯边缘检测
void MainWindow::on_pushButton_9_clicked()
{
    if (finalImage[0].empty()) {
            QMessageBox::warning(this, tr("无已合成图片"), tr("请导入一个文件夹并点击HDR图片合成重试！"));
            if (this->parent()) {   // 检查父窗口是否存在
                this->close();      // 关闭当前窗口
                QCoreApplication::processEvents(); //处理所有未完成的事件
                this->parentWidget()->show();      // 显示父窗口
            }
            return;
    }

    if(flag_cut==1){
        //Laplacian边缘检测
        cv::Laplacian(cutImage[currentPhotoIndex/6], showImage_bianyuan[currentPhotoIndex/6], CV_8U);
        QImage image((const unsigned char*)(showImage_bianyuan[currentPhotoIndex/6].data), showImage_bianyuan[currentPhotoIndex/6].cols, showImage_bianyuan[currentPhotoIndex/6].rows, QImage::Format_Indexed8);
        QPixmap pixmap = QPixmap::fromImage(image).scaled(ui->label_2->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->label_2->setPixmap(pixmap);
        qApp->processEvents();
    }

    else{
    //Laplacian边缘检测
    cv::Laplacian(finalImage[currentPhotoIndex/6], showImage_bianyuan[currentPhotoIndex/6], CV_8U);
    QImage image((const unsigned char*)(showImage_bianyuan[currentPhotoIndex/6].data), showImage_bianyuan[currentPhotoIndex/6].cols, showImage_bianyuan[currentPhotoIndex/6].rows, QImage::Format_Indexed8);
    QPixmap pixmap = QPixmap::fromImage(image).scaled(ui->label_2->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_2->setPixmap(pixmap);
    qApp->processEvents();
    }
}


//sobel边缘检测
void MainWindow::on_pushButton_10_clicked()
{
    if (finalImage[0].empty()) {
            QMessageBox::warning(this, tr("无已合成图片"), tr("请导入一个文件夹并点击HDR图片合成重试！"));
            if (this->parent()) {   // 检查父窗口是否存在
                this->close();      // 关闭当前窗口
                QCoreApplication::processEvents(); //处理所有未完成的事件
                this->parentWidget()->show();      // 显示父窗口
            }
            return;
    }
    if(flag_cut==1){
        //Sobel边缘检测
        cv::Sobel(cutImage[currentPhotoIndex/6], showImage_bianyuan[currentPhotoIndex/6], CV_8U, 1, 0, 3, 1, 0, cv::BORDER_DEFAULT);
        QImage image((const unsigned char*)(showImage_bianyuan[currentPhotoIndex/6].data), showImage_bianyuan[currentPhotoIndex/6].cols, showImage_bianyuan[currentPhotoIndex/6].rows, QImage::Format_Indexed8);
        QPixmap pixmap = QPixmap::fromImage(image).scaled(ui->label_2->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->label_2->setPixmap(pixmap);
        qApp->processEvents();
    }
    else{
    //Sobel边缘检测
    cv::Sobel(finalImage[currentPhotoIndex/6], showImage_bianyuan[currentPhotoIndex/6], CV_8U, 1, 0, 3, 1, 0, cv::BORDER_DEFAULT);
    QImage image((const unsigned char*)(showImage_bianyuan[currentPhotoIndex/6].data), showImage_bianyuan[currentPhotoIndex/6].cols, showImage_bianyuan[currentPhotoIndex/6].rows, QImage::Format_Indexed8);
    QPixmap pixmap = QPixmap::fromImage(image).scaled(ui->label_2->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_2->setPixmap(pixmap);
    qApp->processEvents();
    }
}


//canny边缘检测
void MainWindow::on_pushButton_11_clicked()
{
    if (finalImage[0].empty()) {
            QMessageBox::warning(this, tr("无已合成图片"), tr("请导入一个文件夹并点击HDR图片合成重试！"));
            if (this->parent()) {   // 检查父窗口是否存在
                this->close();      // 关闭当前窗口
                QCoreApplication::processEvents(); //处理所有未完成的事件
                this->parentWidget()->show();      // 显示父窗口
            }
            return;
    }

    if(flag_cut==1){
        //Canny边缘检测
        cv::Canny(cutImage[currentPhotoIndex/6], showImage_bianyuan[currentPhotoIndex/6], 50, 150);
        QImage image((const unsigned char*)(showImage_bianyuan[currentPhotoIndex/6].data), showImage_bianyuan[currentPhotoIndex/6].cols, showImage_bianyuan[currentPhotoIndex/6].rows, QImage::Format_Indexed8);
        QPixmap pixmap = QPixmap::fromImage(image).scaled(ui->label_2->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->label_2->setPixmap(pixmap);
        qApp->processEvents();
    }

    else
    {
    //Canny边缘检测
    cv::Canny(finalImage[currentPhotoIndex/6], showImage_bianyuan[currentPhotoIndex/6], 50, 150);
    QImage image((const unsigned char*)(showImage_bianyuan[currentPhotoIndex/6].data), showImage_bianyuan[currentPhotoIndex/6].cols, showImage_bianyuan[currentPhotoIndex/6].rows, QImage::Format_Indexed8);
    QPixmap pixmap = QPixmap::fromImage(image).scaled(ui->label_2->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_2->setPixmap(pixmap);
    qApp->processEvents();
    }
}


//放大图片
void MainWindow::on_pushButton_13_clicked()
{
    if (finalImage[0].empty()) {
            QMessageBox::warning(this, tr("无已合成图片"), tr("请导入一个文件夹并点击HDR图片合成重试！"));
            if (this->parent()) {   // 检查父窗口是否存在
                this->close();      // 关闭当前窗口
                QCoreApplication::processEvents(); //处理所有未完成的事件
                this->parentWidget()->show();      // 显示父窗口
            }
            return;
    }
    if(flag_cut==1){
        cv::setNumThreads(4);
        //放大图片
        cv::Mat zoomed = cv::Mat::zeros(
                ui->label_2->height(),
                ui->label_2->width(),
                showImage_cut[currentPhotoIndex/6].type()
            );

            // 计算图像的缩放比例
            double scale = std::min(
                (double)ui->label_2->width() / showImage_cut[currentPhotoIndex/6].cols,
                (double)ui->label_2->height() / showImage_cut[currentPhotoIndex/6].rows
            ) * 1.5;

            // 进行缩放
            cv::resize(
                showImage_cut[currentPhotoIndex/6], showImage_cut[currentPhotoIndex/6],
                cv::Size(), scale, scale,
                cv::INTER_LINEAR
            );

            // 将缩放后的图像显示在 label2 中心处
            QImage qimage((uchar*)showImage_cut[currentPhotoIndex/6].data, showImage_cut[currentPhotoIndex/6].cols, showImage_cut[currentPhotoIndex/6].rows, showImage_cut[currentPhotoIndex/6].step, QImage::Format_Grayscale8);
            qimage = qimage.scaled(qimage.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QPixmap pixmap = QPixmap::fromImage(qimage);
            int x = ui->label_2->width() / 2 - pixmap.width() / 2;
            int y = ui->label_2->height() / 2 - pixmap.height() / 2;
            ui->label_2->setPixmap(pixmap);
            ui->label_2->setGeometry(x, y, pixmap.width(), pixmap.height());
            qApp->processEvents();



    }
    else{
            cv::setNumThreads(4);
            //放大图片
            cv::Mat zoomed = cv::Mat::zeros(
                    ui->label_2->height(),
                    ui->label_2->width(),
                    showImage[currentPhotoIndex/6].type()
                );

                // 计算图像的缩放比例
                double scale = std::min(
                    (double)ui->label_2->width() / showImage[currentPhotoIndex/6].cols,
                    (double)ui->label_2->height() / showImage[currentPhotoIndex/6].rows
                ) * 1.5;

                // 进行缩放
                cv::resize(
                    showImage[currentPhotoIndex/6], showImage[currentPhotoIndex/6],
                    cv::Size(), scale, scale,
                    cv::INTER_LINEAR
                );

                // 将缩放后的图像显示在 label2 中心处
                QImage qimage((uchar*)showImage[currentPhotoIndex/6].data, showImage[currentPhotoIndex/6].cols, showImage[currentPhotoIndex/6].rows, showImage[currentPhotoIndex/6].step, QImage::Format_Grayscale8);
                qimage = qimage.scaled(qimage.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                QPixmap pixmap = QPixmap::fromImage(qimage);
                int x = ui->label_2->width() / 2 - pixmap.width() / 2;
                int y = ui->label_2->height() / 2 - pixmap.height() / 2;
                ui->label_2->setPixmap(pixmap);
                ui->label_2->setGeometry(x, y, pixmap.width(), pixmap.height());
            qApp->processEvents();
    }
}


//缩小
void MainWindow::on_pushButton_14_clicked()
{
    if (finalImage[0].empty()) {
            QMessageBox::warning(this, tr("无已合成图片"), tr("请导入一个文件夹并点击HDR图片合成重试！"));
            if (this->parent()) {   // 检查父窗口是否存在
                this->close();      // 关闭当前窗口
                QCoreApplication::processEvents(); //处理所有未完成的事件
                this->parentWidget()->show();      // 显示父窗口
            }
            return;
    }
    if(flag_cut==1){
        cv::setNumThreads(4);
        cv::Mat zoomed = cv::Mat::zeros(
                ui->label_2->height(),
                ui->label_2->width(),
                showImage_cut[currentPhotoIndex/6].type()
            );

            // 计算图像的缩放比例
            double scale = std::min(
                (double)ui->label_2->width() / showImage_cut[currentPhotoIndex/6].cols,
                (double)ui->label_2->height() / showImage_cut[currentPhotoIndex/6].rows
            ) * 0.75;

            // 进行缩放
            cv::resize(
                showImage_cut[currentPhotoIndex/6], showImage_cut[currentPhotoIndex/6],
                cv::Size(), scale, scale,
                cv::INTER_LINEAR
            );

            // 将缩放后的图像显示在 label2 中心处
            QImage qimage((uchar*)showImage_cut[currentPhotoIndex/6].data, showImage_cut[currentPhotoIndex/6].cols, showImage_cut[currentPhotoIndex/6].rows, showImage_cut[currentPhotoIndex/6].step, QImage::Format_Grayscale8);
            qimage = qimage.scaled(qimage.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QPixmap pixmap = QPixmap::fromImage(qimage);
            int x = ui->label_2->width() / 2 - pixmap.width() / 2;
            int y = ui->label_2->height() / 2 - pixmap.height() / 2;
            ui->label_2->setPixmap(pixmap);
            ui->label_2->setGeometry(x, y, pixmap.width(), pixmap.height());
        qApp->processEvents();
    }
    else{
        cv::setNumThreads(4);
        cv::Mat zoomed = cv::Mat::zeros(
                ui->label_2->height(),
                ui->label_2->width(),
                showImage[currentPhotoIndex/6].type()
            );

            // 计算图像的缩放比例
            double scale = std::min(
                (double)ui->label_2->width() / showImage[currentPhotoIndex/6].cols,
                (double)ui->label_2->height() / showImage[currentPhotoIndex/6].rows
            ) * 0.75;

            // 进行缩放
            cv::resize(
                showImage[currentPhotoIndex/6], showImage[currentPhotoIndex/6],
                cv::Size(), scale, scale,
                cv::INTER_LINEAR
            );

            // 将缩放后的图像显示在 label2 中心处
            QImage qimage((uchar*)showImage[currentPhotoIndex/6].data, showImage[currentPhotoIndex/6].cols, showImage[currentPhotoIndex/6].rows, showImage[currentPhotoIndex/6].step, QImage::Format_Grayscale8);
            qimage = qimage.scaled(qimage.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QPixmap pixmap = QPixmap::fromImage(qimage);
            int x = ui->label_2->width() / 2 - pixmap.width() / 2;
            int y = ui->label_2->height() / 2 - pixmap.height() / 2;
            ui->label_2->setPixmap(pixmap);
            ui->label_2->setGeometry(x, y, pixmap.width(), pixmap.height());
        qApp->processEvents();
    }
}


//旋转
void MainWindow::on_pushButton_12_clicked(){
    if (finalImage[0].empty()) {
            QMessageBox::warning(this, tr("无已合成图片"), tr("请导入一个文件夹并点击HDR图片合成重试！"));
            if (this->parent()) {   // 检查父窗口是否存在
                this->close();      // 关闭当前窗口
                QCoreApplication::processEvents(); //处理所有未完成的事件
                this->parentWidget()->show();      // 显示父窗口
            }
            return;
    }
    if(flag_cut==1) {
        //旋转图片
        cv::setNumThreads(4);
            cv::Mat rotatedImage;
            cv::rotate(showImage_cut[currentPhotoIndex/6], rotatedImage, cv::ROTATE_90_CLOCKWISE);

            // 在label_2中显示图像
            QImage qimg(rotatedImage.data, rotatedImage.cols, rotatedImage.rows, rotatedImage.step, QImage::Format_Grayscale8);
            QPixmap pixmap = QPixmap::fromImage(qimg);

            // 获取label_2的大小
            int label2Width = ui->label_2->width();
            int label2Height = ui->label_2->height();

            // 获取旋转后的图像大小
            int imageWidth = pixmap.width();
            int imageHeight = pixmap.height();

            // 调整图像大小以适应label_2
            if (imageWidth <= label2Width && imageHeight <= label2Height)
            {
                ui->label_2->setPixmap(pixmap.scaled(imageWidth, imageHeight, Qt::KeepAspectRatio));
            }
            else
            {
                float scale = std::min((float)label2Width / imageWidth, (float)label2Height / imageHeight);
                ui->label_2->setPixmap(pixmap.scaled(imageWidth * scale, imageHeight * scale, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            }

            ui->label_2->setScaledContents(true);
            showImage_cut[currentPhotoIndex/6]=rotatedImage.clone();
    }
    else{
        // 旋转90度
        cv::setNumThreads(4);
            cv::Mat rotatedImage;
            cv::rotate(showImage[currentPhotoIndex/6], rotatedImage, cv::ROTATE_90_CLOCKWISE);

            // 在label_2中显示图像
            QImage qimg(rotatedImage.data, rotatedImage.cols, rotatedImage.rows, rotatedImage.step, QImage::Format_Grayscale8);
            QPixmap pixmap = QPixmap::fromImage(qimg);

            // 获取label_2的大小
            int label2Width = ui->label_2->width();
            int label2Height = ui->label_2->height();

            // 获取旋转后的图像大小
            int imageWidth = pixmap.width();
            int imageHeight = pixmap.height();

            // 调整图像大小以适应label_2
            if (imageWidth <= label2Width && imageHeight <= label2Height)
            {
                ui->label_2->setPixmap(pixmap.scaled(imageWidth, imageHeight, Qt::KeepAspectRatio));
            }
            else
            {
                float scale = std::min((float)label2Width / imageWidth, (float)label2Height / imageHeight);
                ui->label_2->setPixmap(pixmap.scaled(imageWidth * scale, imageHeight * scale, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            }

            ui->label_2->setScaledContents(true);
            showImage[currentPhotoIndex/6]=rotatedImage.clone();
    }
}

//原图像上一张
void MainWindow::on_pushButton_17_clicked()
{
    if(currentPhotoIndex==0){
        QMessageBox::warning(this, tr("提示！"), tr("当前已经是原图像第一张了！"));

        if (this->parent()) {   // 检查父窗口是否存在
            this->close();      // 关闭当前窗口
            QCoreApplication::processEvents(); //处理所有未完成的事件
            this->parentWidget()->show();      // 显示父窗口
        }
        return;
    }

    if(flag_cut==1){
        currentPhotoIndex=currentPhotoIndex-1;
        // 将 std::string 转换为 QString
        QString qstr4 = QString::number(currentPhotoIndex+1);
        QString qstr5 = QString::fromStdString(tempfilenames_src[currentPhotoIndex/6][currentPhotoIndex%6]);
        qstr5 = qstr4+"."+qstr5;
        // 将 QString 设置为 QLabel 的文本
        ui->label_5->setText(qstr5);
        QImage image(srcImage[currentPhotoIndex/6][currentPhotoIndex%6].data, srcImage[currentPhotoIndex/6][currentPhotoIndex%6].cols, srcImage[currentPhotoIndex/6][currentPhotoIndex%6].rows, QImage::Format_Grayscale16);
        QPixmap pixmap = QPixmap::fromImage(image).scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->label->setPixmap(pixmap);
        if (!dstImage[0].empty()) {
            QString qstr3 = QString::number((currentPhotoIndex)/6+1);
            QString qstr6 = QString::fromStdString(tempfilenames_dst[currentPhotoIndex/6]);
            qstr6 = qstr3+"."+qstr6;
            // 将 QString 设置为 QLabel 的文本
            ui->label_6->setText(qstr6);

            QImage image1((const unsigned char*)(cutImage[currentPhotoIndex/6].data), cutImage[currentPhotoIndex/6].cols, cutImage[currentPhotoIndex/6].rows, QImage::Format_Grayscale8);
            QPixmap pixmap1 = QPixmap::fromImage(image1).scaled(ui->label_2->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            ui->label_2->setPixmap(pixmap1);
        }
        qApp->processEvents();
    }

    else{
    currentPhotoIndex=currentPhotoIndex-1;
    // 将 std::string 转换为 QString
    QString qstr4 = QString::number(currentPhotoIndex+1);
    QString qstr5 = QString::fromStdString(tempfilenames_src[currentPhotoIndex/6][currentPhotoIndex%6]);
    qstr5 = qstr4+"."+qstr5;
    // 将 QString 设置为 QLabel 的文本
    ui->label_5->setText(qstr5);
    QImage image(srcImage[currentPhotoIndex/6][currentPhotoIndex%6].data, srcImage[currentPhotoIndex/6][currentPhotoIndex%6].cols, srcImage[currentPhotoIndex/6][currentPhotoIndex%6].rows, QImage::Format_Grayscale16);
    QPixmap pixmap = QPixmap::fromImage(image).scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label->setPixmap(pixmap);
    if (!dstImage[0].empty()) {
        QString qstr3 = QString::number((currentPhotoIndex)/6+1);
        QString qstr6 = QString::fromStdString(tempfilenames_dst[currentPhotoIndex/6]);
        qstr6 = qstr3+"."+qstr6;
        // 将 QString 设置为 QLabel 的文本
        ui->label_6->setText(qstr6);
        QImage image1((const unsigned char*)(finalImage[currentPhotoIndex/6].data), finalImage[currentPhotoIndex/6].cols, finalImage[currentPhotoIndex/6].rows, QImage::Format_Grayscale8);
        QPixmap pixmap1 = QPixmap::fromImage(image1).scaled(ui->label_2->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->label_2->setPixmap(pixmap1);
    }
    qApp->processEvents();
    }
}
//原图像下一张
void MainWindow::on_pushButton_15_clicked()
{
    if(currentPhotoIndex==flag_src-1){
        QMessageBox::warning(this, tr("提示！"), tr("当前已经是原图像最后一张了！"));

        if (this->parent()) {   // 检查父窗口是否存在
            this->close();      // 关闭当前窗口
            QCoreApplication::processEvents(); //处理所有未完成的事件
            this->parentWidget()->show();      // 显示父窗口
        }
        return;
    }
    if(flag_cut==1){
        currentPhotoIndex=currentPhotoIndex+1;
        // 将 std::string 转换为 QString
        QString qstr4 = QString::number(currentPhotoIndex+1);
        QString qstr5 = QString::fromStdString(tempfilenames_src[currentPhotoIndex/6][currentPhotoIndex%6]);
        qstr5 = qstr4+"."+qstr5;
        // 将 QString 设置为 QLabel 的文本
        ui->label_5->setText(qstr5);

        QImage image(srcImage[currentPhotoIndex/6][currentPhotoIndex%6].data, srcImage[currentPhotoIndex/6][currentPhotoIndex%6].cols, srcImage[currentPhotoIndex/6][currentPhotoIndex%6].rows, QImage::Format_Grayscale16);
        QPixmap pixmap = QPixmap::fromImage(image).scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->label->setPixmap(pixmap);
        if (!dstImage[0].empty()) {
            QString qstr3 = QString::number((currentPhotoIndex)/6+1);
            QString qstr6 = QString::fromStdString(tempfilenames_dst[currentPhotoIndex/6]);
            qstr6 = qstr3+"."+qstr6;
            // 将 QString 设置为 QLabel 的文本
            ui->label_6->setText(qstr6);
            QImage image1((const unsigned char*)(cutImage[currentPhotoIndex/6].data), cutImage[currentPhotoIndex/6].cols, cutImage[currentPhotoIndex/6].rows, QImage::Format_Grayscale8);
            QPixmap pixmap1 = QPixmap::fromImage(image1).scaled(ui->label_2->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            ui->label_2->setPixmap(pixmap1);
        }
        qApp->processEvents();

    }
    else{
    currentPhotoIndex=currentPhotoIndex+1;
    // 将 std::string 转换为 QString
    QString qstr4 = QString::number(currentPhotoIndex+1);
    QString qstr5 = QString::fromStdString(tempfilenames_src[currentPhotoIndex/6][currentPhotoIndex%6]);
    qstr5 = qstr4+"."+qstr5;
    // 将 QString 设置为 QLabel 的文本
    ui->label_5->setText(qstr5);

    QImage image(srcImage[currentPhotoIndex/6][currentPhotoIndex%6].data, srcImage[currentPhotoIndex/6][currentPhotoIndex%6].cols, srcImage[currentPhotoIndex/6][currentPhotoIndex%6].rows, QImage::Format_Grayscale16);
    QPixmap pixmap = QPixmap::fromImage(image).scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label->setPixmap(pixmap);
    if (!dstImage[0].empty()) {
        QString qstr3 = QString::number((currentPhotoIndex)/6+1);
        QString qstr6 = QString::fromStdString(tempfilenames_dst[currentPhotoIndex/6]);
        qstr6 = qstr3+"."+qstr6;
        // 将 QString 设置为 QLabel 的文本
        ui->label_6->setText(qstr6);
        QImage image1((const unsigned char*)(finalImage[currentPhotoIndex/6].data), finalImage[currentPhotoIndex/6].cols, finalImage[currentPhotoIndex/6].rows, QImage::Format_Grayscale8);
        QPixmap pixmap1 = QPixmap::fromImage(image1).scaled(ui->label_2->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->label_2->setPixmap(pixmap1);
    }
    qApp->processEvents();
    }
}

//结果图像上一张
void MainWindow::on_pushButton_16_clicked()
{
    if(currentPhotoIndex/6==0){
        QMessageBox::warning(this, tr("提示！"), tr("当前已经是合成图的第一张了！"));

        if (this->parent()) {   // 检查父窗口是否存在
            this->close();      // 关闭当前窗口
            QCoreApplication::processEvents(); //处理所有未完成的事件
            this->parentWidget()->show();      // 显示父窗口
        }
        return;
    }

    if(flag_cut==1){
        currentPhotoIndex=((currentPhotoIndex/6)-1)*6;

        QString qstr3 = QString::number((currentPhotoIndex)/6+1);
        QString qstr6 = QString::fromStdString(tempfilenames_dst[currentPhotoIndex/6]);
        qstr6 = qstr3+"."+qstr6;
        // 将 QString 设置为 QLabel 的文本
        ui->label_6->setText(qstr6);

        // 将 std::string 转换为 QString
        QString qstr4 = QString::number(currentPhotoIndex+1);
        QString qstr5 = QString::fromStdString(tempfilenames_src[currentPhotoIndex/6][currentPhotoIndex%6]);
        qstr5 = qstr4+"."+qstr5;
        // 将 QString 设置为 QLabel 的文本
        ui->label_5->setText(qstr5);

        QImage image(srcImage[currentPhotoIndex/6][currentPhotoIndex%6].data, srcImage[currentPhotoIndex/6][currentPhotoIndex%6].cols, srcImage[currentPhotoIndex/6][currentPhotoIndex%6].rows, QImage::Format_Grayscale16);
        QPixmap pixmap = QPixmap::fromImage(image).scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->label->setPixmap(pixmap);

        QImage image1((const unsigned char*)(cutImage[(currentPhotoIndex/6)].data), cutImage[(currentPhotoIndex/6)].cols, cutImage[(currentPhotoIndex/6)].rows, QImage::Format_Grayscale8);
        QPixmap pixmap1 = QPixmap::fromImage(image1).scaled(ui->label_2->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->label_2->setPixmap(pixmap1);
        qApp->processEvents();

    }


    else{
    currentPhotoIndex=((currentPhotoIndex/6)-1)*6;

    QString qstr3 = QString::number((currentPhotoIndex)/6+1);
    QString qstr6 = QString::fromStdString(tempfilenames_dst[currentPhotoIndex/6]);
    qstr6 = qstr3+"."+qstr6;
    // 将 QString 设置为 QLabel 的文本
    ui->label_6->setText(qstr6);

    // 将 std::string 转换为 QString
    QString qstr4 = QString::number(currentPhotoIndex+1);
    QString qstr5 = QString::fromStdString(tempfilenames_src[currentPhotoIndex/6][currentPhotoIndex%6]);
    qstr5 = qstr4+"."+qstr5;
    // 将 QString 设置为 QLabel 的文本
    ui->label_5->setText(qstr5);

    QImage image(srcImage[currentPhotoIndex/6][currentPhotoIndex%6].data, srcImage[currentPhotoIndex/6][currentPhotoIndex%6].cols, srcImage[currentPhotoIndex/6][currentPhotoIndex%6].rows, QImage::Format_Grayscale16);
    QPixmap pixmap = QPixmap::fromImage(image).scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label->setPixmap(pixmap);

    QImage image1((const unsigned char*)(finalImage[(currentPhotoIndex/6)].data), finalImage[(currentPhotoIndex/6)].cols, finalImage[(currentPhotoIndex/6)].rows, QImage::Format_Grayscale8);
    QPixmap pixmap1 = QPixmap::fromImage(image1).scaled(ui->label_2->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label_2->setPixmap(pixmap1);
    qApp->processEvents();
    }
}

//结果图下一张
void MainWindow::on_pushButton_18_clicked()
{
    if(currentPhotoIndex/6==flag_dst%6){
        QMessageBox::warning(this, tr("提示！"), tr("当前已经是合成图的最后一张了！"));

        if (this->parent()) {   // 检查父窗口是否存在
            this->close();      // 关闭当前窗口
            QCoreApplication::processEvents(); //处理所有未完成的事件
            this->parentWidget()->show();      // 显示父窗口
        }
        return;
    }

    if(flag_cut==1){
        currentPhotoIndex=((currentPhotoIndex/6)+1)*6;

        QString qstr3 = QString::number((currentPhotoIndex)/6+1);
        QString qstr6 = QString::fromStdString(tempfilenames_dst[currentPhotoIndex/6]);
        qstr6 = qstr3+"."+qstr6;
        // 将 QString 设置为 QLabel 的文本
        ui->label_6->setText(qstr6);

        // 将 std::string 转换为 QString
        QString qstr4 = QString::number(currentPhotoIndex+1);
        QString qstr5 = QString::fromStdString(tempfilenames_src[currentPhotoIndex/6][currentPhotoIndex%6]);
        qstr5 = qstr4+"."+qstr5;
        // 将 QString 设置为 QLabel 的文本
        ui->label_5->setText(qstr5);

        QImage image(srcImage[currentPhotoIndex/6][currentPhotoIndex%6].data, srcImage[currentPhotoIndex/6][currentPhotoIndex%6].cols, srcImage[currentPhotoIndex/6][currentPhotoIndex%6].rows, QImage::Format_Grayscale16);
        QPixmap pixmap = QPixmap::fromImage(image).scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->label->setPixmap(pixmap);

        QImage image1((const unsigned char*)(cutImage[(currentPhotoIndex/6)].data), cutImage[(currentPhotoIndex/6)].cols, cutImage[(currentPhotoIndex/6)].rows, QImage::Format_Grayscale8);
        QPixmap pixmap1 = QPixmap::fromImage(image1).scaled(ui->label_2->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->label_2->setPixmap(pixmap1);
        qApp->processEvents();

    }


    else{
        currentPhotoIndex=((currentPhotoIndex/6)+1)*6;

        QString qstr3 = QString::number((currentPhotoIndex)/6+1);
        QString qstr6 = QString::fromStdString(tempfilenames_dst[currentPhotoIndex/6]);
        qstr6 = qstr3+"."+qstr6;
        // 将 QString 设置为 QLabel 的文本
        ui->label_6->setText(qstr6);

        // 将 std::string 转换为 QString
        QString qstr4 = QString::number(currentPhotoIndex+1);
        QString qstr5 = QString::fromStdString(tempfilenames_src[currentPhotoIndex/6][currentPhotoIndex%6]);
        qstr5 = qstr4+"."+qstr5;
        // 将 QString 设置为 QLabel 的文本
        ui->label_5->setText(qstr5);

        QImage image(srcImage[currentPhotoIndex/6][currentPhotoIndex%6].data, srcImage[currentPhotoIndex/6][currentPhotoIndex%6].cols, srcImage[currentPhotoIndex/6][currentPhotoIndex%6].rows, QImage::Format_Grayscale16);
        QPixmap pixmap = QPixmap::fromImage(image).scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->label->setPixmap(pixmap);

        QImage image1((const unsigned char*)(finalImage[(currentPhotoIndex/6)].data), finalImage[(currentPhotoIndex/6)].cols, finalImage[(currentPhotoIndex/6)].rows, QImage::Format_Grayscale8);
        QPixmap pixmap1 = QPixmap::fromImage(image1).scaled(ui->label_2->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->label_2->setPixmap(pixmap1);
        qApp->processEvents();
    }
}

//裁剪边框的调整与预览
void MainWindow::on_action_yulan_triggered()
{
    if(finalImage[0].empty()){
        QMessageBox::warning(this, tr("提示！"), tr("暂无已合成图像！"));

        if (this->parent()) {   // 检查父窗口是否存在
            this->close();      // 关闭当前窗口
            QCoreApplication::processEvents(); //处理所有未完成的事件
            this->parentWidget()->show();      // 显示父窗口
        }
        return;

    }
    else{
    caijian *configWindow = new caijian;
    configWindow->setWindowTitle("裁剪边框调整与预览");
    configWindow->show();
    }
}

//映射曲线拟合
void MainWindow::on_action_inputdata_triggered()
{
    if(finalImage[0].empty()){
        QMessageBox::warning(this, tr("提示！"), tr("请先进行数据文件的合成以及HDR图像曝光融合！"));

        if (this->parent()) {   // 检查父窗口是否存在
            this->close();      // 关闭当前窗口
            QCoreApplication::processEvents(); //处理所有未完成的事件
            this->parentWidget()->show();      // 显示父窗口
        }
        return;
    }
    else{
        flag_change=0;
        flag_quxian=1;
        quxian *configWindow = new quxian;
        configWindow->setWindowTitle("映射曲线");
        configWindow->show();
        // 连接子窗口的信号到MainWindow中的槽函数
        connect(configWindow, &quxian::updateImageRequested, this, &MainWindow::updateLabelImage);
    }
}


void MainWindow::updateLabelImage()
{
    if(flag_cut==1){
        currentPhotoIndex=0;
        // 使用新图像更新标签中的图像
        // 将 std::string 转换为 QString
        QString qstr4 = QString::number(currentPhotoIndex+1);
        QString qstr5 = QString::fromStdString(tempfilenames_src[currentPhotoIndex/6][currentPhotoIndex%6]);
        qstr5 = qstr4+"."+qstr5;
        // 将 QString 设置为 QLabel 的文本
        ui->label_5->setText(qstr5);

        QImage image0(srcImage[0][0].data, srcImage[0][0].cols, srcImage[0][0].rows, QImage::Format_Grayscale16);
        QPixmap pixmap0 = QPixmap::fromImage(image0).scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->label->setPixmap(pixmap0);

        QString qstr3 = QString::number((currentPhotoIndex/6)*6+currentPhotoIndex%6+1);
        QString qstr6 = QString::fromStdString(tempfilenames_dst[currentPhotoIndex/6]);
        qstr6 = qstr3+"."+qstr6;
        // 将 QString 设置为 QLabel 的文本
        ui->label_6->setText(qstr6);

        QImage image1((const unsigned char*)(cutImage[0].data), cutImage[0].cols, cutImage[0].rows, QImage::Format_Grayscale8);
        QPixmap pixmap = QPixmap::fromImage(image1).scaled(ui->label_2->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->label_2->setPixmap(pixmap);

    }
    else{
        currentPhotoIndex=0;
        // 使用新图像更新标签中的图像
        // 将 std::string 转换为 QString
        QString qstr4 = QString::number(currentPhotoIndex+1);
        QString qstr5 = QString::fromStdString(tempfilenames_src[currentPhotoIndex/6][currentPhotoIndex%6]);
        qstr5 = qstr4+"."+qstr5;
        // 将 QString 设置为 QLabel 的文本
        ui->label_5->setText(qstr5);

        QImage image0(srcImage[0][0].data, srcImage[0][0].cols, srcImage[0][0].rows, QImage::Format_Grayscale16);
        QPixmap pixmap0 = QPixmap::fromImage(image0).scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->label->setPixmap(pixmap0);

        QString qstr3 = QString::number((currentPhotoIndex/6)*6+currentPhotoIndex%6+1);
        QString qstr6 = QString::fromStdString(tempfilenames_dst[currentPhotoIndex/6]);
        qstr6 = qstr3+"."+qstr6;
        // 将 QString 设置为 QLabel 的文本
        ui->label_6->setText(qstr6);

        QImage image1((const unsigned char*)(finalImage[0].data), finalImage[0].cols, finalImage[0].rows, QImage::Format_Grayscale8);
        QPixmap pixmap = QPixmap::fromImage(image1).scaled(ui->label_2->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->label_2->setPixmap(pixmap);
    }
}


//新建映射曲线
void MainWindow::on_action_new_map_triggered()
{
    if(finalImage[0].empty()){
        QMessageBox::warning(this, tr("提示！"), tr("请先进行数据文件的合成以及HDR图像曝光融合！"));

        if (this->parent()) {   // 检查父窗口是否存在
            this->close();      // 关闭当前窗口
            QCoreApplication::processEvents(); //处理所有未完成的事件
            this->parentWidget()->show();      // 显示父窗口
        }
        return;
    }
    else{
        flag_change=0;
        flag_quxian=0;
        quxian *configWindow = new quxian;
        configWindow->setWindowTitle("映射曲线");
        configWindow->show();
        // 连接子窗口的信号到MainWindow中的槽函数
        connect(configWindow, &quxian::updateImageRequested, this, &MainWindow::updateLabelImage);

    }
}


