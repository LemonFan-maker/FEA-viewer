#include "VectorFieldWidget.h"
#include <QDebug>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkWarpVector.h>

VectorFieldWidget::VectorFieldWidget(QWidget *parent)
    : QWidget(parent)
    , m_renderer(nullptr)
    , m_inputData(nullptr)
    , m_isPointData(true)
    , m_warpEnabled(false)
    , m_streamlineEnabled(false)
{
    setupUI();
    setupVTK();
}

VectorFieldWidget::~VectorFieldWidget()
{
}

void VectorFieldWidget::setupUI()
{
    setWindowTitle("矢量场可视化");
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 变形图控制组
    m_warpGroup = new QGroupBox("变形图 (Warp by Vector)", this);
    QVBoxLayout *warpLayout = new QVBoxLayout(m_warpGroup);
    
    m_enableWarpCheckBox = new QCheckBox("启用变形图", this);
    connect(m_enableWarpCheckBox, &QCheckBox::toggled,
            this, &VectorFieldWidget::onWarpEnabledChanged);
    warpLayout->addWidget(m_enableWarpCheckBox);
    
    // 变形缩放
    QHBoxLayout *scaleLayout = new QHBoxLayout();
    scaleLayout->addWidget(new QLabel("变形缩放:", this));
    m_warpScaleSlider = new QSlider(Qt::Horizontal, this);
    m_warpScaleSlider->setRange(1, 100);
    m_warpScaleSlider->setValue(10);
    m_warpScaleSlider->setEnabled(false);
    connect(m_warpScaleSlider, &QSlider::valueChanged,
            this, &VectorFieldWidget::onWarpParametersChanged);
    scaleLayout->addWidget(m_warpScaleSlider);
    m_warpScaleLabel = new QLabel("1.0", this);
    m_warpScaleLabel->setMinimumWidth(40);
    scaleLayout->addWidget(m_warpScaleLabel);
    warpLayout->addLayout(scaleLayout);
    
    // 显示原始几何体
    m_showOriginalCheckBox = new QCheckBox("显示原始几何体", this);
    m_showOriginalCheckBox->setEnabled(false);
    m_showOriginalCheckBox->setChecked(true);
    connect(m_showOriginalCheckBox, &QCheckBox::toggled,
            this, &VectorFieldWidget::onShowOriginalChanged);
    warpLayout->addWidget(m_showOriginalCheckBox);
    
    mainLayout->addWidget(m_warpGroup);
    
    // 流线控制组
    m_streamlineGroup = new QGroupBox("流线 (Streamlines)", this);
    QVBoxLayout *streamlineLayout = new QVBoxLayout(m_streamlineGroup);
    
    m_enableStreamlineCheckBox = new QCheckBox("启用流线", this);
    connect(m_enableStreamlineCheckBox, &QCheckBox::toggled,
            this, &VectorFieldWidget::onStreamlineEnabledChanged);
    streamlineLayout->addWidget(m_enableStreamlineCheckBox);
    
    // 流线数量
    QHBoxLayout *countLayout = new QHBoxLayout();
    countLayout->addWidget(new QLabel("流线数量:", this));
    m_streamlineCountSpinBox = new QSpinBox(this);
    m_streamlineCountSpinBox->setRange(10, 1000);
    m_streamlineCountSpinBox->setValue(50);
    m_streamlineCountSpinBox->setEnabled(false);
    connect(m_streamlineCountSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &VectorFieldWidget::onStreamlineParametersChanged);
    countLayout->addWidget(m_streamlineCountSpinBox);
    streamlineLayout->addLayout(countLayout);
    
    // 积分步长
    QHBoxLayout *stepLayout = new QHBoxLayout();
    stepLayout->addWidget(new QLabel("积分步长:", this));
    m_integrationStepSpinBox = new QDoubleSpinBox(this);
    m_integrationStepSpinBox->setRange(0.001, 1.0);
    m_integrationStepSpinBox->setValue(0.01);
    m_integrationStepSpinBox->setDecimals(3);
    m_integrationStepSpinBox->setEnabled(false);
    connect(m_integrationStepSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &VectorFieldWidget::onStreamlineParametersChanged);
    stepLayout->addWidget(m_integrationStepSpinBox);
    streamlineLayout->addLayout(stepLayout);
    
    // 最大步数
    QHBoxLayout *maxStepsLayout = new QHBoxLayout();
    maxStepsLayout->addWidget(new QLabel("最大步数:", this));
    m_maxStepsSpinBox = new QSpinBox(this);
    m_maxStepsSpinBox->setRange(100, 10000);
    m_maxStepsSpinBox->setValue(2000);
    m_maxStepsSpinBox->setEnabled(false);
    connect(m_maxStepsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &VectorFieldWidget::onStreamlineParametersChanged);
    maxStepsLayout->addWidget(m_maxStepsSpinBox);
    streamlineLayout->addLayout(maxStepsLayout);
    
    // 种子点模式
    QHBoxLayout *seedLayout = new QHBoxLayout();
    seedLayout->addWidget(new QLabel("种子点:", this));
    m_seedModeComboBox = new QComboBox(this);
    m_seedModeComboBox->addItem("随机分布");
    m_seedModeComboBox->addItem("边界分布");
    m_seedModeComboBox->setEnabled(false);
    connect(m_seedModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &VectorFieldWidget::onStreamlineParametersChanged);
    seedLayout->addWidget(m_seedModeComboBox);
    streamlineLayout->addLayout(seedLayout);
    
    // 重新生成按钮
    m_regenerateStreamlinesButton = new QPushButton("重新生成流线", this);
    m_regenerateStreamlinesButton->setEnabled(false);
    connect(m_regenerateStreamlinesButton, &QPushButton::clicked,
            this, &VectorFieldWidget::onStreamlineParametersChanged);
    streamlineLayout->addWidget(m_regenerateStreamlinesButton);
    
    mainLayout->addWidget(m_streamlineGroup);
    
    mainLayout->addStretch();
}

