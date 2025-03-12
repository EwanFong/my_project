demo👇
https://github.com/user-attachments/assets/0eceea4b-a26b-4ba1-94cb-b0c4b2a40886

基于Qt和OpenCV的图像修复工具，集成Real-ESRGAN超分辨率模型

## 主要功能 ✨
- 🖌️ 智能图像修复（Inpainting）
- 🔍 4倍超分辨率重建
- 🎨 实时预览修复效果
- 📚 操作历史记录（Undo/Redo）
- 🖼️ 多格式支持：PNG/JPG

使用指南 📖
1.文件操作
Ctrl+S 保存结果
Ctrl+Z 撤销操作
2.工具
点击Draw按钮启用1画笔，2矩形，3圆形，4三角形，5打马赛克（在点击Draw按钮后按键盘1-5快捷键启用不同功能，1-4功能可按QE调整边框粗细）
点击Denoise按钮启用去噪模式，1-4分别四种去噪方式（同上快捷键启用功能，按Q减少kernel数量，按E增加kernel数量）
点击Inpaint按钮开始修复（超分辨率图像修复）

其中onnx模型根据以下
官方https://github.com/xinntao/Real-ESRGAN
预训练权重RealESRGAN_x4plus.pth

致谢 🙏
Real-ESRGAN团队提供基础模型
OpenCV和Qt社区的技术支持
ONNX Runtime的推理优化


