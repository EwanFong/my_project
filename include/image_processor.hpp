#pragma once
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>
#include <mutex>
#include <QString>

class ImageProcessor {
public:
    enum ProcessingMode {
        DRAW_NONE = 0,
        DRAW_FREEHAND = 1,
        DRAW_RECTANGLE = 2,
        DRAW_CIRCLE = 3,
        DRAW_TRIANGLE = 4,
        DRAW_MOSAIC = 5
    };

    static QString toolsToString(ProcessingMode tools)
    {
        switch (tools) {
        case DRAW_NONE:
            return "None";
        case DRAW_FREEHAND:
            return "Freehand";
        case DRAW_RECTANGLE:
            return "Rectangel";
        case DRAW_CIRCLE:
            return "Circle";
        case DRAW_TRIANGLE:
            return "Triangle";
        case DRAW_MOSAIC:
            return "Mosaic";
        default:
            break;
        }
    }

    explicit ImageProcessor(const cv::Mat& src);
    ~ImageProcessor() = default;

    static void onMouse(int event, int x, int y, int flags, void* userdata);

    void refreshDisplay() { working_ = original_.clone();}
    void setMode(ProcessingMode newMode);

    void resetTrianglePoints() { trianglePoints.clear();}

    friend class MainWindow;

    int brushSize = 1;
    int mosaicSize = 1;
    int Border_Width = 1;

    cv::Mat original_;
    cv::Mat working_;
    cv::Mat tempWorking_;



private:    
    void applyMosaic(cv::Mat& image, cv::Rect roi, int blockSize);
    void lineBresenham(cv::Point p1, cv::Point p2, std::vector<cv::Point>& points);
    std::vector<cv::Point> trianglePoints;
    std::mutex mutex_;

    cv::Point startPoint_;
    cv::Point lastBrushPos = cv::Point(-1, -1);
    cv::Mat persistenceLayer_;
    cv::Mat previewLayer_;
    ProcessingMode currentMode_ = DRAW_NONE;
};