void VectorFieldWidget::setupVTK()
{
    // 设置变形图组件
    m_warpFilter = vtkSmartPointer<vtkWarpVector>::New();
    m_warpFilter->SetScaleFactor(1.0);
    
    m_warpMapper = vtkSmartPointer<vtkDataSetMapper>::New();
    m_warpMapper->SetInputConnection(m_warpFilter->GetOutputPort());
    
    m_warpActor = vtkSmartPointer<vtkActor>::New();
    m_warpActor->SetMapper(m_warpMapper);
    
    // 设置原始几何体显示
    m_originalMapper = vtkSmartPointer<vtkDataSetMapper>::New();
    
    m_originalActor = vtkSmartPointer<vtkActor>::New();
    m_originalActor->SetMapper(m_originalMapper);
    m_originalActor->GetProperty()->SetRepresentationToWireframe();
    m_originalActor->GetProperty()->SetColor(0.3, 0.3, 0.3); // 深灰色线框
    m_originalActor->GetProperty()->SetLineWidth(2.0);
    m_originalActor->GetProperty()->SetOpacity(0.6); // 半透明
    m_originalActor->SetVisibility(false); // 默认隐藏
    
    // 设置流线组件
    m_streamTracer = vtkSmartPointer<vtkStreamTracer>::New();
    m_streamTracer->SetIntegrationDirectionToBoth();
    m_streamTracer->SetIntegratorTypeToRungeKutta4();
    
    m_seedSource = vtkSmartPointer<vtkPointSource>::New();
    
    m_streamlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    m_streamlineMapper->SetInputConnection(m_streamTracer->GetOutputPort());
    
    m_streamlineActor = vtkSmartPointer<vtkActor>::New();
    m_streamlineActor->SetMapper(m_streamlineMapper);
    m_streamlineActor->GetProperty()->SetColor(0.0, 1.0, 0.0); // 绿色流线
    m_streamlineActor->GetProperty()->SetLineWidth(2.0);
}

