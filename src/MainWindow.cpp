#include "MainWindow.h"
#include <QFileInfo>
#include <QMouseEvent>
#include <QMenuBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_openFileButton(nullptr)
    , m_dataComboBox(nullptr)
    , m_statusLabel(nullptr)
    , m_vtkWidget(nullptr)
    , m_currentData(nullptr)
    , m_currentGeometryData(nullptr)
    , m_currentDataType(DATA_TYPE_NONE)
    , m_clippingWidget(nullptr)
    , m_contourWidget(nullptr)
    , m_vectorFieldWidget(nullptr)
    , m_dataPicker(nullptr)
    , m_pickingAction(nullptr)
{
    setupUI();
    setupVTK();
    setupDockWidgets();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    setWindowTitle("简易有限元分析结果查看器");
    setMinimumSize(800, 600);

    // 创建中央窗口部件
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // 创建工具栏布局
    QGridLayout *toolbarLayout = new QGridLayout();
    
    // 第一行：文件操作
    m_openFileButton = new QPushButton("打开文件", this);
    m_openFileButton->setMaximumWidth(100);
    connect(m_openFileButton, &QPushButton::clicked, this, &MainWindow::openFile);
    
    QLabel *dataLabel = new QLabel("数据类型:", this);
    m_dataComboBox = new QComboBox(this);
    m_dataComboBox->setMinimumWidth(250);
    m_dataComboBox->setMaximumWidth(350);
    m_dataComboBox->setEnabled(false);
    connect(m_dataComboBox, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            this, &MainWindow::onDataSelectionChanged);
    
    m_statusLabel = new QLabel("请选择一个VTK文件", this);
    
    toolbarLayout->addWidget(m_openFileButton, 0, 0);
    toolbarLayout->addWidget(dataLabel, 0, 1);
    toolbarLayout->addWidget(m_dataComboBox, 0, 2);
    toolbarLayout->addWidget(m_statusLabel, 0, 3, 1, 2);
    
    // 第二行：显示控制
    // 网格显示控制
    m_wireframeCheckBox = new QCheckBox("显示网格", this);
    m_wireframeCheckBox->setEnabled(false);
    connect(m_wireframeCheckBox, &QCheckBox::stateChanged, this, &MainWindow::onDisplayModeChanged);
    
    // 颜色映射选择
    QLabel *colorMapLabel = new QLabel("颜色映射:", this);
    m_colorMapComboBox = new QComboBox(this);
    m_colorMapComboBox->setEnabled(false);
    connect(m_colorMapComboBox, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            this, &MainWindow::onColorMapChanged);
    
    // 透明度控制
    QLabel *opacityLabel = new QLabel("透明度:", this);
    m_opacitySlider = new QSlider(Qt::Horizontal, this);
    m_opacitySlider->setRange(10, 100);
    m_opacitySlider->setValue(100);
    m_opacitySlider->setMaximumWidth(150);
    m_opacitySlider->setEnabled(false);
    connect(m_opacitySlider, &QSlider::valueChanged, this, &MainWindow::onOpacityChanged);
    
    m_opacityLabel = new QLabel("100%", this);
    m_opacityLabel->setMinimumWidth(40);
    
    toolbarLayout->addWidget(m_wireframeCheckBox, 1, 0);
    toolbarLayout->addWidget(colorMapLabel, 1, 1);
    toolbarLayout->addWidget(m_colorMapComboBox, 1, 2);
    toolbarLayout->addWidget(opacityLabel, 1, 3);
    toolbarLayout->addWidget(m_opacitySlider, 1, 4);
    toolbarLayout->addWidget(m_opacityLabel, 1, 5);

    // VTK渲染窗口
    m_vtkWidget = new QVTKOpenGLNativeWidget(this);
    
    // 添加到主布局
    QWidget *toolbarWidget = new QWidget(this);
    toolbarWidget->setLayout(toolbarLayout);
    
    mainLayout->addWidget(toolbarWidget);
    mainLayout->addWidget(m_vtkWidget, 1);
    
    // 创建状态栏
    statusBar()->showMessage("就绪");
}

