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
    QHBoxLayout *toolbarLayout = new QHBoxLayout();
    
    // 打开文件按钮
    m_openFileButton = new QPushButton("打开文件", this);
    m_openFileButton->setMaximumWidth(100);
    connect(m_openFileButton, &QPushButton::clicked, this, &MainWindow::openFile);
    
    // 数据选择下拉框
    QLabel *dataLabel = new QLabel("数据类型:", this);
    m_dataComboBox = new QComboBox(this);
    m_dataComboBox->setMinimumWidth(250);
    m_dataComboBox->setMaximumWidth(350);
    m_dataComboBox->setEnabled(false);
    connect(m_dataComboBox, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            this, &MainWindow::onDataSelectionChanged);
    
    // 状态标签
    m_statusLabel = new QLabel("请选择一个VTK文件", this);
    
    toolbarLayout->addWidget(m_openFileButton);
    toolbarLayout->addWidget(dataLabel);
    toolbarLayout->addWidget(m_dataComboBox);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(m_statusLabel);

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

    // 创建演员
    m_actor = vtkSmartPointer<vtkActor>::New();
    m_actor->SetMapper(m_mapper);

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

    // 设置标量数据
    if (isPointData) {
        m_mapper->SetScalarModeToUsePointData();
        m_currentData->GetPointData()->SetActiveScalars(arrayName.toStdString().c_str());
    } else {
        m_mapper->SetScalarModeToUseCellData();
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

    // 更新标量条
    m_scalarBar->SetLookupTable(m_lookupTable);
    m_scalarBar->SetTitle(m_currentDataArrayName.toStdString().c_str());

    // 清除之前的演员
    m_renderer->RemoveAllViewProps();

    // 添加新的演员
    m_renderer->AddActor(m_actor);
    m_renderer->AddActor2D(m_scalarBar);

    // 重置视图
    resetView();

    // 刷新渲染
    m_renderWindow->Render();
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