void VectorFieldWidget::setData(vtkUnstructuredGrid *data)
{
    m_inputData = data;
    if (m_inputData) {
        qDebug() << "VectorFieldWidget: 设置数据，点数:" << m_inputData->GetNumberOfPoints()
                 << "单元数:" << m_inputData->GetNumberOfCells();
        
        // 检查矢量数据数组
        vtkPointData* pointData = m_inputData->GetPointData();
        qDebug() << "点数据数组数量:" << pointData->GetNumberOfArrays();
        for (int i = 0; i < pointData->GetNumberOfArrays(); ++i) {
            vtkDataArray* array = pointData->GetArray(i);
            if (array && array->GetName() && array->GetNumberOfComponents() == 3) {
                qDebug() << "  矢量点数据数组" << i << ":" << array->GetName()
                         << "组件数:" << array->GetNumberOfComponents();
            }
        }
        
        vtkCellData* cellData = m_inputData->GetCellData();
        qDebug() << "单元数据数组数量:" << cellData->GetNumberOfArrays();
        for (int i = 0; i < cellData->GetNumberOfArrays(); ++i) {
            vtkDataArray* array = cellData->GetArray(i);
            if (array && array->GetName() && array->GetNumberOfComponents() == 3) {
                qDebug() << "  矢量单元数据数组" << i << ":" << array->GetName()
                         << "组件数:" << array->GetNumberOfComponents();
            }
        }
    }
}

void VectorFieldWidget::setRenderer(vtkRenderer *renderer)
{
    m_renderer = renderer;
}

void VectorFieldWidget::setActiveVectorArray(const QString &arrayName, bool isPointData)
{
    if (!m_inputData || arrayName.isEmpty()) return;
    
    m_activeVectorArrayName = arrayName;
    m_isPointData = isPointData;
    
    qDebug() << "VectorFieldWidget: 设置活动矢量数组:" << arrayName << "是否为点数据:" << isPointData;
    
    if (isPointData) {
        m_inputData->GetPointData()->SetActiveVectors(arrayName.toStdString().c_str());
    } else {
        m_inputData->GetCellData()->SetActiveVectors(arrayName.toStdString().c_str());
    }
    
    // 更新可视化
    if (m_warpEnabled) {
        updateWarpVisualization();
    }
    if (m_streamlineEnabled) {
        updateStreamlineVisualization();
    }
}

void VectorFieldWidget::onWarpEnabledChanged(bool enabled)
{
    m_warpEnabled = enabled;
    
    // 设置属性供外部查询
    setProperty("warpEnabled", enabled);
    
    // 启用/禁用控件
    m_warpScaleSlider->setEnabled(enabled);
    m_showOriginalCheckBox->setEnabled(enabled);
    
    if (enabled) {
        updateWarpVisualization();
        // 启用变形图时，根据复选框状态设置原始几何体可见性
        if (m_originalActor) {
            m_originalActor->SetVisibility(m_showOriginalCheckBox->isChecked());
        }
    } else {
        // 禁用变形图时，隐藏原始几何体
        if (m_originalActor) {
            m_originalActor->SetVisibility(false);
        }
    }
    
    emit vectorVisualizationChanged();
}

void VectorFieldWidget::onStreamlineEnabledChanged(bool enabled)
{
    m_streamlineEnabled = enabled;
    
    // 设置属性供外部查询
    setProperty("streamlineEnabled", enabled);
    
    // 启用/禁用控件
    m_streamlineCountSpinBox->setEnabled(enabled);
    m_integrationStepSpinBox->setEnabled(enabled);
    m_maxStepsSpinBox->setEnabled(enabled);
    m_seedModeComboBox->setEnabled(enabled);
    m_regenerateStreamlinesButton->setEnabled(enabled);
    
    if (enabled) {
        updateStreamlineVisualization();
    }
    
    emit vectorVisualizationChanged();
}