void MainWindow::setupVTK()
{
    // 创建渲染窗口
    m_renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    m_vtkWidget->setRenderWindow(m_renderWindow);

    // 创建渲染器
    m_renderer = vtkSmartPointer<vtkRenderer>::New();
    m_renderer->SetBackground(0.1, 0.1, 0.1); // 深灰色背景
    m_renderWindow->AddRenderer(m_renderer);

    // 创建数据映射器
    m_mapper = vtkSmartPointer<vtkDataSetMapper>::New();

    // 创建主演员（实体显示）
    m_actor = vtkSmartPointer<vtkActor>::New();
    m_actor->SetMapper(m_mapper);
    
    // 创建网格映射器和演员
    m_wireframeMapper = vtkSmartPointer<vtkDataSetMapper>::New();
    m_wireframeActor = vtkSmartPointer<vtkActor>::New();
    m_wireframeActor->SetMapper(m_wireframeMapper);
    m_wireframeActor->GetProperty()->SetRepresentationToWireframe();
    m_wireframeActor->GetProperty()->SetColor(0.0, 0.0, 0.0); // 黑色网格
    m_wireframeActor->GetProperty()->SetLineWidth(1.0);

    // 创建颜色查找表
    m_lookupTable = vtkSmartPointer<vtkLookupTable>::New();
    m_lookupTable->SetHueRange(0.667, 0.0); // 蓝色到红色
    m_lookupTable->SetSaturationRange(1.0, 1.0);
    m_lookupTable->SetValueRange(1.0, 1.0);
    m_lookupTable->SetAlphaRange(1.0, 1.0);
    m_lookupTable->SetNumberOfColors(256);
    m_lookupTable->Build();

    // 创建标量条
    m_scalarBar = vtkSmartPointer<vtkScalarBarActor>::New();
    m_scalarBar->SetLookupTable(m_lookupTable);
    m_scalarBar->SetTitle("数值");
    m_scalarBar->SetNumberOfLabels(5);
    
    // 设置为横向显示并放在右下角
    m_scalarBar->SetOrientationToHorizontal();
    m_scalarBar->SetWidth(0.4);   // 横向宽度
    m_scalarBar->SetHeight(0.08); // 横向高度
    m_scalarBar->SetPosition(0.55, 0.05); // 右下角位置
    
    // 设置标量条文本属性 - 增大字体
    vtkTextProperty *titleProp = m_scalarBar->GetTitleTextProperty();
    titleProp->SetFontSize(16);  // 增大标题字体
    titleProp->SetColor(1.0, 1.0, 1.0);
    titleProp->SetBold(1);       // 加粗标题
    
    vtkTextProperty *labelProp = m_scalarBar->GetLabelTextProperty();
    labelProp->SetFontSize(14);  // 增大标签字体
    labelProp->SetColor(1.0, 1.0, 1.0);
    labelProp->SetBold(1);       // 加粗标签

    // 设置交互器样式
    vtkSmartPointer<vtkInteractorStyleTrackballCamera> style = 
        vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    m_renderWindow->GetInteractor()->SetInteractorStyle(style);
    
    // 创建坐标轴
    m_axesActor = vtkSmartPointer<vtkAxesActor>::New();
    m_axesActor->SetShaftTypeToCylinder();
    m_axesActor->SetXAxisLabelText("X");
    m_axesActor->SetYAxisLabelText("Y");
    m_axesActor->SetZAxisLabelText("Z");
    m_axesActor->SetTotalLength(1.5, 1.5, 1.5);
    
    // 创建方向标记小部件
    m_orientationWidget = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
    m_orientationWidget->SetOrientationMarker(m_axesActor);
    m_orientationWidget->SetInteractor(m_renderWindow->GetInteractor());
    m_orientationWidget->SetViewport(0.0, 0.0, 0.2, 0.2); // 左下角
    m_orientationWidget->SetEnabled(1);
    m_orientationWidget->InteractiveOff();
    
    // 设置颜色映射
    setupColorMaps();
}

