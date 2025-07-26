#include "MainWindow.h"
#include <QFileInfo>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_openFileButton(nullptr)
    , m_dataComboBox(nullptr)
    , m_statusLabel(nullptr)
    , m_vtkWidget(nullptr)
    , m_currentData(nullptr)
{
    setupUI();
    setupVTK();
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
    mainLayout->addLayout(toolbarLayout);
    mainLayout->addWidget(m_vtkWidget, 1);
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
}void
 MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "打开VTK文件",
        "",
        "VTK文件 (*.vtu *.vtk);;所有文件 (*.*)"
    );

    if (fileName.isEmpty()) {
        return;
    }

    m_currentFileName = fileName;
    
    // 根据文件扩展名选择合适的读取器
    if (fileName.endsWith(".vtu", Qt::CaseInsensitive)) {
        // XML格式
        m_xmlReader = vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
        m_xmlReader->SetFileName(fileName.toStdString().c_str());
        m_xmlReader->Update();
        
        if (m_xmlReader->GetOutput()->GetNumberOfCells() == 0) {
            QMessageBox::warning(this, "错误", "无法读取文件或文件为空");
            return;
        }
        
        m_currentData = m_xmlReader->GetOutput();
    }
    else if (fileName.endsWith(".vtk", Qt::CaseInsensitive)) {
        // Legacy格式
        m_legacyReader = vtkSmartPointer<vtkUnstructuredGridReader>::New();
        m_legacyReader->SetFileName(fileName.toStdString().c_str());
        m_legacyReader->Update();
        
        if (m_legacyReader->GetOutput()->GetNumberOfCells() == 0) {
            QMessageBox::warning(this, "错误", "无法读取文件或文件为空");
            return;
        }
        
        m_currentData = m_legacyReader->GetOutput();
    }
    else {
        QMessageBox::warning(this, "错误", "不支持的文件格式");
        return;
    }

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
            m_dataComboBox->addItem(QString("点数据: %1").arg(arrayName), 
                                   QVariant::fromValue(QPair<QString, bool>(arrayName, true)));
        }
    }

    // 获取单元数据数组
    vtkCellData *cellData = m_currentData->GetCellData();
    for (int i = 0; i < cellData->GetNumberOfArrays(); ++i) {
        vtkDataArray *array = cellData->GetArray(i);
        if (array && array->GetName()) {
            QString arrayName = QString::fromStdString(array->GetName());
            m_dataComboBox->addItem(QString("单元数据: %1").arg(arrayName), 
                                   QVariant::fromValue(QPair<QString, bool>(arrayName, false)));
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

    // 设置标量数据
    if (isPointData) {
        m_mapper->SetScalarModeToUsePointData();
        m_wireframeMapper->SetScalarModeToUsePointData();
        m_currentData->GetPointData()->SetActiveScalars(arrayName.toStdString().c_str());
    } else {
        m_mapper->SetScalarModeToUseCellData();
        m_wireframeMapper->SetScalarModeToUseCellData();
        m_currentData->GetCellData()->SetActiveScalars(arrayName.toStdString().c_str());
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
    if (!m_currentData) return;
    
    // 清除之前的演员
    m_renderer->RemoveAllViewProps();
    
    // 总是添加主演员（实体）
    m_renderer->AddActor(m_actor);
    
    // 根据复选框状态添加网格演员
    if (m_wireframeCheckBox->isChecked()) {
        m_renderer->AddActor(m_wireframeActor);
    }
    
    // 添加标量条
    m_renderer->AddActor2D(m_scalarBar);
    
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
    
    float opacity = value / 100.0f;
    m_actor->GetProperty()->SetOpacity(opacity);
    
    // 更新标签
    m_opacityLabel->setText(QString("%1%").arg(value));
    
    // 刷新渲染
    m_renderWindow->Render();
}