void VectorFieldWidget::onWarpParametersChanged()
{
    // 更新标签
    double scale = m_warpScaleSlider->value() / 10.0;
    m_warpScaleLabel->setText(QString::number(scale, 'f', 1));
    
    if (m_warpEnabled) {
        updateWarpVisualization();
    }
}

void VectorFieldWidget::onShowOriginalChanged(bool enabled)
{
    if (m_originalActor) {
        m_originalActor->SetVisibility(enabled);
    }
    emit vectorVisualizationChanged();
}

void VectorFieldWidget::onStreamlineParametersChanged()
{
    if (m_streamlineEnabled) {
        updateStreamlineVisualization();
    }
}

void VectorFieldWidget::updateWarpVisualization()
{
    if (!m_inputData || m_activeVectorArrayName.isEmpty()) return;
    
    // 设置变形图的输入数据
    m_warpFilter->SetInputData(m_inputData);
    
    // 设置缩放因子
    double scale = m_warpScaleSlider->value() / 10.0;
    m_warpFilter->SetScaleFactor(scale);
    
    // 更新变形图
    m_warpFilter->Update();
    
    // 设置原始几何体的输入数据
    m_originalMapper->SetInputData(m_inputData);
    
    qDebug() << "VectorFieldWidget: 更新变形图，缩放因子:" << scale
             << "输出点数:" << m_warpFilter->GetOutput()->GetNumberOfPoints()
             << "输出单元数:" << m_warpFilter->GetOutput()->GetNumberOfCells();
}

void VectorFieldWidget::updateStreamlineVisualization()
{
    if (!m_inputData || m_activeVectorArrayName.isEmpty()) return;
    
    // 设置流线追踪器的输入
    m_streamTracer->SetInputData(m_inputData);
    
    // 创建种子点
    createStreamlineSeeds();
    m_streamTracer->SetSourceConnection(m_seedSource->GetOutputPort());
    
    // 设置积分参数
    m_streamTracer->SetInitialIntegrationStep(m_integrationStepSpinBox->value());
    m_streamTracer->SetMaximumNumberOfSteps(m_maxStepsSpinBox->value());
    
    m_streamTracer->Update();
    
    qDebug() << "VectorFieldWidget: 更新流线，种子点数:" << m_seedSource->GetOutput()->GetNumberOfPoints()
             << "流线输出点数:" << m_streamTracer->GetOutput()->GetNumberOfPoints()
             << "流线输出单元数:" << m_streamTracer->GetOutput()->GetNumberOfCells();
}

void VectorFieldWidget::createStreamlineSeeds()
{
    if (!m_inputData) return;
    
    // 获取数据边界
    double bounds[6];
    m_inputData->GetBounds(bounds);
    
    // 设置种子点源
    m_seedSource->SetNumberOfPoints(m_streamlineCountSpinBox->value());
    
    if (m_seedModeComboBox->currentIndex() == 0) {
        // 随机分布
        m_seedSource->SetCenter(
            (bounds[0] + bounds[1]) / 2.0,
            (bounds[2] + bounds[3]) / 2.0,
            (bounds[4] + bounds[5]) / 2.0
        );
        m_seedSource->SetRadius(
            qMax(qMax(bounds[1] - bounds[0], bounds[3] - bounds[2]), bounds[5] - bounds[4]) * 0.3
        );
    } else {
        // 边界分布
        m_seedSource->SetCenter(bounds[0], bounds[2], bounds[4]);
        m_seedSource->SetRadius(
            qMax(qMax(bounds[1] - bounds[0], bounds[3] - bounds[2]), bounds[5] - bounds[4]) * 0.1
        );
    }
    
    m_seedSource->Update();
}

void VectorFieldWidget::onVisualizationModeChanged()
{
    emit vectorVisualizationChanged();
}