void MainWindow::setupDockWidgets()
{
    // 创建剖切控制停靠窗口
    m_clippingWidget = new ClippingWidget(this);
    m_clippingDock = new QDockWidget("剖切控制", this);
    m_clippingDock->setWidget(m_clippingWidget);
    m_clippingDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, m_clippingDock);
    
    // 创建等值面控制停靠窗口
    m_contourWidget = new ContourWidget(this);
    m_contourDock = new QDockWidget("等值面控制", this);
    m_contourDock->setWidget(m_contourWidget);
    m_contourDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, m_contourDock);
    
    // 创建矢量场控制停靠窗口
    m_vectorFieldWidget = new VectorFieldWidget(this);
    m_vectorFieldDock = new QDockWidget("矢量场可视化", this);
    m_vectorFieldDock->setWidget(m_vectorFieldWidget);
    m_vectorFieldDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, m_vectorFieldDock);
    
    // 将三个停靠窗口标签化
    tabifyDockWidget(m_clippingDock, m_contourDock);
    tabifyDockWidget(m_contourDock, m_vectorFieldDock);
    m_clippingDock->raise(); // 默认显示剖切控制
    
    // 创建数据拾取器
    m_dataPicker = new DataPicker(this);
    connect(m_dataPicker, &DataPicker::pointPicked,
            this, &MainWindow::onPointPicked);
    
    // 连接信号
    connect(m_clippingWidget, &ClippingWidget::clippingChanged,
            this, &MainWindow::onClippingChanged);
    connect(m_contourWidget, &ContourWidget::contoursChanged,
            this, &MainWindow::onContoursChanged);
    connect(m_vectorFieldWidget, &VectorFieldWidget::vectorVisualizationChanged,
            this, &MainWindow::onVectorVisualizationChanged);
    
    // 创建菜单栏
    QMenuBar *menuBar = this->menuBar();
    QMenu *viewMenu = menuBar->addMenu("视图");
    viewMenu->addAction(m_clippingDock->toggleViewAction());
    viewMenu->addAction(m_contourDock->toggleViewAction());
    viewMenu->addAction(m_vectorFieldDock->toggleViewAction());
    
    QMenu *toolsMenu = menuBar->addMenu("工具");
    m_pickingAction = toolsMenu->addAction("启用数据拾取");
    m_pickingAction->setCheckable(true);
    connect(m_pickingAction, &QAction::toggled, m_dataPicker, &DataPicker::enablePicking);
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "打开文件",
        "",
        "所有支持的文件 (*.vtu *.vtk *.stl *.obj *.ply);;VTK分析文件 (*.vtu *.vtk);;3D模型文件 (*.stl *.obj *.ply);;STL文件 (*.stl);;OBJ文件 (*.obj);;PLY文件 (*.ply);;所有文件 (*.*)"
    );

    if (fileName.isEmpty()) {
        return;
    }

    m_currentFileName = fileName;
    
    // 清除之前的数据
    m_currentData = nullptr;
    m_currentGeometryData = nullptr;
    m_currentDataType = DATA_TYPE_NONE;
    
    // 根据文件扩展名选择合适的读取器
    if (fileName.endsWith(".vtu", Qt::CaseInsensitive)) {
        // VTK XML格式 - 分析数据
        m_xmlReader = vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
        m_xmlReader->SetFileName(fileName.toStdString().c_str());
        m_xmlReader->Update();
        
        if (m_xmlReader->GetOutput()->GetNumberOfCells() == 0) {
            QMessageBox::warning(this, "错误", "无法读取VTU文件或文件为空");
            return;
        }
        
        m_currentData = m_xmlReader->GetOutput();
        m_currentDataType = DATA_TYPE_UNSTRUCTURED_GRID;
    }
    else if (fileName.endsWith(".vtk", Qt::CaseInsensitive)) {
        // VTK Legacy格式 - 分析数据
        m_legacyReader = vtkSmartPointer<vtkUnstructuredGridReader>::New();
        m_legacyReader->SetFileName(fileName.toStdString().c_str());
        m_legacyReader->Update();
        
        if (m_legacyReader->GetOutput()->GetNumberOfCells() == 0) {
            QMessageBox::warning(this, "错误", "无法读取VTK文件或文件为空");
            return;
        }
        
        m_currentData = m_legacyReader->GetOutput();
        m_currentDataType = DATA_TYPE_UNSTRUCTURED_GRID;
    }
    else if (fileName.endsWith(".stl", Qt::CaseInsensitive)) {
        // STL格式 - 纯几何数据
        m_stlReader = vtkSmartPointer<vtkSTLReader>::New();
        m_stlReader->SetFileName(fileName.toStdString().c_str());
        m_stlReader->Update();
        
        if (m_stlReader->GetOutput()->GetNumberOfCells() == 0) {
            QMessageBox::warning(this, "错误", "无法读取STL文件或文件为空");
            return;
        }
        
        m_currentGeometryData = m_stlReader->GetOutput();
        m_currentDataType = DATA_TYPE_GEOMETRY_ONLY;
    }
    else if (fileName.endsWith(".obj", Qt::CaseInsensitive)) {
        // OBJ格式 - 纯几何数据
        try {
            m_objReader = vtkSmartPointer<vtkOBJReader>::New();
            m_objReader->SetFileName(fileName.toStdString().c_str());
            m_objReader->Update();
            
            if (m_objReader->GetOutput()->GetNumberOfCells() == 0) {
                QMessageBox::warning(this, "错误", "无法读取OBJ文件或文件为空");
                return;
            }
            
            m_currentGeometryData = m_objReader->GetOutput();
            m_currentDataType = DATA_TYPE_GEOMETRY_ONLY;
        } catch (...) {
            QMessageBox::warning(this, "错误", "OBJ读取器不可用，请检查VTK安装");
            return;
        }
    }
    else if (fileName.endsWith(".ply", Qt::CaseInsensitive)) {
        // PLY格式 - 纯几何数据
        try {
            m_plyReader = vtkSmartPointer<vtkPLYReader>::New();
            m_plyReader->SetFileName(fileName.toStdString().c_str());
            m_plyReader->Update();
            
            if (m_plyReader->GetOutput()->GetNumberOfCells() == 0) {
                QMessageBox::warning(this, "错误", "无法读取PLY文件或文件为空");
                return;
            }
            
            m_currentGeometryData = m_plyReader->GetOutput();
            m_currentDataType = DATA_TYPE_GEOMETRY_ONLY;
        } catch (...) {
            QMessageBox::warning(this, "错误", "PLY读取器不可用，请检查VTK安装");
            return;
        }
    }
    else {
        QMessageBox::warning(this, "错误", "不支持的文件格式\\n支持的格式：VTU, VTK, STL, OBJ, PLY");
        return;
    }

    if (m_currentDataType == DATA_TYPE_GEOMETRY_ONLY) {
        // 几何文件处理（STL、OBJ、PLY等）
        setupGeometryVisualization();
        
        // 几何文件没有标量数据，禁用相关控件
        m_dataComboBox->setEnabled(false);
        m_dataComboBox->clear();
        QString fileExt = QFileInfo(fileName).suffix().toUpper();
        m_dataComboBox->addItem(QString("无数据 (%1几何)").arg(fileExt));
        
        // 启用基本控件
        m_wireframeCheckBox->setEnabled(true);
        m_opacitySlider->setEnabled(true);
        
        // 禁用颜色映射（几何文件没有标量数据）
        m_colorMapComboBox->setEnabled(false);
        
        // 更新状态
        m_statusLabel->setText(QString("已加载%1: %2").arg(fileExt).arg(QFileInfo(fileName).fileName()));
    } else {
        // VTK文件处理
        // 填充数据下拉框
        populateDataComboBox();
        
        // 启用控件
        m_wireframeCheckBox->setEnabled(true);
        m_colorMapComboBox->setEnabled(true);
        m_opacitySlider->setEnabled(true);
        
        // 更新状态
        m_statusLabel->setText(QString("已加载: %1").arg(QFileInfo(fileName).fileName()));
        
        // 如果有数据数组，自动选择第一个
        if (m_dataComboBox->count() > 0) {
            m_dataComboBox->setCurrentIndex(0);
            onDataSelectionChanged(m_dataComboBox->currentText());
        }
    }
}

