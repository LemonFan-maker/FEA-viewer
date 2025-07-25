#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>

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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openFile();
    void onDataSelectionChanged(const QString &dataName);

private:
    void setupUI();
    void setupVTK();
    void updateVisualization();
    void populateDataComboBox();
    void resetView();

    // UI组件
    QPushButton *m_openFileButton;
    QComboBox *m_dataComboBox;
    QLabel *m_statusLabel;
    QVTKOpenGLNativeWidget *m_vtkWidget;

    // VTK组件
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_renderWindow;
    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkXMLUnstructuredGridReader> m_xmlReader;
    vtkSmartPointer<vtkUnstructuredGridReader> m_legacyReader;
    vtkSmartPointer<vtkDataSetMapper> m_mapper;
    vtkSmartPointer<vtkActor> m_actor;
    vtkSmartPointer<vtkScalarBarActor> m_scalarBar;
    vtkSmartPointer<vtkLookupTable> m_lookupTable;

    // 数据
    vtkSmartPointer<vtkUnstructuredGrid> m_currentData;
    QString m_currentFileName;
    QString m_currentDataArrayName;
};

#endif // MAINWINDOW_H