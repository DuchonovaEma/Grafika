#include "ImageViewer.h"


ImageViewer::ImageViewer(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::ImageViewerClass)
{
	ui->setupUi(this);
	vW = new ViewerWidget(QSize(500, 500), ui->scrollArea);
	ui->scrollArea->setWidget(vW);

	ui->scrollArea->setBackgroundRole(QPalette::Dark);
	ui->scrollArea->setWidgetResizable(false);
	ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	vW->setObjectName("ViewerWidget");
	vW->installEventFilter(this);

	globalColor = Qt::blue;
	QString style_sheet = QString("background-color: %1;").arg(globalColor.name(QColor::HexRgb));
	ui->pushButtonSetColor->setStyleSheet(style_sheet);
}

// Event filters
bool ImageViewer::eventFilter(QObject* obj, QEvent* event)
{
	if (obj->objectName() == "ViewerWidget") {
		return ViewerWidgetEventFilter(obj, event);
	}
	return QMainWindow::eventFilter(obj, event);
}

//ViewerWidget Events
bool ImageViewer::ViewerWidgetEventFilter(QObject* obj, QEvent* event)
{
	ViewerWidget* w = static_cast<ViewerWidget*>(obj);

	if (!w) {
		return false;
	}

	if (event->type() == QEvent::MouseButtonPress) {
		ViewerWidgetMouseButtonPress(w, event);
	}
	else if (event->type() == QEvent::MouseButtonRelease) {
		ViewerWidgetMouseButtonRelease(w, event);
	}
	else if (event->type() == QEvent::MouseMove) {
		ViewerWidgetMouseMove(w, event);
	}
	else if (event->type() == QEvent::Leave) {
		ViewerWidgetLeave(w, event);
	}
	else if (event->type() == QEvent::Enter) {
		ViewerWidgetEnter(w, event);
	}
	else if (event->type() == QEvent::Wheel) {
		ViewerWidgetWheel(w, event);
	}

	return QObject::eventFilter(obj, event);
}
void ImageViewer::ViewerWidgetMouseButtonPress(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);


	if (e->button() == Qt::LeftButton &&
		!ui->toolButtonDrawLine->isChecked() &&
		!ui->toolButtonDrawCircle->isChecked() &&
		!ui->toolButtonDrawPolygon->isChecked()) {

		// Ak existuje nejaký objekt
		if (w->getDrawLineBegin() != QPoint() ||
			w->getPolygonPoints().size() >= 3 ||
			w->getCircleRadius() > 0) {  
			w->setIsDragging(true);              
			w->setLastMousePos(e->pos());    
		}

	}


	//line
	if (e->button() == Qt::LeftButton && ui->toolButtonDrawLine->isChecked())
	{
		if (w->getDrawLineActivated()) {
			w->drawLine(w->getDrawLineBegin(), e->pos(), globalColor, ui->comboBoxLineAlg->currentIndex());
			w->setLineEnd(e->pos());
			w->setDrawLineActivated(false);
		}
		else {
			w->setDrawLineBegin(e->pos());
			w->setDrawLineActivated(true);
			w->setPixel(e->pos().x(), e->pos().y(), globalColor);
			w->update();
		}
	}

	//circle
	if (e->button() == Qt::LeftButton && ui->toolButtonDrawCircle->isChecked())
	{
		if (w->getDrawCircleActivated()) {
			w->drawCircle(w->getDrawCircleBegin(), e->pos(), globalColor);
			w->setDrawCircleActivated(false);
			w->update();
		}
		else {
			w->setDrawCircleBegin(e->pos());
			w->setDrawCircleActivated(true);
			w->setPixel(e->pos().x(), e->pos().y(), globalColor);
			w->update();
		}
	}

	// PRE POLYGÓN
	if (e->button() == Qt::LeftButton && ui->toolButtonDrawPolygon->isChecked())
	{
		w->addPolygonPoint(e->pos());
		w->setPixel(e->pos().x(), e->pos().y(), globalColor);
		w->setDrawPolygonActivated(true);

		QVector<QPoint> points = w->getPolygonPoints();
		if (points.size() >= 2) {
			w->drawLine(points[points.size() - 2], points[points.size() - 1],
				globalColor, ui->comboBoxLineAlg->currentIndex());
		}

		w->update();
	}
	else if (e->button() == Qt::RightButton && ui->toolButtonDrawPolygon->isChecked())
	{
		// ukoncenie
		if (w->getDrawPolygonActivated()) {
			w->finishPolygon(globalColor, ui->comboBoxLineAlg->currentIndex());
			ui->toolButtonDrawPolygon->setChecked(false);
		}
	}

	if (e->button() == Qt::LeftButton &&
		!ui->toolButtonDrawLine->isChecked() &&
		!ui->toolButtonDrawCircle->isChecked() &&
		!ui->toolButtonDrawPolygon->isChecked())
	{
		if (w->isPointInsidePolygon(e->pos())) {
			if (w->getPolygonPoints().size() == 3) {
				QColor col0(255, 0, 0);   // red
				QColor col1(0, 255, 0);   // green
				QColor col2(0, 0, 255);   // blue
				QVector<QPoint> pts = w->getPolygonPoints();
				w->fillTriangleBarycentric(pts[0], pts[1], pts[2],
					col0, col1, col2);
			}
			else {
				w->fillPolygonScanline(globalColor);
			}
		}
	}

	if (e->button() == Qt::LeftButton && ui->toolButtonBezier->isChecked())
	{
		w->addBezierPoint(e->pos());
	}
	else if (e->button() == Qt::RightButton && ui->toolButtonBezier->isChecked())
	{
		
		if (w->getBezierPoints().size() >= 2) {
			w->drawBezierCurve();
		}
		else {
			QMessageBox::warning(this, "Bezier", "Potrebujete aspoò 2 riadiace body!");
		}
		ui->toolButtonBezier->setChecked(false);
	}

}


