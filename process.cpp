#include "process.h"
#include <QMetaType>
#include <opencv2/opencv.hpp>

cv::Mat srcImage[600][6];
cv::Mat dstImage[600];
cv::Mat finalImage[600];
cv::Mat finalImage_map[600];
cv::Mat tempImage[600][6];
cv::Mat showImage[600];
cv::Mat cutImage[600];
cv::Mat cutImage_map[600];
cv::Mat showImage_cut[600];
cv::Mat showImage_bianyuan[600];
std::vector<int> counts;
double csvfile1[10001][2];
int flag_biankaung=0;
int maxvalue;


Process::Process()
{

}
