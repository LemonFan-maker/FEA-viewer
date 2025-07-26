#ifndef VECTORFIELDWIDGET_H
#define VECTORFIELDWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QSlider>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QPushButton>

#include <vtkSmartPointer.h>
#include <vtkWarpVector.h>
#include <vtkStreamTracer.h>
#include <vtkLineSource.h>
#include <vtkPointSource.h>
#include <vtkDataSetMapper.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkProperty.h>
#include <vtkLookupTable.h>

class VectorFieldWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VectorFieldWidget(QWidget *parent = nullptr);
    ~VectorFieldWidget();

    void setData(vtkUnstructuredGrid *data);
    void setRenderer(vtkRenderer *renderer);
    void setActiveVectorArray(const QString &arrayName, bool isPointData);
    
    vtkActor* getWarpActor() const { return m_warpActor; }
    vtkActor* getOriginalActor() const { return m_originalActor; }
    vtkActor* getStreamlineActor() const { return m_streamlineActor; }

signals:
    void vectorVisualizationChanged();

private slots:
    void onWarpEnabledChanged(bool enabled);
    void onStreamlineEnabledChanged(bool enabled);
    void onWarpParametersChanged();
    void onStreamlineParametersChanged();
    void onShowOriginalChanged(bool enabled);
    void onVisualizationModeChanged();

private:
    void setupUI();
    void setupVTK();
    void updateWarpVisualization();
    void updateStreamlineVisualization();
    void createStreamlineSeeds();

    // UI组件
    QCheckBox *m_enableWarpCheckBox;
    QCheckBox *m_enableStreamlineCheckBox;
    
    // 变形图控制
    QGroupBox *m_warpGroup;
    QSlider *m_warpScaleSlider;
    QCheckBox *m_showOriginalCheckBox;
    QLabel *m_warpScaleLabel;
    
    // 流线控制
    QGroupBox *m_streamlineGroup;
    QSpinBox *m_streamlineCountSpinBox;
    QDoubleSpinBox *m_integrationStepSpinBox;
    QSpinBox *m_maxStepsSpinBox;
    QComboBox *m_seedModeComboBox;
    QPushButton *m_regenerateStreamlinesButton;

    // VTK组件 - 变形图
    vtkSmartPointer<vtkWarpVector> m_warpFilter;
    vtkSmartPointer<vtkDataSetMapper> m_warpMapper;
    vtkSmartPointer<vtkActor> m_warpActor;
    vtkSmartPointer<vtkDataSetMapper> m_originalMapper;
    vtkSmartPointer<vtkActor> m_originalActor;
    
    // VTK组件 - 流线
    vtkSmartPointer<vtkStreamTracer> m_streamTracer;
    vtkSmartPointer<vtkPointSource> m_seedSource;
    vtkSmartPointer<vtkPolyDataMapper> m_streamlineMapper;
    vtkSmartPointer<vtkActor> m_streamlineActor;
    
    // 数据
    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkUnstructuredGrid> m_inputData;
    QString m_activeVectorArrayName;
    bool m_isPointData;
    bool m_warpEnabled;
    bool m_streamlineEnabled;
};

#endif // VECTORFIELDWIDGET_H