void ImageViewer::ViewerWidgetMouseButtonRelease(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);

	if (e->button() == Qt::LeftButton) {
		w->setIsDragging(false);
	}
}
void ImageViewer::ViewerWidgetMouseMove(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);

	if (w->getIsDragging()) {
		int dx = e->pos().x() - w->getLastMousePos().x();
		int dy = e->pos().y() - w->getLastMousePos().y();

		w->moveObject(dx, dy, ui->comboBoxLineAlg->currentIndex(), globalColor);
		w->setLastMousePos(e->pos());
	}
}
void ImageViewer::ViewerWidgetLeave(ViewerWidget* w, QEvent* event)
{
}
void ImageViewer::ViewerWidgetEnter(ViewerWidget* w, QEvent* event)
{
}
void ImageViewer::ViewerWidgetWheel(ViewerWidget* w, QEvent* event)
{
	QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);

	double scaleFactor;
	if (wheelEvent->angleDelta().y() > 0) {
		scaleFactor = 1.25;
	}
	else {
		scaleFactor = 0.75;
	}
	w->scale(scaleFactor, scaleFactor, ui->comboBoxLineAlg->currentIndex(), globalColor);

}

//ImageViewer Events
void ImageViewer::closeEvent(QCloseEvent* event)
{
	if (QMessageBox::Yes == QMessageBox::question(this, "Close Confirmation", "Are you sure you want to exit?", QMessageBox::Yes | QMessageBox::No))
	{
		event->accept();
	}
	else {
		event->ignore();
	}
}

//Image functions
bool ImageViewer::openImage(QString filename)
{
	QImage loadedImg(filename);
	if (!loadedImg.isNull()) {
		return vW->setImage(loadedImg);
	}
	return false;
}
bool ImageViewer::saveImage(QString filename)
{
	QFileInfo fi(filename);
	QString extension = fi.completeSuffix();

	QImage* img = vW->getImage();
	return img->save(filename, extension.toStdString().c_str());
}

//Slots
void ImageViewer::on_actionOpen_triggered()
{
	QString folder = settings.value("folder_img_load_path", "").toString();

	QString fileFilter = "Image data (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm *.xbm *.xpm);;All files (*)";
	QString fileName = QFileDialog::getOpenFileName(this, "Load image", folder, fileFilter);
	if (fileName.isEmpty()) { return; }

	QFileInfo fi(fileName);
	settings.setValue("folder_img_load_path", fi.absoluteDir().absolutePath());


	if (fileName.endsWith(".vtk", Qt::CaseInsensitive)) {
		vW->getWingedEdge()->loadFromVTK(fileName);

		double azimut = ui->horizontalSlider_azimut->value();
		double zenit = ui->horizontalSlider_Zenit->value();
		int project = ui->comboBoxProjection->currentIndex();
		double distance = ui->doubleSpinBoxDistance->value();
		QColor color = globalColor;
		int algType = ui->comboBoxLineAlg->currentIndex();

		vW->drawWingedEdge(0, 0, 0, 0, color, algType);
	}
	else {
		if (!openImage(fileName)) {
			msgBox.setText("Unable to open image.");
			msgBox.setIcon(QMessageBox::Warning);
			msgBox.exec();
		}
	}
}
void ImageViewer::on_actionSave_as_triggered()
{
	QString folder = settings.value("folder_img_save_path", "").toString();

	QString fileFilter = "Image data (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm *.xbm *.xpm);;All files (*)";
	QString fileName = QFileDialog::getSaveFileName(this, "Save image", folder, fileFilter);
	if (!fileName.isEmpty()) {
		QFileInfo fi(fileName);
		settings.setValue("folder_img_save_path", fi.absoluteDir().absolutePath());

		if (!saveImage(fileName)) {
			msgBox.setText("Unable to save image.");
			msgBox.setIcon(QMessageBox::Warning);
		}
		else {
			msgBox.setText(QString("File %1 saved.").arg(fileName));
			msgBox.setIcon(QMessageBox::Information);
		}
		msgBox.exec();
	}
}
void ImageViewer::on_actionClear_triggered()
{
	vW->clearAll();
}
void ImageViewer::on_actionExit_triggered()
{
	this->close();
}