void MainWindow::populateDataComboBox()
{
    m_dataComboBox->clear();
    
    if (!m_currentData) {
        m_dataComboBox->setEnabled(false);
        return;
    }

    // 获取点数据数组
    vtkPointData *pointData = m_currentData->GetPointData();
    for (int i = 0; i < pointData->GetNumberOfArrays(); ++i) {
        vtkDataArray *array = pointData->GetArray(i);
        if (array && array->GetName()) {
            QString arrayName = QString::fromStdString(array->GetName());
            int components = array->GetNumberOfComponents();
            
            if (components == 1) {
                // 标量数据
                m_dataComboBox->addItem(QString("点数据: %1 (标量)").arg(arrayName), 
                                       QVariant::fromValue(QPair<QString, bool>(arrayName, true)));
            } else if (components == 3) {
                // 矢量数据
                m_dataComboBox->addItem(QString("点数据: %1 (矢量)").arg(arrayName), 
                                       QVariant::fromValue(QPair<QString, bool>(arrayName, true)));
            }
        }
    }

    // 获取单元数据数组
    vtkCellData *cellData = m_currentData->GetCellData();
    for (int i = 0; i < cellData->GetNumberOfArrays(); ++i) {
        vtkDataArray *array = cellData->GetArray(i);
        if (array && array->GetName()) {
            QString arrayName = QString::fromStdString(array->GetName());
            int components = array->GetNumberOfComponents();
            
            if (components == 1) {
                // 标量数据
                m_dataComboBox->addItem(QString("单元数据: %1 (标量)").arg(arrayName), 
                                       QVariant::fromValue(QPair<QString, bool>(arrayName, false)));
            } else if (components == 3) {
                // 矢量数据
                m_dataComboBox->addItem(QString("单元数据: %1 (矢量)").arg(arrayName), 
                                       QVariant::fromValue(QPair<QString, bool>(arrayName, false)));
            }
        }
    }

    m_dataComboBox->setEnabled(m_dataComboBox->count() > 0);
}

