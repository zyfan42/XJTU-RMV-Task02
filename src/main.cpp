#include <iostream>
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

using namespace cv;
using namespace std;

// 将图像转换为灰度图
void convertToGray(const Mat &inputImage, Mat &outputImage)
{
    cvtColor(inputImage, outputImage, COLOR_BGR2GRAY);
}

// 将图像转换为HSV
void convertToHSV(const Mat &inputImage, Mat &outputImage)
{
    cvtColor(inputImage, outputImage, COLOR_BGR2HSV);
}

// 应用均值模糊
void applyMeanBlur(const Mat &inputImage, Mat &outputImage, int kernelSize)
{
    blur(inputImage, outputImage, Size(kernelSize, kernelSize));
}

// 应用高斯模糊
void applyGaussianBlur(const Mat &inputImage, Mat &outputImage, int kernelSize)
{
    GaussianBlur(inputImage, outputImage, Size(kernelSize, kernelSize), 0);
}

// 使用HSV提取红色区域
void extractRedColor(const Mat &hsvImage, Mat &mask)
{
    Mat lowerRed, upperRed;
    inRange(hsvImage, Scalar(0, 120, 70), Scalar(10, 255, 255), lowerRed);
    inRange(hsvImage, Scalar(170, 120, 70), Scalar(180, 255, 255), upperRed);
    mask = lowerRed | upperRed;
}

// 查找红色轮廓和边界框
void findRedContours(const Mat &mask, Mat &outputImage, vector<vector<Point>> &contours, ofstream &logFile)
{
    findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    for (size_t i = 0; i < contours.size(); i++)
    {
        drawContours(outputImage, contours, (int)i, Scalar(0, 255, 0), 2);
        Rect boundingBox = boundingRect(contours[i]);
        rectangle(outputImage, boundingBox.tl(), boundingBox.br(), Scalar(255, 0, 0), 2);
        double area = contourArea(contours[i]);
        logFile << "Contour " << i << " Area: " << area << endl;
    }
}

// 膨胀图像
void dilateImage(const Mat &inputImage, Mat &outputImage)
{
    Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
    dilate(inputImage, outputImage, element);
}

// 腐蚀图像
void erodeImage(const Mat &inputImage, Mat &outputImage)
{
    Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
    erode(inputImage, outputImage, element);
}

// 阈值图像
void thresholdImage(const Mat &inputImage, Mat &outputImage)
{
    threshold(inputImage, outputImage, 100, 255, THRESH_BINARY);
}

// 泛洪填充图像
void floodFillImage(Mat &image)
{
    Point seedPoint = Point(10, 10);
    floodFill(image, seedPoint, Scalar(255, 0, 0));
}

// 按给定角度旋转图像
void rotateImage(const Mat &inputImage, Mat &outputImage, double angle)
{
    Point2f center(inputImage.cols / 2.0, inputImage.rows / 2.0);
    Mat rotationMatrix = getRotationMatrix2D(center, angle, 1.0);
    warpAffine(inputImage, outputImage, rotationMatrix, inputImage.size());
}

// 裁剪图像到左上四分之一
void cropImage(const Mat &inputImage, Mat &outputImage)
{
    Rect cropRegion(0, 0, inputImage.cols / 2, inputImage.rows / 2);
    outputImage = inputImage(cropRegion).clone();
}

// 在图像上绘制形状和文本
void drawShapes(Mat &image)
{
    circle(image, Point(100, 100), 50, Scalar(255, 0, 0), 2);
    rectangle(image, Point(200, 200), Point(300, 300), Scalar(0, 255, 0), 2);
    putText(image, "OpenCV Drawing", Point(50, 400), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);
}

int main()
{
    // 检查并创建资产目录
    string assetsDir = "../assets";
    if (!fs::exists(assetsDir))
    {
        fs::create_directory(assetsDir);
    }

    // 打开日志文件
    ofstream logFile("../assets/contour_areas.log");
    if (!logFile.is_open())
    {
        cerr << "错误: 无法打开日志文件。" << endl;
        return -1;
    }

    // 读取图像
    Mat image = imread("../resources/test_image.png");
    if (image.empty())
    {
        cerr << "错误: 无法打开或找到图像。" << endl;
        return -1;
    }

    // 转换为灰度图
    Mat grayImage;
    convertToGray(image, grayImage);
    imwrite("../assets/gray_image.jpg", grayImage);

    // 转换为HSV
    Mat hsvImage;
    convertToHSV(image, hsvImage);
    imwrite("../assets/hsv_image.jpg", hsvImage);

    // 提取红色区域
    Mat redMask;
    extractRedColor(hsvImage, redMask);
    imwrite("../assets/red_mask.jpg", redMask);

    // 查找红色轮廓和边界框
    Mat contourImage = image.clone();
    vector<vector<Point>> contours;
    findRedContours(redMask, contourImage, contours, logFile);
    imwrite("../assets/contour_image.jpg", contourImage);

    // 应用高斯模糊
    Mat blurredImage;
    applyGaussianBlur(image, blurredImage, 15);
    imwrite("../assets/blurred_image.jpg", blurredImage);

    // 膨胀和腐蚀图像
    Mat dilatedImage, erodedImage;
    dilateImage(redMask, dilatedImage);
    erodeImage(dilatedImage, erodedImage);
    imwrite("../assets/eroded_image.jpg", erodedImage);

    // 旋转图像35度
    Mat rotatedImage;
    rotateImage(image, rotatedImage, 35);
    imwrite("../assets/rotated_image.jpg", rotatedImage);

    // 裁剪图像
    Mat croppedImage;
    cropImage(image, croppedImage);
    imwrite("../assets/cropped_image.jpg", croppedImage);

    // 在图像上绘制形状和文本
    drawShapes(image);
    imwrite("../assets/drawn_image.jpg", image);

    // 关闭日志文件
    logFile.close();

    return 0;
}