void ImageViewer::on_pushButtonSetColor_clicked()
{
	QColor newColor = QColorDialog::getColor(globalColor, this);
	if (newColor.isValid()) {
		QString style_sheet = QString("background-color: %1;").arg(newColor.name(QColor::HexRgb));
		ui->pushButtonSetColor->setStyleSheet(style_sheet);
		globalColor = newColor;
	}
}

void ImageViewer::on_pushButtonRotate_clicked()
{
	double angle = ui->doubleSpinBoxDegrees->value();
	bool clockwise = ui->checkBoxClockwise->isChecked(); 
	int algType = ui->comboBoxLineAlg->currentIndex();
	QColor color = globalColor; 

	vW->rotate(angle, clockwise, algType, color);
}

void ImageViewer::on_pushButtonMirror_clicked()
{
	QColor color = globalColor;
	int algType = ui->comboBoxLineAlg->currentIndex();

	if (vW->getPolygonPoints().size() >= 3) {
		// polygon
		QVector<QPoint> poly = vW->getPolygonPoints();
		QPoint A = poly[0];
		QPoint B = poly[1];
		vW->mirrorOverLine(A, B, color, algType);
	}
	else if (vW->getDrawLineBegin() != QPoint() && vW->getLineEnd() != QPoint()) {
		//usecka
		QPoint start = vW->getDrawLineBegin();
		QPoint end = vW->getLineEnd();

		QPoint A(start.x() - 100, start.y());  
		QPoint B(start.x() + 100, start.y());  

		vW->mirrorOverLine(A, B, color, algType);
	}
}

void ImageViewer::on_pushButtonScale_clicked()
{
	double scaleX = ui->doubleSpinBoxScaleX->value();
	double scaleY = ui->doubleSpinBoxScaleY->value();

	vW->scale(scaleX, scaleY, ui->comboBoxLineAlg->currentIndex(), globalColor);
}

void ImageViewer::on_pushButtonShear_clicked()
{
	double shearFactor = ui->doubleSpinBoxShear->value();  
	vW->shearX(shearFactor, ui->comboBoxLineAlg->currentIndex(), globalColor);
}

void ImageViewer::on_pushButtonClear_clicked()
{
	on_actionClear_triggered();
}

void ImageViewer::on_horizontalSlider_azimut_valueChanged(int value)
{
	ui->label_AzimutValue->setText(QString::number(value));

	double azimut = value;
	double zenit = ui->horizontalSlider_Zenit->value();
	int project = ui->comboBoxProjection->currentIndex();
	double distance = ui->doubleSpinBoxDistance->value();

	vW->drawWingedEdge(azimut, zenit, project, distance, globalColor, ui->comboBoxLineAlg->currentIndex());
	vW->drawColoredModel(azimut, zenit, project, distance);

}

void ImageViewer::on_horizontalSlider_Zenit_valueChanged(int value)
{
	ui->label_ZenitValue->setText(QString::number(value));

	double azimut = ui->horizontalSlider_azimut->value();
	double zenit = value;
	int project = ui->comboBoxProjection->currentIndex();
	double distance = ui->doubleSpinBoxDistance->value();

	vW->drawWingedEdge(azimut, zenit, project, distance, globalColor, ui->comboBoxLineAlg->currentIndex());
	vW->drawColoredModel(azimut, zenit, project, distance);

}

void ImageViewer::on_pushButtonColorCube_clicked()
{
	if (vW->getWingedEdge()->getFaces().empty()) {
		QMessageBox::warning(this, "Chyba", "Najprv naèítaj VTK kocku!");
		return;
	}
	double azimut = ui->horizontalSlider_azimut->value();
	double zenit = ui->horizontalSlider_Zenit->value();
	int project = ui->comboBoxProjection->currentIndex();
	double distance = ui->doubleSpinBoxDistance->value();

	vW->drawColoredCube(azimut, zenit, project, distance);

}

