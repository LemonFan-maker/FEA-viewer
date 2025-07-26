#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QSlider>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QGroupBox>

#include <QVTKOpenGLNativeWidget.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkUnstructuredGridReader.h>
#include <vtkSTLReader.h>
#include <vtkOBJReader.h>
#include <vtkPLYReader.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyData.h>
#include <vtkDataSetMapper.h>
#include <vtkActor.h>
#include <vtkScalarBarActor.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkLookupTable.h>
#include <vtkTextProperty.h>
#include <vtkCamera.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkSmartPointer.h>
#include <vtkNew.h>
#include <vtkProperty.h>
#include <vtkAxesActor.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkColorSeries.h>
#include <QDockWidget>
#include <QStatusBar>

// 包含功能模块
#include "visualization/ClippingWidget.h"
#include "visualization/ContourWidget.h"
#include "visualization/VectorFieldWidget.h"
#include "interaction/DataPicker.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openFile();
    void onDataSelectionChanged(const QString &dataName);
    void onDisplayModeChanged(int state);
    void onColorMapChanged(const QString &colorMapName);
    void onOpacityChanged(int value);
    void onClippingChanged();
    void onContoursChanged();
    void onVectorVisualizationChanged();
    void onPointPicked(const QString &info);
    void onVTKWidgetMousePress(QMouseEvent *event);

private:
    void setupUI();
    void setupVTK();
    void setupDockWidgets();
    void updateVisualization();
    void populateDataComboBox();
    void resetView();
    void setupColorMaps();
    void applyColorMap(const QString &colorMapName);
    void updateDisplayMode();
    void updateAdvancedFeatures();
    void setupGeometryVisualization();

    // UI组件
    QPushButton *m_openFileButton;
    QComboBox *m_dataComboBox;
    QCheckBox *m_wireframeCheckBox;
    QComboBox *m_colorMapComboBox;
    QSlider *m_opacitySlider;
    QLabel *m_statusLabel;
    QLabel *m_opacityLabel;
    QVTKOpenGLNativeWidget *m_vtkWidget;

    // VTK组件
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_renderWindow;
    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkXMLUnstructuredGridReader> m_xmlReader;
    vtkSmartPointer<vtkUnstructuredGridReader> m_legacyReader;
    vtkSmartPointer<vtkSTLReader> m_stlReader;
    vtkSmartPointer<vtkOBJReader> m_objReader;
    vtkSmartPointer<vtkPLYReader> m_plyReader;
    vtkSmartPointer<vtkPolyDataMapper> m_geometryMapper;
    vtkSmartPointer<vtkDataSetMapper> m_mapper;
    vtkSmartPointer<vtkDataSetMapper> m_wireframeMapper;
    vtkSmartPointer<vtkActor> m_actor;
    vtkSmartPointer<vtkActor> m_wireframeActor;
    vtkSmartPointer<vtkScalarBarActor> m_scalarBar;
    vtkSmartPointer<vtkLookupTable> m_lookupTable;
    vtkSmartPointer<vtkAxesActor> m_axesActor;
    vtkSmartPointer<vtkOrientationMarkerWidget> m_orientationWidget;

    // 功能模块
    ClippingWidget *m_clippingWidget;
    ContourWidget *m_contourWidget;
    VectorFieldWidget *m_vectorFieldWidget;
    DataPicker *m_dataPicker;
    
    // 停靠窗口
    QDockWidget *m_clippingDock;
    QDockWidget *m_contourDock;
    QDockWidget *m_vectorFieldDock;
    
    // 菜单项
    QAction *m_pickingAction;

    // 数据类型枚举
    enum DataType {
        DATA_TYPE_NONE,
        DATA_TYPE_UNSTRUCTURED_GRID,  // VTK分析数据
        DATA_TYPE_GEOMETRY_ONLY       // 纯几何数据（STL、OBJ、PLY等）
    };
    
    // 数据
    vtkSmartPointer<vtkUnstructuredGrid> m_currentData;
    vtkSmartPointer<vtkPolyData> m_currentGeometryData;
    QString m_currentFileName;
    QString m_currentDataArrayName;
    DataType m_currentDataType;
};

#endif // MAINWINDOW_H