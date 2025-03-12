#include "image_processor.hpp"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <mutex>

ImageProcessor::ImageProcessor(const cv::Mat& src)
    : original_(src.clone()),
    working_(src.clone()),
    tempWorking_(src.clone()),
    brushSize(3),
    mosaicSize(5),
    Border_Width(4)
{
}

void ImageProcessor::setMode(ProcessingMode newMode)
{
    currentMode_ = newMode;
    resetTrianglePoints();
}

void ImageProcessor::onMouse(int event, int x, int y, int flags, void* userdata)
{
    ImageProcessor* processor = static_cast<ImageProcessor*>(userdata);
    std::lock_guard<std::mutex> lock(processor->mutex_);

    cv::Point currentPos(x,y);

    switch (event) {
        case cv::EVENT_LBUTTONDOWN:{
            processor->startPoint_ = currentPos;
            processor->tempWorking_ = processor->working_.clone();
            if (processor->currentMode_ == DRAW_TRIANGLE) {
                processor->trianglePoints.push_back(currentPos);
                cv::circle(processor->working_, currentPos, 4, cv::Scalar(0,0,0), -1);
            }
            if (processor->currentMode_ == DRAW_FREEHAND) {
                processor->lastBrushPos = currentPos;
            }
            break;
        }

        case cv::EVENT_MOUSEMOVE:{
            if (flags & cv::EVENT_FLAG_LBUTTON) {
                processor->working_ = processor->tempWorking_.clone();
                switch (processor->currentMode_) {
                    case DRAW_NONE:
                    break;
                    case DRAW_RECTANGLE:{
                            cv::rectangle(processor->working_,
                                          processor->startPoint_, currentPos,
                                          cv::Scalar(0,0,0), processor->Border_Width);
                            break;
                    }

                    case DRAW_CIRCLE: {
                        int radius = cv::norm(currentPos - processor->startPoint_);
                        cv::circle(processor->working_,
                                   processor->startPoint_, radius,
                                   cv::Scalar(0,0,0), processor->Border_Width);
                        break;
                    }

                    case DRAW_FREEHAND:{
                        if (processor->lastBrushPos.x >= 0) {
                            std::vector<cv::Point> points;
                            processor->lineBresenham(processor->lastBrushPos, cv::Point(x,y), points);

                            for (const auto& pt : points){
                                cv::circle (processor->original_, pt,
                                           processor->brushSize,
                                           cv::Scalar(0,0,0),
                                           cv::FILLED);
                            }

                            processor->working_ = processor->original_.clone();
                        }
                        processor->lastBrushPos = currentPos;
                        break;
                    }

                    case DRAW_MOSAIC: {
                        int blockSize = processor->mosaicSize * 5;

                        cv::Point pt1 = processor->startPoint_;
                        cv::Point pt2 = currentPos;
                        cv::Rect roi(cv::Rect(pt1, pt2) & cv::Rect(0, 0,processor->tempWorking_.cols, processor->tempWorking_.rows));

                        if (roi.area() > 0 && blockSize > 0){
                            cv::Mat temp = processor->tempWorking_.clone();
                            processor->applyMosaic(temp, roi, blockSize);
                            processor->working_ = temp;
                        }
                        break;
                    }

                    case DRAW_TRIANGLE: {
                        if (!processor->trianglePoints.empty()){
                            processor->working_ = processor->original_.clone();
                            if (processor->trianglePoints.size() >= 1){
                                for (size_t i = 1; i < processor->trianglePoints.size(); ++i){
                                    cv::line(processor->working_, processor->trianglePoints[i -1 ], processor->trianglePoints[i], cv::Scalar(0,0,0), processor->Border_Width);
                                }
                                cv::line(processor->working_, processor->trianglePoints.back(), currentPos, cv::Scalar(0,0,0), processor->Border_Width);
                            }
                        }
                    }
                }
            }
            break;
        }

        case cv::EVENT_LBUTTONUP:{
            if (processor->currentMode_ == DRAW_TRIANGLE) {
                if (processor->trianglePoints.size() == 3){
                    for(int i = 0; i < 3; ++i){
                        cv::line(processor->original_, processor->trianglePoints[i],processor->trianglePoints[(i+1)%3], cv::Scalar(0,0,0), processor->Border_Width);
                    }
                    processor->working_ = processor->original_.clone();
                    processor->resetTrianglePoints();
                }
            } else {
                processor->original_ = processor->working_.clone();
            }

            processor->lastBrushPos = cv::Point(-1, -1);
            break;
        }
    }
}

void ImageProcessor::lineBresenham(cv::Point p1, cv::Point p2, std::vector<cv::Point> &points)
{
    int dx = abs(p2.x - p1.x);
    int dy = -abs(p2.y - p1.y);
    int sx = p1.x < p2.x ? 1 : -1;
    int sy = p1.y < p2.y ? 1 : -1;
    int err = dx + dy;

    while (true) {
        points.push_back(cv::Point(p1.x, p1.y));
        if (p1.x == p2.x && p1.y == p2.y) break;
        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            p1.x += sx;
        }
        if (e2 <= dx) {
            err += dx;
            p1.y += sy;
        }
    }
}

void ImageProcessor::applyMosaic(cv::Mat& image, cv::Rect roi, int blockSize) {
    if (roi.empty() || blockSize <= 0) return;

    blockSize = std::max(blockSize, 1);

    int cols = std::max(roi.width / blockSize, 1);
    int rows = std::max(roi.height / blockSize, 1);

    cv::Mat roiMat = image(roi);
    cv::Mat small;

    cv::resize(roiMat, small, cv::Size(cols, rows), 0, 0, cv::INTER_NEAREST);
    cv::resize(small, roiMat, roiMat.size(), 0, 0, cv::INTER_NEAREST);
}