void ImageViewer::on_pushButtonColorModel_clicked() {

	if (vW->getWingedEdge()->getFaces().empty()) {
		QMessageBox::warning(this, "Chyba", "Najprv naèítaj VTK model!");
		return;
	}

	on_doubleSpinBox_DR_valueChanged(ui->doubleSpinBox_DR->value());
	on_doubleSpinBox_DG_valueChanged(ui->doubleSpinBox_DG->value());
	on_doubleSpinBox_DB_valueChanged(ui->doubleSpinBox_DB->value());

	on_doubleSpinBox_AR_valueChanged(ui->doubleSpinBox_AR->value());
	on_doubleSpinBox_AG_valueChanged(ui->doubleSpinBox_AG->value());
	on_doubleSpinBox_AB_valueChanged(ui->doubleSpinBox_AB->value());

	on_doubleSpinBox_RR_valueChanged(ui->doubleSpinBox_RR->value());
	on_doubleSpinBox_RG_valueChanged(ui->doubleSpinBox_RG->value());
	on_doubleSpinBox_RB_valueChanged(ui->doubleSpinBox_RB->value());

	on_doubleSpinBox_Ostrost_valueChanged(ui->doubleSpinBox_Ostrost->value());

	on_sbR_valueChanged(ui->sbR->value());
	on_sbG_valueChanged(ui->sbG->value());
	on_sbB_valueChanged(ui->sbB->value());

	on_sbAR_valueChanged(ui->sbAR->value());
	on_sbAG_valueChanged(ui->sbAG->value());
	on_sbAB_valueChanged(ui->sbAB->value());

	on_dsbX_valueChanged(ui->dsbX->value());
	on_dsbY_valueChanged(ui->dsbY->value());
	on_dsbZ_valueChanged(ui->dsbZ->value());



	double azimut = ui->horizontalSlider_azimut->value();
	double zenit = ui->horizontalSlider_Zenit->value();
	int project = ui->comboBoxProjection->currentIndex();
	double distance = ui->doubleSpinBoxDistance->value();



	vW->drawColoredModel(azimut, zenit, project, distance);
}

void ImageViewer::on_dsbX_valueChanged(double x)
{
	vW->light.position.setX(x);
	vW->update();
}

void ImageViewer::on_dsbY_valueChanged(double y)
{
	vW->light.position.setY(y);
	vW->update();
}

void ImageViewer::on_dsbZ_valueChanged(double z)
{
	vW->light.position.setZ(-z); //aby sa osvetlovala predna strana
	vW->update();
}

void ImageViewer::on_sbR_valueChanged(int r)
{
	vW->light.color.setRed(r);
	vW->update();
}

void ImageViewer::on_sbG_valueChanged(int g)
{
	vW->light.color.setGreen(g);
	vW->update();
}

void ImageViewer::on_sbB_valueChanged(int b)
{
	vW->light.color.setBlue(b);
	vW->update();
}

void ImageViewer::on_sbAR_valueChanged(int r)
{
	vW->ambientLight.setRed(r);
	vW->update();
}

void ImageViewer::on_sbAG_valueChanged(int g)
{
	vW->ambientLight.setGreen(g);
	vW->update();
}

void ImageViewer::on_sbAB_valueChanged(int b)
{
	vW->ambientLight.setBlue(b);
	vW->update();
}

void ImageViewer::on_doubleSpinBox_DR_valueChanged(double r)
{
	vW->material.diffuseR=r;
	vW->update();
}

void ImageViewer::on_doubleSpinBox_DG_valueChanged(double g)
{
	vW->material.diffuseG=g;
	vW->update();
}

void ImageViewer::on_doubleSpinBox_DB_valueChanged(double b)
{
	vW->material.diffuseB=b;
	vW->update();
}

void ImageViewer::on_doubleSpinBox_RR_valueChanged(double r)
{
	vW->material.specularR=r;
	vW->update();
}

void ImageViewer::on_doubleSpinBox_RG_valueChanged(double g)
{
	vW->material.specularG=g;
	vW->update();
}

void ImageViewer::on_doubleSpinBox_RB_valueChanged(double b)
{
	vW->material.specularB=b;
	vW->update();
}

void ImageViewer::on_doubleSpinBox_AR_valueChanged(double r)
{
	vW->material.ambientR=r;
	vW->update();
}

void ImageViewer::on_doubleSpinBox_AG_valueChanged(double g)
{
	vW->material.ambientG=g;
	vW->update();
}

void ImageViewer::on_doubleSpinBox_AB_valueChanged(double b)
{
	vW->material.ambientB=b;
	vW->update();
}

void ImageViewer::on_doubleSpinBox_Ostrost_valueChanged(double o)
{
	vW->material.ostrost=o;
	vW->update();
}
















