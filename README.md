# 简易有限元分析(FEA)结果查看器

基于C++、Qt6、VTK开发的有限元分析结果可视化工具。

## 功能特性

- **文件加载**: 支持VTK格式文件(.vtu, .vtk)
- **三维交互**: 鼠标旋转、平移、缩放模型
- **云图显示**: 根据标量数据生成彩色云图
- **数据切换**: 支持多种数据类型的切换显示
- **图例显示**: 显示颜色与数值的对应关系

## 环境要求

- Windows 10/11
- Visual Studio 2022
- Qt 6.9.1 (路径: H:\Programs\Qt\6.9.1\msvc2022_64)
- VTK 9.5.0 (路径: H:\SourceCode\vtk-9.5.0\install)
- Eigen 3.4.0 (路径: H:\SourceCode\eigen-3.4.0\install)

## 构建和部署方法

### 🚀 方法一：一键构建（推荐）
```bash
python build.py
```
自动构建项目并复制所有VTK和Qt DLL文件，彻底解决依赖问题

### 方法二：快速启动（如果已构建）
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

1. 启动程序
2. 点击"打开文件"按钮选择VTK文件(.vtu或.vtk格式)
3. 文件加载后，从下拉菜单中选择要显示的数据类型
4. 使用鼠标交互操作:
   - 左键拖拽: 旋转模型
   - 中键拖拽: 平移模型
   - 滚轮: 缩放模型
