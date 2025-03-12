#pragma once

#include <opencv2/opencv.hpp>
#include <QString>


class ImageProcessor_denoisy{
public:
    ImageProcessor_denoisy(const cv::Mat image);

    enum denoisyMode {
        DENOISY_NONE = 0,
        DENOISY_BLUR = 1,
        DENOISY_MEDIANBLUR = 2,
        DENOISY_GAUSSIANBLUR = 3,
        DENOISY_BILATERALFILTER = 4
    };

    static QString methodToString(denoisyMode method)
    {
        switch (method) {
        case DENOISY_NONE:
            return "None";
        case DENOISY_BLUR:
            return "Mean Blur";
        case DENOISY_MEDIANBLUR:
            return "Median Blur";
        case DENOISY_GAUSSIANBLUR:
            return "Gaussian Blur";
        case DENOISY_BILATERALFILTER:
            return "Bilateralfilter Blur";
        default:
            break;
        }
    }


    cv::Mat denoiseImage(denoisyMode method, int kernelSize);
private:
    cv::Mat m_image;
    denoisyMode currentmode_ = DENOISY_NONE;
};