void MainWindow::onDataSelectionChanged(const QString &displayName)
{
    if (!m_currentData || displayName.isEmpty()) {
        return;
    }

    // 从下拉框获取数据数组信息
    QVariant userData = m_dataComboBox->currentData();
    if (!userData.isValid()) {
        return;
    }

    QPair<QString, bool> arrayInfo = userData.value<QPair<QString, bool>>();
    QString arrayName = arrayInfo.first;
    bool isPointData = arrayInfo.second;

    m_currentDataArrayName = arrayName;

    // 设置映射器的输入数据
    m_mapper->SetInputData(m_currentData);
    m_wireframeMapper->SetInputData(m_currentData);

    // 检查是否是矢量数据
    vtkDataArray *dataArray = nullptr;
    if (isPointData) {
        dataArray = m_currentData->GetPointData()->GetArray(arrayName.toStdString().c_str());
    } else {
        dataArray = m_currentData->GetCellData()->GetArray(arrayName.toStdString().c_str());
    }

    bool isVectorData = (dataArray && dataArray->GetNumberOfComponents() == 3);

    if (isVectorData) {
        // 矢量数据 - 设置为矢量数组
        if (isPointData) {
            m_currentData->GetPointData()->SetActiveVectors(arrayName.toStdString().c_str());
        } else {
            m_currentData->GetCellData()->SetActiveVectors(arrayName.toStdString().c_str());
        }
        
        // 对于矢量数据，使用矢量的大小作为标量进行着色
        if (isPointData) {
            m_mapper->SetScalarModeToUsePointData();
            m_wireframeMapper->SetScalarModeToUsePointData();
        } else {
            m_mapper->SetScalarModeToUseCellData();
            m_wireframeMapper->SetScalarModeToUseCellData();
        }
        
        // 启用矢量场可视化面板
        if (m_vectorFieldDock) {
            m_vectorFieldDock->setEnabled(true);
            m_vectorFieldWidget->setActiveVectorArray(arrayName, isPointData);
        }
        
        qDebug() << "选择了矢量数据:" << arrayName << "组件数:" << dataArray->GetNumberOfComponents();
    } else {
        // 标量数据
        if (isPointData) {
            m_mapper->SetScalarModeToUsePointData();
            m_wireframeMapper->SetScalarModeToUsePointData();
            m_currentData->GetPointData()->SetActiveScalars(arrayName.toStdString().c_str());
        } else {
            m_mapper->SetScalarModeToUseCellData();
            m_wireframeMapper->SetScalarModeToUseCellData();
            m_currentData->GetCellData()->SetActiveScalars(arrayName.toStdString().c_str());
        }
        
        // 禁用矢量场可视化面板
        if (m_vectorFieldDock) {
            m_vectorFieldDock->setEnabled(false);
        }
        
        qDebug() << "选择了标量数据:" << arrayName;
    }

    // 更新可视化
    updateVisualization();
}

void MainWindow::updateVisualization()
{
    if (!m_currentData) {
        return;
    }

    // 获取数据范围
    double range[2];
    m_currentData->GetScalarRange(range);

    // 更新查找表范围
    m_lookupTable->SetTableRange(range);
    m_lookupTable->Build();

    // 设置映射器的查找表
    m_mapper->SetLookupTable(m_lookupTable);
    m_mapper->SetScalarRange(range);
    m_wireframeMapper->SetLookupTable(m_lookupTable);
    m_wireframeMapper->SetScalarRange(range);

    // 更新标量条
    m_scalarBar->SetLookupTable(m_lookupTable);
    m_scalarBar->SetTitle(m_currentDataArrayName.toStdString().c_str());

    // 使用新的显示模式更新函数
    updateDisplayMode();

    // 更新高级功能
    updateAdvancedFeatures();

    // 重置视图
    resetView();
}

void MainWindow::resetView()
{
    if (!m_renderer) {
        return;
    }

    m_renderer->ResetCamera();
    
    // 设置相机位置以获得更好的初始视角
    vtkCamera *camera = m_renderer->GetActiveCamera();
    camera->SetViewUp(0, 1, 0);
    camera->Azimuth(30);
    camera->Elevation(30);
    
    m_renderer->ResetCameraClippingRange();
}

void MainWindow::setupColorMaps()
{
    // 添加颜色映射选项
    m_colorMapComboBox->addItem("彩虹 (蓝→红)");
    m_colorMapComboBox->addItem("热力图 (黑→红→黄)");
    m_colorMapComboBox->addItem("蓝白红");
    m_colorMapComboBox->addItem("灰度");
    m_colorMapComboBox->addItem("绿蓝");
    m_colorMapComboBox->addItem("紫红");
}

