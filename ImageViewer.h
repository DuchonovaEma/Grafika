#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets>
#include "ui_ImageViewer.h"
#include "ViewerWidget.h"

class ImageViewer : public QMainWindow
{
	Q_OBJECT

public:
	ImageViewer(QWidget* parent = Q_NULLPTR);
	~ImageViewer() { delete ui; }
private:
	Ui::ImageViewerClass* ui;
	ViewerWidget* vW;

	QColor globalColor;
	QSettings settings;
	QMessageBox msgBox;

	//Event filters
	bool eventFilter(QObject* obj, QEvent* event);

	//ViewerWidget Events
	bool ViewerWidgetEventFilter(QObject* obj, QEvent* event);
	void ViewerWidgetMouseButtonPress(ViewerWidget* w, QEvent* event);
	void ViewerWidgetMouseButtonRelease(ViewerWidget* w, QEvent* event);
	void ViewerWidgetMouseMove(ViewerWidget* w, QEvent* event);
	void ViewerWidgetLeave(ViewerWidget* w, QEvent* event);
	void ViewerWidgetEnter(ViewerWidget* w, QEvent* event);
	void ViewerWidgetWheel(ViewerWidget* w, QEvent* event);

	//ImageViewer Events
	void closeEvent(QCloseEvent* event);

	//Image functions
	bool openImage(QString filename);
	bool saveImage(QString filename);

private slots:
	void on_actionOpen_triggered();
	void on_actionSave_as_triggered();
	void on_actionClear_triggered();
	void on_actionExit_triggered();

	//Tools slots
	void on_pushButtonSetColor_clicked();
	void on_pushButtonRotate_clicked();
	void on_pushButtonMirror_clicked();
	void on_pushButtonScale_clicked();
	void on_pushButtonShear_clicked();
	void on_pushButtonClear_clicked();
	void on_horizontalSlider_azimut_valueChanged(int value);
	void on_horizontalSlider_Zenit_valueChanged(int value);
	void on_pushButtonColorCube_clicked();
	void on_pushButtonColorModel_clicked();

	//phong
	void on_dsbX_valueChanged(double x);
	void on_dsbY_valueChanged(double y);
	void on_dsbZ_valueChanged(double z);

	void on_sbR_valueChanged(int r);
	void on_sbG_valueChanged(int g);
	void on_sbB_valueChanged(int b);

	void on_sbAR_valueChanged(int r);
	void on_sbAG_valueChanged(int g);
	void on_sbAB_valueChanged(int b);

	void on_doubleSpinBox_DR_valueChanged(double r);
	void on_doubleSpinBox_DG_valueChanged(double g);
	void on_doubleSpinBox_DB_valueChanged(double b);

	void on_doubleSpinBox_RR_valueChanged(double r);
	void on_doubleSpinBox_RG_valueChanged(double g);
	void on_doubleSpinBox_RB_valueChanged(double b);

	void on_doubleSpinBox_AR_valueChanged(double r);
	void on_doubleSpinBox_AG_valueChanged(double g);
	void on_doubleSpinBox_AB_valueChanged(double b);

	void on_doubleSpinBox_Ostrost_valueChanged(double o);










};
