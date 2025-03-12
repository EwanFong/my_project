#pragma once

#include <opencv2/opencv.hpp>
#include <onnxruntime_cxx_api.h>

class ONNXInferenceWrapper {
    private:
        struct TimeCost
        {
            double preprocess = 0;
            double inference = 0;
            double postprocess = 0;
        } time_cost_;
        
    
        Ort::Env env_;
        Ort::Session session_{nullptr};
        Ort::MemoryInfo memory_info_ = Ort::MemoryInfo::CreateCpu(
            OrtAllocatorType::OrtArenaAllocator, 
            OrtMemType::OrtMemTypeDefault
        );
        
        std::vector<const char*> input_names_;
        std::vector<const char*> output_names_;
        std::vector<int64_t> input_shape_;
        std::vector<std::vector<float>> persistent_buffers_;
        std::string channel_order_ = "bgr";

        cv::Size original_size;
        cv::Mat working_img;

        // 预处理参数
        float mean_ = 0.0f;
        float std_ = 255.0f;
        int scale_ = 4;
        int dynamic_pad_ = 0;
    
        void validate_model_path(const std::string& path);
    
    public:
        ONNXInferenceWrapper(const std::string& model_path, int scale = 4);

        ~ONNXInferenceWrapper();
    
        cv::Mat enhance(const cv::Mat& img);
        cv::Mat pre_downScale(const cv::Mat& img, int max_area = 720 * 720);
        cv::Mat fin_upScale(const cv::Mat& img);
    
        void print_timecost() const;

    private:
        std::pair<Ort::Value, cv::Size> preprocess(const cv::Mat& img);
        Ort::Value inference(Ort::Value& input_tensor);
        cv::Mat postprocess(Ort::Value& output_tensor, const cv::Size& orig_size);
        void debug_print_color_info(const cv::Mat& img, const std::string& stage);
    };
    