void MainWindow::applyColorMap(const QString &colorMapName)
{
    if (!m_lookupTable) return;
    
    if (colorMapName == "彩虹 (蓝→红)") {
        m_lookupTable->SetHueRange(0.667, 0.0); // 蓝色到红色
        m_lookupTable->SetSaturationRange(1.0, 1.0);
        m_lookupTable->SetValueRange(1.0, 1.0);
    }
    else if (colorMapName == "热力图 (黑→红→黄)") {
        m_lookupTable->SetHueRange(0.0, 0.167); // 红色到黄色
        m_lookupTable->SetSaturationRange(1.0, 1.0);
        m_lookupTable->SetValueRange(0.0, 1.0); // 从黑到亮
    }
    else if (colorMapName == "蓝白红") {
        m_lookupTable->SetHueRange(0.667, 0.0); // 蓝到红
        m_lookupTable->SetSaturationRange(1.0, 1.0);
        m_lookupTable->SetValueRange(0.5, 1.0); // 较亮的颜色
    }
    else if (colorMapName == "灰度") {
        m_lookupTable->SetHueRange(0.0, 0.0); // 无色相
        m_lookupTable->SetSaturationRange(0.0, 0.0); // 无饱和度
        m_lookupTable->SetValueRange(0.0, 1.0); // 黑到白
    }
    else if (colorMapName == "绿蓝") {
        m_lookupTable->SetHueRange(0.333, 0.667); // 绿到蓝
        m_lookupTable->SetSaturationRange(1.0, 1.0);
        m_lookupTable->SetValueRange(1.0, 1.0);
    }
    else if (colorMapName == "紫红") {
        m_lookupTable->SetHueRange(0.833, 0.0); // 紫到红
        m_lookupTable->SetSaturationRange(1.0, 1.0);
        m_lookupTable->SetValueRange(1.0, 1.0);
    }
    
    m_lookupTable->SetAlphaRange(1.0, 1.0);
    m_lookupTable->SetNumberOfColors(256);
    m_lookupTable->Build();
    
    // 更新渲染
    if (m_currentData) {
        updateVisualization();
    }
}

void MainWindow::updateDisplayMode()
{
    if (!m_currentData && !m_currentGeometryData) return;
    
    // 清除之前的演员
    m_renderer->RemoveAllViewProps();
    
    // 总是添加主演员（实体）
    m_renderer->AddActor(m_actor);
    
    // 根据复选框状态添加网格演员
    if (m_wireframeCheckBox->isChecked()) {
        m_renderer->AddActor(m_wireframeActor);
    }
    
    // 只有VTK文件才添加标量条（STL文件没有标量数据）
    if (m_currentDataType == DATA_TYPE_UNSTRUCTURED_GRID) {
        m_renderer->AddActor2D(m_scalarBar);
    }
    
    // 刷新渲染
    m_renderWindow->Render();
}

void MainWindow::onDisplayModeChanged(int state)
{
    updateDisplayMode();
}

void MainWindow::onColorMapChanged(const QString &colorMapName)
{
    applyColorMap(colorMapName);
}

void MainWindow::onOpacityChanged(int value)
{
    if (!m_actor) return;
    
    // 检查是否有矢量场可视化处于活动状态
    bool vectorVisualizationActive = false;
    if (m_vectorFieldWidget) {
        bool warpEnabled = m_vectorFieldWidget->property("warpEnabled").toBool();
        bool streamlineEnabled = m_vectorFieldWidget->property("streamlineEnabled").toBool();
        vectorVisualizationActive = warpEnabled || streamlineEnabled;
    }
    
    float opacity;
    if (vectorVisualizationActive) {
        // 矢量场可视化活动时，强制使用10%透明度
        opacity = 0.1f;
        // 但仍然更新标签显示用户设置的值
        m_opacityLabel->setText(QString("%1% (矢量场模式: 10%)").arg(value));
    } else {
        // 正常模式，使用用户设置的透明度
        opacity = value / 100.0f;
        m_opacityLabel->setText(QString("%1%").arg(value));
    }
    
    m_actor->GetProperty()->SetOpacity(opacity);
    
    // 刷新渲染
    m_renderWindow->Render();
}

