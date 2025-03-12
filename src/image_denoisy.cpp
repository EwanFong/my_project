#include "image_denoisy.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

ImageProcessor_denoisy::ImageProcessor_denoisy(const cv::Mat image)
{
    m_image = image;
}

cv::Mat ImageProcessor_denoisy::denoiseImage(denoisyMode method, int kernelSize)
{
    cv::Mat result;

    switch (method)
    {
    case DENOISY_BLUR:
        cv::blur(m_image, result, cv::Size(kernelSize, kernelSize));
        break;
    
    case DENOISY_MEDIANBLUR:
        cv::medianBlur(m_image, result, kernelSize);
        break;

    case DENOISY_GAUSSIANBLUR:
        cv::GaussianBlur(m_image, result, cv::Size(kernelSize, kernelSize), 0);
        break;

    case DENOISY_BILATERALFILTER:
        cv::bilateralFilter(m_image, result, kernelSize, 75, 75);
        break;
    
    default:
        result = m_image;
        break;
    }
    return result;
}


