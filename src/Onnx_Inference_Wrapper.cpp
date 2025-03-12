#include <opencv2/opencv.hpp>
#include <onnxruntime_cxx_api.h>
#include <vector>
#include <string>
#include <stdexcept>
#include <fstream>
#include <iomanip> 
#include <chrono>
#include "Onnx_Inference_Wrapper.hpp"

void ONNXInferenceWrapper::validate_model_path(const std::string& path) 
{
    std::ifstream file(path);
    if (!file.good()) throw std::runtime_error("Model file not found: " + path);
}
    
ONNXInferenceWrapper::ONNXInferenceWrapper(const std::string& model_path, int scale)
    : env_(ORT_LOGGING_LEVEL_WARNING, "ESRGAN"), scale_(scale) 
{
    validate_model_path(model_path);
            
            // 会话配置
    Ort::SessionOptions session_options;
    session_options.SetIntraOpNumThreads(3);
    session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
            
            // 加载模型
    session_ = Ort::Session(env_, model_path.c_str(), session_options);
    
            // 获取输入输出信息
    Ort::AllocatorWithDefaultOptions allocator;
    input_names_.push_back(session_.GetInputNameAllocated(0, allocator).release());
    output_names_.push_back(session_.GetOutputNameAllocated(0, allocator).release());
            
            // 推导预处理参数
    std::string input_name_lower = input_names_[0];
    std::transform(input_name_lower.begin(), input_name_lower.end(),
                 input_name_lower.begin(), ::tolower);
    if (input_name_lower.find("0.5") != std::string::npos)
    {
        mean_ = 0.5f;
        std_ = 0.5f;
    }
    if (input_name_lower.find("rgb") != std::string::npos)
    {
        channel_order_ = "rgb";
    }
    
            // 获取输入形状
    auto type_info = session_.GetInputTypeInfo(0);
    auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
    input_shape_ = tensor_info.GetShape();
}
    
ONNXInferenceWrapper::~ONNXInferenceWrapper()
{
    Ort::AllocatorWithDefaultOptions allocator;
    allocator.Free(const_cast<void*>(reinterpret_cast<const void*>(input_names_[0])));
    allocator.Free(const_cast<void*>(reinterpret_cast<const void*>(output_names_[0])));
}
    
cv::Mat ONNXInferenceWrapper::pre_downScale(const cv::Mat& img, int max_area)
{
    original_size = img.size();
    double area = img.cols * img.rows;
    if (area <= max_area) return img.clone();

    double scale = sqrt(max_area / area);
    cv::resize(img, working_img, cv::Size(), scale, scale, cv::INTER_AREA);
    return working_img;
}

cv::Mat ONNXInferenceWrapper::enhance(const cv::Mat& img)
{
    persistent_buffers_.clear();
    auto start = std::chrono::high_resolution_clock::now();
    auto [input_tensor, orig_size] = preprocess(img);
    
    auto mid1 = std::chrono::high_resolution_clock::now();
    Ort::Value output_tensor = inference(input_tensor);
    
    auto mid2 = std::chrono::high_resolution_clock::now();
    cv::Mat result = postprocess(output_tensor, orig_size);
    
    auto end = std::chrono::high_resolution_clock::now();
    
    time_cost_.preprocess = std::chrono::duration<double, std::milli>(mid1 - start).count();
    time_cost_.inference = std::chrono::duration<double, std::milli>(mid2 - mid1).count();
    time_cost_.postprocess = std::chrono::duration<double, std::milli>(end - mid2).count();
    
    return result;
}

cv::Mat ONNXInferenceWrapper::fin_upScale(const cv::Mat& sr_img)
{
    if (working_img.empty()) return sr_img;
    cv::Mat fin_result;
    cv::resize(sr_img, fin_result, original_size, 0, 0, cv::INTER_LANCZOS4);
    return fin_result;
}
    
void ONNXInferenceWrapper::print_timecost() const
{
    std::cout << "\n=== 阶段耗时分析 ===" << std::endl;
    std::cout << "预处理:" << time_cost_.preprocess << "ms" << std::endl;
    std::cout << "推理:" << time_cost_.inference << "ms" << std::endl;
    std::cout << "后处理:" << time_cost_.postprocess << "ms" << std::endl;
    std::cout << "总计:" << (time_cost_.preprocess + time_cost_.inference + time_cost_.postprocess) << "ms" << std::endl;
}        

std::pair<Ort::Value, cv::Size> ONNXInferenceWrapper::preprocess(const cv::Mat& img)
{
    cv::Mat processed = img.clone();
    cv::Size orig_size(img.cols, img.rows);
    
    std::cout << "\n=== 输入图像调试信息 ===" << std::endl;
    debug_print_color_info(img, "原始输入");
    
            // 动态填充
    int pad_h = (scale_ - (processed.rows % scale_)) % scale_;
    int pad_w = (scale_ - (processed.cols % scale_)) % scale_;
    if (pad_h == scale_) pad_h = 0;
    if (pad_w == scale_) pad_w = 0;
    cv::copyMakeBorder(processed, processed, 0, pad_h, 0, pad_w,
                     cv::BORDER_REFLECT);
    
            // 颜色转换
    if (channel_order_ == "rgb") {
        cv::cvtColor(processed, processed, cv::COLOR_BGR2RGB);
        debug_print_color_info(processed, "转换后的RGB输入");
    } else {
        debug_print_color_info(processed, "原始BGR输入");
    }
    
    
            // 归一化处理
    processed.convertTo(processed, CV_32F);
    if (mean_ == 0.5f){
        processed = (processed / 255.0f - 0.5f) / 0.5f;
    }else{
        processed = processed / 255.0f;
    }
            // 转换为NCHW格式
    std::vector<cv::Mat> channels;
    cv::split(processed, channels);
    const size_t total_pixels = processed.rows * processed.cols;
    std::vector<float> blob_data(3 * total_pixels); // 使用vector管理内存

    // 拷贝数据到连续内存
    for (int c = 0; c < 3; ++c) {
        cv::Mat channel = channels[c].reshape(1, 1);
        std::memcpy(blob_data.data() + c * total_pixels,
                    channel.ptr<float>(),
                    total_pixels * sizeof(float));
    }

    // 创建输入张量（深拷贝数据）
    std::vector<int64_t> input_dims = {1, 3, processed.rows, processed.cols};
    Ort::Value tensor = Ort::Value::CreateTensor<float>(
        memory_info_,
        blob_data.data(),
        blob_data.size(),
        input_dims.data(),
        input_dims.size()
    );

    return {std::move(tensor), orig_size};
}
    