void MainWindow::updateAdvancedFeatures()
{
    // 几何文件不支持高级功能（剖切、等值面、数据拾取、矢量场）
    if (m_currentDataType == DATA_TYPE_GEOMETRY_ONLY) {
        // 禁用高级功能面板
        if (m_clippingDock) m_clippingDock->setEnabled(false);
        if (m_contourDock) m_contourDock->setEnabled(false);
        if (m_vectorFieldDock) m_vectorFieldDock->setEnabled(false);
        
        // 禁用数据拾取功能
        if (m_dataPicker) {
            m_dataPicker->enablePicking(false);
        }
        if (m_pickingAction) {
            m_pickingAction->setEnabled(false);
            m_pickingAction->setChecked(false);
        }
        
        // 更新状态标签
        QString fileExt = QFileInfo(m_currentFileName).suffix().toUpper();
        m_statusLabel->setText(QString("%1几何文件 - 仅支持基本显示功能").arg(fileExt));
        return;
    }
    
    if (!m_currentData) return;
    
    // 启用高级功能面板
    if (m_clippingDock) m_clippingDock->setEnabled(true);
    if (m_contourDock) m_contourDock->setEnabled(true);
    if (m_vectorFieldDock) m_vectorFieldDock->setEnabled(true);
    
    // 启用数据拾取功能（但不自动开启，由用户控制）
    if (m_pickingAction) {
        m_pickingAction->setEnabled(true);
    }
    
    // 更新剖切功能
    if (m_clippingWidget) {
        m_clippingWidget->setData(m_currentData);
        m_clippingWidget->setRenderer(m_renderer);
    }
    
    // 更新等值面功能
    if (m_contourWidget) {
        m_contourWidget->setData(m_currentData);
        m_contourWidget->setRenderer(m_renderer);
        
        // 设置活动标量数组
        if (!m_currentDataArrayName.isEmpty()) {
            // 检查是否是点数据还是单元数据
            bool isPointData = m_currentData->GetPointData()->GetArray(m_currentDataArrayName.toStdString().c_str()) != nullptr;
            m_contourWidget->setActiveScalarArray(m_currentDataArrayName, isPointData);
        }
        
        qDebug() << "MainWindow: 等值面功能已更新，当前数据数组:" << m_currentDataArrayName;
    }
    
    // 更新矢量场功能
    if (m_vectorFieldWidget) {
        m_vectorFieldWidget->setData(m_currentData);
        m_vectorFieldWidget->setRenderer(m_renderer);
        
        qDebug() << "MainWindow: 矢量场功能已更新";
    }
    
    // 更新数据拾取器
    if (m_dataPicker) {
        m_dataPicker->setData(m_currentData);
        m_dataPicker->setRenderer(m_renderer);
        m_dataPicker->setInteractor(m_renderWindow->GetInteractor());
        m_dataPicker->setActiveScalarArray(m_currentDataArrayName, true); // 假设是点数据
    }
}

void MainWindow::onClippingChanged()
{
    if (!m_clippingWidget || !m_renderer) return;
    
    // 移除之前的剖切演员
    if (m_clippingWidget->getClippedActor()) {
        m_renderer->RemoveActor(m_clippingWidget->getClippedActor());
    }
    
    // 检查是否启用了剖切
    bool clippingEnabled = m_clippingWidget->property("clippingEnabled").toBool();
    
    if (clippingEnabled) {
        // 如果启用了剖切，隐藏原始模型，显示剖切结果
        if (m_actor) {
            m_renderer->RemoveActor(m_actor);
        }
        if (m_wireframeActor) {
            m_renderer->RemoveActor(m_wireframeActor);
        }
        
        // 添加剖切演员
        if (m_clippingWidget->getClippedActor()) {
            m_renderer->AddActor(m_clippingWidget->getClippedActor());
        }
    } else {
        // 如果禁用了剖切，恢复原始模型显示
        updateDisplayMode(); // 这会重新添加原始模型
    }
    
    m_renderWindow->Render();
}

void MainWindow::onContoursChanged()
{
    if (!m_contourWidget || !m_renderer) return;
    
    // 移除之前的等值面演员
    if (m_contourWidget->getContourActor()) {
        m_renderer->RemoveActor(m_contourWidget->getContourActor());
    }
    
    // 检查是否启用了等值面并且有等值面数据
    bool contourEnabled = m_contourWidget->property("contourEnabled").toBool();
    bool hasContours = m_contourWidget->property("hasContours").toBool();
    
    if (contourEnabled && hasContours) {
        // 添加等值面演员
        if (m_contourWidget->getContourActor()) {
            m_renderer->AddActor(m_contourWidget->getContourActor());
        }
    }
    
    m_renderWindow->Render();
}

