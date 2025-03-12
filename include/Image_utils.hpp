#pragma once

#include <opencv2/opencv.hpp>
#include <QImage>

class ImageUtils{
public:
    static cv::Mat QImageToMat(const QImage& qImage, bool cloneData = true)
    {
        QImage image = qImage;
        if(image.format() != QImage::Format_RGB888){
            image = qImage.convertToFormat(QImage::Format_RGB888);
        }

        cv::Mat mat(image.height(), image.width(), CV_8UC3,
                    const_cast<uchar*>(image.bits()),
                    static_cast<size_t>(image.bytesPerLine()));

        cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR);

        return cloneData ? mat.clone() : mat;
    }

    static QImage MatToQImage(const cv::Mat& mat, bool cloneData = true)
    {
        if (mat.empty()){
            return QImage();
        }

        cv::Mat displayMat;
        cv::cvtColor(mat, displayMat, cv::COLOR_BGR2RGB);

        QImage qImage(displayMat.data,
                      displayMat.cols,
                      displayMat.rows,
                      static_cast<int>(displayMat.step),
                      QImage::Format_RGB888);
                      
        return cloneData ? qImage.copy() : qImage;
    }
};