# Image Restoration Toolkit 🔧🖼️

+++[![CI Status](https://github.com/Ewanfong/image-restoration-toolkit/actions/workflows/build.yml/badge.svg)](https://github.com/yourusername/image-restoration-toolkit/actions)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![OpenCV Version](https://img.shields.io/badge/OpenCV-4.11%2B-blue)](https://opencv.org)
[![Qt Version](https://img.shields.io/badge/Qt-6.53%2B-green)](https://www.qt.io)+++

基于Qt和OpenCV的图像修复工具，集成Real-ESRGAN超分辨率模型

## 主要功能 ✨
- 🖌️ 智能图像修复（Inpainting）
- 🔍 4倍超分辨率重建
- 🎨 实时预览修复效果
- 📚 操作历史记录（Undo/Redo）
- 🖼️ 多格式支持：PNG/JPG

## 技术栈 🛠️
- **核心框架**: Qt 6.53+ 
- **图像处理**: OpenCV 4.11+
- **模型推理**: ONNX Runtime 1.51+
- **模型架构**: Real-ESRGAN (Apache-2.0 License)

## 使用指南 📖
## 版本验证 ✅
本项目已通过以下环境测试：
| 组件       | 版本          |
|------------|---------------|
| OpenCV     | 4.11         |
| Qt         | 6.53        |
| C++标准    | C++17         |
| ONNX Runtime| 1.15.1        |

### 快速开始
# 克隆仓库
git clone https://github.com/EwanFong/my_project.git
cd my_project

./my_project
1. **文件操作**
   - `Ctrl+S` 保存结果
   - `Ctrl+Z` 撤销操作

2. **工具使用** +++（表格更清晰）+++
| 按钮       | 快捷键 | 功能说明                     | 参数调整          |
|------------|--------|------------------------------|-------------------|
| **Draw**   | 1-5    | 选择画笔/形状工具            | Q/E调整边框粗细   |
| **Denoise**| 1-4    | 选择去噪算法                 | Q/E调整kernel数量 |
| **Inpaint**| -      | 执行修复+超分辨率            | -                 |

+++## 模型合规性说明 ⚖️
本项目严格遵循开源协议要求：
1. **模型转换**：使用官方提供的RealESRGAN_x4plus.pth权重转换为ONNX格式
2. **代码原创性**：推理逻辑完全使用C++重新实现
3. **协议继承**：
   - 项目代码采用[MIT License](LICENSE)
   - 模型部分继承[Apache-2.0 License](thirdparty/RealESRGAN_LICENSE)
