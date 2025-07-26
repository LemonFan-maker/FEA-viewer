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

private:
    void setupUI();
    void setupVTK();
    void updateVisualization();
    void populateDataComboBox();
    void resetView();
    void setupColorMaps();
    void applyColorMap(const QString &colorMapName);
    void updateDisplayMode();

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
    vtkSmartPointer<vtkDataSetMapper> m_mapper;
    vtkSmartPointer<vtkDataSetMapper> m_wireframeMapper;
    vtkSmartPointer<vtkActor> m_actor;
    vtkSmartPointer<vtkActor> m_wireframeActor;
    vtkSmartPointer<vtkScalarBarActor> m_scalarBar;
    vtkSmartPointer<vtkLookupTable> m_lookupTable;
    vtkSmartPointer<vtkAxesActor> m_axesActor;
    vtkSmartPointer<vtkOrientationMarkerWidget> m_orientationWidget;

    // 数据
    vtkSmartPointer<vtkUnstructuredGrid> m_currentData;
    QString m_currentFileName;
    QString m_currentDataArrayName;
};

#endif // MAINWINDOW_H