void MainWindow::onPointPicked(const QString &info)
{
    statusBar()->showMessage(info);
}

void MainWindow::onVTKWidgetMousePress(QMouseEvent *event)
{
    if (m_dataPicker && m_dataPicker->isPickingEnabled()) {
        m_dataPicker->onMouseClick(event->x(), event->y());
    }
}

void MainWindow::setupGeometryVisualization()
{
    if (!m_currentGeometryData) return;
    
    // 创建几何映射器
    m_geometryMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    m_geometryMapper->SetInputData(m_currentGeometryData);
    
    // 设置主演员使用几何映射器
    m_actor->SetMapper(m_geometryMapper);
    
    // 设置网格演员也使用几何映射器
    m_wireframeActor->SetMapper(m_geometryMapper);
    
    // 根据文件类型设置默认颜色
    QString fileExt = QFileInfo(m_currentFileName).suffix().toLower();
    if (fileExt == "stl") {
        m_actor->GetProperty()->SetColor(0.8, 0.8, 0.8);  // STL: 灰色
    } else if (fileExt == "obj") {
        m_actor->GetProperty()->SetColor(0.9, 0.7, 0.5);  // OBJ: 米色
    } else if (fileExt == "ply") {
        m_actor->GetProperty()->SetColor(0.6, 0.8, 0.9);  // PLY: 浅蓝色
    } else {
        m_actor->GetProperty()->SetColor(0.8, 0.8, 0.8);  // 默认: 灰色
    }
    
    // 清除渲染器中的所有演员
    m_renderer->RemoveAllViewProps();
    
    // 添加几何演员
    m_renderer->AddActor(m_actor);
    
    // 重置视图
    resetView();
    
    // 刷新渲染
    m_renderWindow->Render();
    
    qDebug() << "几何文件已加载 (" << fileExt.toUpper() << ")，点数:" << m_currentGeometryData->GetNumberOfPoints()
             << "面数:" << m_currentGeometryData->GetNumberOfCells();
}

void MainWindow::onVectorVisualizationChanged()
{
    if (!m_vectorFieldWidget || !m_renderer) return;
    
    // 移除之前的矢量场演员
    if (m_vectorFieldWidget->getWarpActor()) {
        m_renderer->RemoveActor(m_vectorFieldWidget->getWarpActor());
    }
    if (m_vectorFieldWidget->getOriginalActor()) {
        m_renderer->RemoveActor(m_vectorFieldWidget->getOriginalActor());
    }
    if (m_vectorFieldWidget->getStreamlineActor()) {
        m_renderer->RemoveActor(m_vectorFieldWidget->getStreamlineActor());
    }
    
    // 检查是否启用了变形图或流线
    bool warpEnabled = m_vectorFieldWidget->property("warpEnabled").toBool();
    bool streamlineEnabled = m_vectorFieldWidget->property("streamlineEnabled").toBool();
    bool vectorVisualizationActive = warpEnabled || streamlineEnabled;
    
    if (vectorVisualizationActive) {
        // 启用矢量场可视化时，将主几何体透明度降低到10%
        if (m_actor) {
            m_actor->SetVisibility(true);
            m_actor->GetProperty()->SetOpacity(0.1);
        }
    } else {
        // 恢复主几何体的正常透明度
        if (m_actor) {
            m_actor->SetVisibility(true);
            // 恢复用户设置的透明度
            double userOpacity = m_opacitySlider->value() / 100.0;
            m_actor->GetProperty()->SetOpacity(userOpacity);
        }
    }
    
    // 添加变形图
    if (warpEnabled) {
        if (m_vectorFieldWidget->getWarpActor()) {
            m_renderer->AddActor(m_vectorFieldWidget->getWarpActor());
        }
        
        // 总是添加原始几何体，由VectorFieldWidget内部控制显示状态
        if (m_vectorFieldWidget->getOriginalActor()) {
            m_renderer->AddActor(m_vectorFieldWidget->getOriginalActor());
        }
    }
    
    // 添加流线
    if (streamlineEnabled) {
        if (m_vectorFieldWidget->getStreamlineActor()) {
            m_renderer->AddActor(m_vectorFieldWidget->getStreamlineActor());
        }
    }
    
    // 更新透明度标签显示
    if (vectorVisualizationActive) {
        int currentValue = m_opacitySlider->value();
        m_opacityLabel->setText(QString("%1% (矢量场模式: 10%)").arg(currentValue));
    } else {
        int currentValue = m_opacitySlider->value();
        m_opacityLabel->setText(QString("%1%").arg(currentValue));
    }
    
    m_renderWindow->Render();
}