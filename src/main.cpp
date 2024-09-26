#include <iostream>
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;
using namespace cv;
using namespace std;

// 转换颜色空间
void convertColor(const Mat &inputImage, Mat &outputImage, int conversionCode)
{
    cvtColor(inputImage, outputImage, conversionCode);
}

// 应用模糊处理
void applyBlur(const Mat &inputImage, Mat &outputImage, int kernelSize, bool isGaussian = false)
{
    if (isGaussian)
    {
        GaussianBlur(inputImage, outputImage, Size(kernelSize, kernelSize), 0);
    }
    else
    {
        blur(inputImage, outputImage, Size(kernelSize, kernelSize));
    }
}

// 提取红色区域
void extractRedColor(const Mat &hsvImage, Mat &mask)
{
    Mat lowerRed, upperRed;

    inRange(hsvImage, Scalar(0, 43, 46), Scalar(10, 255, 255), lowerRed);
    inRange(hsvImage, Scalar(156, 43, 46), Scalar(180, 255, 255), upperRed);
    mask = lowerRed | upperRed;
}

// 查找并绘制轮廓
void findAndDrawContours(const Mat &mask, Mat &outputImage, ofstream &logFile)
{
    vector<vector<Point>> contours;
    findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    for (size_t i = 0; i < contours.size(); i++)
    {
        drawContours(outputImage, contours, (int)i, Scalar(0, 255, 0), 2);
        Rect boundingBox = boundingRect(contours[i]);
        rectangle(outputImage, boundingBox.tl(), boundingBox.br(), Scalar(255, 0, 0), 2);
        double area = contourArea(contours[i]);
        logFile << "Contour " << i << " Area: " << area << endl;
        cout << "Contour " << i << " Area: " << area << endl;

        // Display area on the image
        string areaText = "Area: " + to_string(area);
        putText(outputImage, areaText, boundingBox.tl() - Point(0, 10), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 1);
    }
}

// 图形学处理
void morphologicalProcess(const Mat &inputImage, Mat &outputImage, int operationType, int kernelSize = 5)
{
    Mat element = getStructuringElement(MORPH_RECT, Size(kernelSize, kernelSize));
    morphologyEx(inputImage, outputImage, operationType, element);
}

// 应用泛洪填充
void applyFloodFill(Mat &image)
{
    Point seedPoint = Point(100, 100);
    floodFill(image, seedPoint, Scalar(0, 255, 0));
}

// 旋转图像
void rotateImage(const Mat &inputImage, Mat &outputImage, double angle)
{
    Point2f center(inputImage.cols / 2.0, inputImage.rows / 2.0);
    Mat rotationMatrix = getRotationMatrix2D(center, angle, 1.0);
    warpAffine(inputImage, outputImage, rotationMatrix, inputImage.size());
}

// 裁剪图像
void cropImage(const Mat &inputImage, Mat &outputImage)
{
    Rect cropRegion(0, 0, inputImage.cols / 2, inputImage.rows / 2);
    outputImage = inputImage(cropRegion).clone();
}

// 绘制形状
void drawShapes(Mat &image)
{
    Scalar deepRed(128, 0, 0);
    Scalar purple(128, 0, 128);
    Scalar pink(255, 105, 180);

    circle(image, Point(250, 250), 150, deepRed, -1);

    rectangle(image, Point(100, 400), Point(500, 600), purple, -1);

    putText(image, "Fate: GO", Point(100, 350), FONT_HERSHEY_TRIPLEX, 3, pink, 4);
}

int main()
{
    // 资源目录路径
    string assetsDir = "../assets";
    if (!fs::exists(assetsDir))
    {
        fs::create_directory(assetsDir);
    }

    // 打开日志文件
    ofstream logFile("../assets/contour_areas.log");
    if (!logFile.is_open())
    {
        return -1;
    }

    // 读取图像
    Mat image = imread("../resources/test_image.png");
    if (image.empty())
    {
        return -1;
    }

    Mat grayImage, hsvImage;
    // 转换为灰度图像
    convertColor(image, grayImage, COLOR_BGR2GRAY);
    imwrite("../assets/gray_image.jpg", grayImage);

    // 转换为HSV图像
    convertColor(image, hsvImage, COLOR_BGR2HSV);
    imwrite("../assets/hsv_image.jpg", hsvImage);

    Mat blurredImage;
    // 应用高斯模糊
    applyBlur(image, blurredImage, 15, true);
    imwrite("../assets/gaussian_blurred_image.jpg", blurredImage);

    // 应用均值模糊
    applyBlur(image, blurredImage, 15, false);
    imwrite("../assets/mean_blurred_image.jpg", blurredImage);

    Mat redMask;
    // 提取红色区域
    extractRedColor(hsvImage, redMask);
    imwrite("../assets/red_mask.jpg", redMask);

    Mat contourImage = image.clone();
    // 查找并绘制轮廓
    findAndDrawContours(redMask, contourImage, logFile);
    imwrite("../assets/contour_image.jpg", contourImage);

    Mat thresholdedImage, processedImage;
    // 应用阈值处理
    threshold(grayImage, thresholdedImage, 100, 255, THRESH_BINARY);
    imwrite("../assets/thresholded_image.jpg", thresholdedImage);

    // 图形学处理
    morphologicalProcess(thresholdedImage, processedImage, MORPH_DILATE);
    morphologicalProcess(processedImage, processedImage, MORPH_ERODE);
    imwrite("../assets/processed_image.jpg", processedImage);

    Mat floodFilledImage = processedImage.clone();
    // 应用泛洪填充
    applyFloodFill(floodFilledImage);
    imwrite("../assets/flood_filled_image.jpg", floodFilledImage);

    // 绘制形状
    drawShapes(image);
    imwrite("../assets/drawn_image.jpg", image);

    Mat rotatedImage, croppedImage;
    // 旋转图像
    rotateImage(image, rotatedImage, 35);
    imwrite("../assets/rotated_image.jpg", rotatedImage);

    // 裁剪图像
    cropImage(image, croppedImage);
    imwrite("../assets/cropped_image.jpg", croppedImage);

    logFile.close();

    return 0;
}