Ort::Value ONNXInferenceWrapper::inference(Ort::Value& input_tensor)
{
    Ort::RunOptions run_options;
        
            // 修正后的输出缓冲区初始化
    std::vector<Ort::Value> output_buffer;
    output_buffer.reserve(output_names_.size()); // 预分配内存但不会构造对象
        
            // 显式构造每个元素（使用nullptr初始化）
    for (size_t i = 0; i < output_names_.size(); ++i) {
        output_buffer.emplace_back(nullptr); // ✅ 调用explicit Value(nullptr_t)构造函数
    }
        
            // API调用（保持类型严格匹配）
    session_.Run(
        run_options,
        input_names_.data(),
        &input_tensor,
        1,
        output_names_.data(),
        output_buffer.data(), // Ort::Value* 类型
        output_buffer.size()
    );
        
    return std::move(output_buffer[0]);
}
    
cv::Mat ONNXInferenceWrapper::postprocess(Ort::Value& output_tensor, const cv::Size& orig_size)
{
    float* output_data = output_tensor.GetTensorMutableData<float>();
    auto shape = output_tensor.GetTensorTypeAndShapeInfo().GetShape();
    
            // 转换为OpenCV格式（NCHW -> HWC）
    cv::Mat output(shape[2], shape[3], CV_32FC3);
    const int channel_size = shape[2] * shape[3];
    
    std::vector<cv::Mat> output_channels(3);
    for (int c = 0; c < 3; ++c) {
        output_channels[c] = cv::Mat(shape[2], shape[3], CV_32FC1, 
                                    output_data + c * channel_size);
    }
    
    cv::merge(output_channels, output);
    
            // 反归一化
    if (mean_ == 0.5f) {
        output = (output * 0.5f + 0.5f) * 255.0f;
    } else {
        output = output * 255.0f;
    }
    output.convertTo(output, CV_8UC3);
    
   std::cout << "\n=== 输出图像调试信息 ===" << std::endl;
    debug_print_color_info(output, "反归一化后的图像");
            
    if (channel_order_ == "rgb") {
        cv::cvtColor(output, output, cv::COLOR_RGB2BGR);
        debug_print_color_info(output, "转换回BGR后的图像");
    }
    
            // 裁剪填充区域
    cv::Rect roi(0, 0, orig_size.width * scale_, orig_size.height * scale_);
    cv::Mat cropped = output(roi).clone();
    
    debug_print_color_info(cropped, "最终输出图像");
    
    return cropped;
}
    
void ONNXInferenceWrapper::debug_print_color_info(const cv::Mat& img, const std::string& stage) {
    std::cout << "调试阶段: " << stage << std::endl;
    std::cout << "图像尺寸: " << img.cols << "x" << img.rows 
              << " 通道数: " << img.channels() << std::endl;
    
            // 输出中心区域5x5像素的RGB值
    const int sample_size = 5;
    const int x_start = std::max(0, img.cols/2 - sample_size/2);
    const int y_start = std::max(0, img.rows/2 - sample_size/2);
            
    std::cout << "中心区域(" << x_start << "," << y_start << ")附近的颜色采样:\n";
            
    for (int y = y_start; y < std::min(y_start+sample_size, img.rows); ++y) {
        for (int x = x_start; x < std::min(x_start+sample_size, img.cols); ++x) {
            if (img.channels() == 3) {
                cv::Vec3b pixel = img.at<cv::Vec3b>(y, x);
                std::cout << std::setw(3) << static_cast<int>(pixel[2]) << "R " 
                          << std::setw(3) << static_cast<int>(pixel[1]) << "G "
                          << std::setw(3) << static_cast<int>(pixel[0]) << "B | ";
            } else if (img.channels() == 1) {
                std::cout << std::setw(3) << static_cast<int>(img.at<uchar>(y, x)) << " | ";
            }
        }
        std::cout << "\n";
    }
    
            // 输出各通道统计信息
    if (img.channels() == 3) {
        cv::Scalar mean = cv::mean(img);
        cv::Scalar stddev;
        cv::meanStdDev(img, mean, stddev);
                
        std::cout << "通道均值(BGR): "
                  << std::fixed << std::setprecision(1)
                  << mean[0] << ", " << mean[1] << ", " << mean[2] << "\n"
                  << "通道标准差(BGR): "
                  << stddev[0] << ", " << stddev[1] << ", " << stddev[2] 
                  << std::endl;
    }    
}

    