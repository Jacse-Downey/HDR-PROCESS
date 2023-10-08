#ifndef PROCESS_H
#define PROCESS_H

#include "opencv2/core/cvstd.hpp"
#include <QObject>
#include <vector>
//#include <opencv2/opencv.hpp>
namespace cv {
    class Mat;
}

extern cv::Mat srcImage[600][6];
extern cv::Mat dstImage[600];
extern cv::Mat finalImage[600];
extern cv::Mat finalImage_map[600];
extern int flag_src;
extern int flag_dst;
extern std::string tempfilenames[600];
extern std::string tempfilenames_dst[600];
extern std::string tempfilenames_src[600][6];
extern cv::Mat tempImage[600][6];
extern cv::Mat showImage[600];
extern cv::Mat showImage_cut[600];
extern cv::Mat showImage_bianyuan[600];
extern cv::Mat cutImage[600];
extern cv::Mat cutImage_map[600];
extern int flag_cut;
extern int flag_quxian;
extern std::vector<int> counts;
extern double a_map;
extern double b_map;
extern double c_map;
extern cv::Mat finalImage_show[600];
extern double csvfile1[10001][2];
extern int flag_biankaung;
extern int maxvalue;

class Process
{
public:
    Process();
};

#endif // PROCESS_H

