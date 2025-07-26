# 简易有限元分析结果查看器

基于C++、Qt6、VTK开发的有限元分析结果可视化工具。

## 功能特性

### 基础功能
- **文件加载**: 支持VTK格式文件(.vtu, .vtk)
- **三维交互**: 鼠标旋转、平移、缩放模型
- **云图显示**: 根据标量数据生成彩色云图
- **数据切换**: 支持多种数据类型的切换显示

### 新增高级功能 ✨
- **网格显示切换**: 实体显示、网格显示、实体+网格组合显示
- **颜色映射表**: 6种颜色方案（彩虹、热力图、蓝白红、灰度、绿蓝、紫红）
- **透明度控制**: 滑块调节模型透明度（10%-100%）
- **方向坐标轴**: 左下角显示XYZ坐标轴，便于定向
- **横向标量条**: 右下角横向显示，字体更大更清晰

### 专业级高级功能 🔥
- **剖切/切片**: 使用虚拟平面切割模型，观察内部结构
- **数据拾取**: 鼠标点击获取精确坐标和数值信息
- **等值面/等值线**: 提取特定数值的等值面，支持手动和自动生成

## 环境要求

- Windows 10/11
- Visual Studio 2022
- Qt 6.9.1 (路径: H:\Programs\Qt\6.9.1\msvc2022_64)
- VTK 9.5.0 (路径: H:\SourceCode\vtk-9.5.0\install)
- Eigen 3.4.0 (路径: H:\SourceCode\eigen-3.4.0\install)

## 构建和部署方法

### 🚀 方法一：一键构建（推荐）
```bash
python ultimate_build.py
```
自动构建项目并复制所有VTK和Qt DLL文件，彻底解决依赖问题

### 方法二：使用主控制脚本
```bash
python main.py
```
然后选择"🚀 构建和部署项目"

### 方法三：快速启动（如果已构建）
```bash
python run.py
```

### 方法四：分步骤构建
1. 检查依赖：`python check_dependencies.py`
2. 构建项目：`python build.py`
3. 部署依赖：`python deploy.py`

### 方法五：手动构建
1. 确保所有依赖库已正确安装在指定路径
2. 创建构建目录：`mkdir build && cd build`
3. 配置CMake：`cmake .. -G "Visual Studio 17 2022" -A x64`
4. 构建项目：`cmake --build . --config Release`
5. CMake会自动复制依赖库到可执行文件目录

## 使用方法

### 基本操作
1. 启动程序
2. 点击"打开文件"按钮选择VTK文件(.vtu或.vtk格式)
3. 文件加载后，从下拉菜单中选择要显示的数据类型

### 高级功能使用
4. **显示模式控制**:
   - 勾选"显示网格"可同时显示实体和网格
   - 网格以黑色线框显示，便于观察网格质量

5. **颜色映射切换**:
   - 从"颜色映射"下拉菜单选择不同的颜色方案
   - 彩虹色适合一般分析，热力图适合温度场，灰度适合打印

6. **透明度调节**:
   - 使用透明度滑块调节模型透明度
   - 适合观察内部结构或重叠模型

7. **鼠标交互操作**:
   - 左键拖拽: 旋转模型
   - 中键拖拽: 平移模型
   - 滚轮: 缩放模型
   - 左下角坐标轴帮助定向

### 专业级功能使用
8. **剖切功能**:
   - 右侧"剖切控制"面板
   - 勾选"启用剖切"开始使用
   - 滑块控制剖切平面位置和方向
   - 实时观察模型内部结构

9. **等值面功能**:
   - 右侧"等值面控制"面板
   - 手动输入特定数值或自动生成
   - 支持多个等值面同时显示
   - 黄色线条显示等值面

10. **数据拾取功能**:
    - 菜单"工具" -> "启用数据拾取"
    - 鼠标点击模型获取精确数值
    - 状态栏显示坐标和数据信息

## 项目结构

```
FEMResultViewer/
├── src/
│   ├── main.cpp                      # 程序入口
│   ├── MainWindow.h                  # 主窗口头文件
│   ├── MainWindow.cpp                # 主窗口实现
│   ├── visualization/               # 可视化功能模块
│   │   ├── ClippingWidget.h         # 剖切控制组件
│   │   ├── ClippingWidget.cpp
│   │   ├── ContourWidget.h          # 等值面控制组件
│   │   └── ContourWidget.cpp
│   └── interaction/                 # 交互功能模块
│       ├── DataPicker.h             # 数据拾取组件
│       └── DataPicker.cpp
├── CMakeLists.txt                   # CMake配置文件
├── copy_vtk_dlls.cmake              # VTK DLL自动复制脚本
├── main.py                          # 主控制脚本
├── ultimate_build.py                # 终极构建脚本
├── test_advanced_features.py        # 高级功能测试脚本
├── check_dependencies.py            # 依赖检查脚本
├── create_test_data.py              # 测试数据生成脚本
└── README.md                        # 说明文档
```

## Python脚本说明

- **main.py**: 主控制面板，提供菜单式操作界面
- **ultimate_build.py**: 终极构建脚本，自动构建并部署所有依赖
- **check_dependencies.py**: 检查开发环境和依赖库
- **test_cmake.py**: 测试CMake配置是否正确
- **create_test_data.py**: 生成测试用的VTK文件
- **run.py**: 快速启动已构建的程序

## 技术特点

- 使用Qt6构建现代化用户界面
- 集成VTK实现高性能三维可视化
- 支持点数据和单元数据的云图显示
- 自动生成颜色映射和图例
- 流畅的三维交互体验
- **CMake自动依赖管理** - 构建时自动复制所需DLL文件

## CMake自动依赖处理

新版本的CMakeLists.txt包含以下自动化功能：

1. **自动检测和复制Qt DLL文件**
2. **自动检测和复制VTK DLL文件**
3. **自动复制Qt平台插件**
4. **使用windeployqt自动部署Qt依赖**
5. **构建完成后立即可运行，无需手动部署**

支持的DLL文件包括：
- Qt核心库：Qt6Core, Qt6Gui, Qt6Widgets, Qt6OpenGL, Qt6OpenGLWidgets
- VTK核心库：所有必需的VTK 9.5模块
- VTK第三方库：expat, lz4, lzma, zlib, glew, freetype
- Qt平台插件：qwindows.dll
## 故障排
除

### 常见问题

**问题1：找不到DLL文件**
- 错误信息：`由于找不到 vtkRenderingOpenGL2-9.5.dll，无法继续执行代码`
- 解决方案：运行 `deploy_simple.bat` 或 `build_and_deploy.bat`

**问题2：找不到Qt DLL文件**
- 错误信息：`由于找不到 Qt6OpenGLWidgets.dll，无法继续执行代码`
- 解决方案：确保Qt路径正确，运行部署脚本

**问题3：CMake配置失败**
- 解决方案：
  1. 检查依赖路径是否正确：`check_dependencies.bat`
  2. 确保Visual Studio 2022已安装
  3. 确保CMake版本 >= 3.16

**问题4：VTK编译错误**
- 解决方案：确保VTK是用相同的编译器和配置编译的
- 建议使用Release配置编译VTK

### 手动部署DLL文件

如果自动部署脚本失败，可以手动复制以下文件到 `build\Release\` 目录：

**Qt DLL文件**（从 `H:\Programs\Qt\6.9.1\msvc2022_64\bin\`）：
- Qt6Core.dll
- Qt6Gui.dll  
- Qt6Widgets.dll
- Qt6OpenGL.dll
- Qt6OpenGLWidgets.dll

**VTK DLL文件**（从 `H:\SourceCode\vtk-9.5.0\install\bin\`）：
- vtkCommonCore-9.5.dll
- vtkCommonDataModel-9.5.dll
- vtkRenderingCore-9.5.dll
- vtkRenderingOpenGL2-9.5.dll
- vtkGUISupportQt-9.5.dll
- 以及其他相关的VTK库文件

**Qt平台插件**：
- 创建 `platforms` 文件夹
- 复制 `qwindows.dll` 到 `platforms\` 文件夹