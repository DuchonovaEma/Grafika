#include   "ViewerWidget.h"
#include "WingedEdge.h"


ViewerWidget::ViewerWidget(QSize imgSize, QWidget* parent)
	: QWidget(parent)
{
	setAttribute(Qt::WA_StaticContents);
	setMouseTracking(true);
	if (imgSize != QSize(0, 0)) {
		img = new QImage(imgSize, QImage::Format_ARGB32);
		img->fill(Qt::white);
		resizeWidget(img->size());
		setDataPtr();
	}
}
ViewerWidget::~ViewerWidget()
{
	delete img;
	img = nullptr;
	data = nullptr;
}
void ViewerWidget::resizeWidget(QSize size)
{
	this->resize(size);
	this->setMinimumSize(size);
	this->setMaximumSize(size);
}

//Image functions
bool ViewerWidget::setImage(const QImage& inputImg)
{
	if (img) {
		delete img;
		img = nullptr;
		data = nullptr;
	}
	img = new QImage(inputImg.convertToFormat(QImage::Format_ARGB32));
	if (!img || img->isNull()) {
		return false;
	}
	resizeWidget(img->size());
	setDataPtr();
	update();

	return true;
}
bool ViewerWidget::isEmpty()
{
	if (img == nullptr) {
		return true;
	}

	if (img->size() == QSize(0, 0)) {
		return true;
	}
	return false;
}

bool ViewerWidget::changeSize(int width, int height)
{
	QSize newSize(width, height);

	if (newSize != QSize(0, 0)) {
		if (img != nullptr) {
			delete img;
		}

		img = new QImage(newSize, QImage::Format_ARGB32);
		if (!img || img->isNull()) {
			return false;
		}
		img->fill(Qt::white);
		resizeWidget(img->size());
		setDataPtr();
		update();
	}

	return true;
}

void ViewerWidget::setPixel(int x, int y, int r, int g, int b, int a)
{
	if (!img || !data) return;
	if (!isInside(x, y)) return;

	r = r > 255 ? 255 : (r < 0 ? 0 : r);
	g = g > 255 ? 255 : (g < 0 ? 0 : g);
	b = b > 255 ? 255 : (b < 0 ? 0 : b);
	a = a > 255 ? 255 : (a < 0 ? 0 : a);

	size_t startbyte = y * img->bytesPerLine() + x * 4;
	data[startbyte] = static_cast<uchar>(b);
	data[startbyte + 1] = static_cast<uchar>(g);
	data[startbyte + 2] = static_cast<uchar>(r);
	data[startbyte + 3] = static_cast<uchar>(a);
}
void ViewerWidget::setPixel(int x, int y, double valR, double valG, double valB, double valA)
{
	valR = valR > 1 ? 1 : (valR < 0 ? 0 : valR);
	valG = valG > 1 ? 1 : (valG < 0 ? 0 : valG);
	valB = valB > 1 ? 1 : (valB < 0 ? 0 : valB);
	valA = valA > 1 ? 1 : (valA < 0 ? 0 : valA);

	setPixel(x, y, static_cast<int>(255 * valR + 0.5), static_cast<int>(255 * valG + 0.5), static_cast<int>(255 * valB + 0.5), static_cast<int>(255 * valA + 0.5));
}
void ViewerWidget::setPixel(int x, int y, const QColor& color)
{
	if (color.isValid()) {
		setPixel(x, y, color.red(), color.green(), color.blue(), color.alpha());
	}
}

bool ViewerWidget::isInside(int x, int y)
{
	return img && x >= 0 && y >= 0 && x < img->width() && y < img->height();
}

//Draw functions
void ViewerWidget::drawLine(QPoint start, QPoint end, QColor color, int algType)
{
	if (!img || !data) return;

	if (algType == 0) {
		drawLineDDA(start, end, color);
	}
	else {
		drawLineBresenham(start, end, color);
	}
	update();

	//Po implementovani drawLineDDA a drawLineBresenham treba vymazat
	/*QPainter painter(img);
	painter.setPen(QPen(color));
	painter.drawLine(start, end);
	update();*/
}

void ViewerWidget::finishPolygon(QColor color, int algType)
{
	if (polygonPoints.size() < 3) {
		QMessageBox::warning(nullptr, "Polygón", "Na vytvorenie polygónu potrebujete aspoň 3 body!");
		polygonPoints.clear();
		setDrawPolygonActivated(false);
		return;
	}


	drawLine(polygonPoints.last(), polygonPoints.first(), color, algType);

	setDrawPolygonActivated(false);

	update();
}

void ViewerWidget::clear()
{
	if (!img) return;
	img->fill(Qt::white);
	update();
}
void ViewerWidget::clearAll()
{
	clear();  // vymaže plátno

	// Vymaže všetky dáta
	drawLineActivated = false;
	drawLineBegin = QPoint(0, 0);
	lineEnd = QPoint(0, 0);

	drawCircleActivated = false;
	drawCircleBegin = QPoint(0, 0);
	circleRadius = 0;

	drawPolygonActivated = false;
	polygonPoints.clear();
	bezierPoints.clear();

	isDragging = false;

	update();
}

void ViewerWidget::drawLineDDA(QPoint start, QPoint end, QColor color)
{
	int x1 = start.x();
	int x2 = end.x();
	int y1 = start.y();
	int y2 = end.y();

	int dx = x2 - x1;
	int dy = y2 - y1;

	if (dx == 0 && dy == 0) {
		// jeden bod
		setPixel(x1, y1, color);
		return;
	}



	if (abs(dx) >= abs(dy)) { //riadiaca os x

		if (x1 > x2) {
			std::swap(x1, x2);
			std::swap(y1, y2);
			dx = x2 - x1; 
			dy = y2 - y1;  
		}

		double m = (dx != 0) ? (double)dy / dx : 0;

		double y = y1;
		setPixel(x1, y1, color);
		int x = x1 + 1;

		while (x <= x2) {
			y += m;
			setPixel(x, (int)(y+0.5), color);
			x++;
		}
	}

	else { //riadiaca os y
		if (y1 > y2) {
			std::swap(y1, y2);
			std::swap(x1, x2);
			dx = x2 - x1;  
			dy = y2 - y1;  
		}

		double m = (dy != 0) ? (double)dx / dy : 0;

		double x = x1;
		setPixel(x1, y1, color);
		int y = y1 + 1;

		while (y <= y2) {
			x += m;
			setPixel((int)(x + 0.5), y, color);
			y++;
		}
	}
}

void ViewerWidget::drawLineBresenham(QPoint start, QPoint end, QColor color)
{
	int x1 = start.x();
	int x2 = end.x();
	int y1 = start.y();
	int y2 = end.y();

	int dx = x2 - x1;
	int dy = y2 - y1;

	if (abs(dx) >= abs(dy)) { //riadiaca os x

		if (x1 > x2) {
			std::swap(x1, x2);
			std::swap(y1, y2);
			dx = x2 - x1;
			dy = y2 - y1;
		}

		int x = x1;
		int y = y1;
		int p, k2, k1;

		if(dy >= 0){ //0<m<1
			 k1 = 2 * dy;       
			 k2 = 2 * dy - 2 * dx; 
			 p = 2 * dy - dx;     

			setPixel(x, y, color);

			while (x < x2) {
				x++;
				if (p > 0) {         
					y++;             
					p += k2;         
				}
				else {
					p += k1;        
				}
				setPixel(x, y, color);
			}
			
		}


		else { //-1<m<0
			k1 = 2 * dy;        
			k2 = 2 * dy + 2 * dx; 
			p = 2 * dy + dx;    

			setPixel(x, y, color);

			while (x < x2) {
				x++;
				if (p < 0) {         
					y--;             
					p += k2;        
				}
				else {
					p += k1;         
				}
				setPixel(x, y, color);
			}
		}

	} //koniec x

	else {//riadiaca os y
		
		if (y1 > y2) {
			std::swap(y1, y2);
			std::swap(x1, x2);
			dx = x2 - x1;
			dy = y2 - y1;
		}

		int x = x1;
		int y = y1;
		int p,k2,k1;

		

		if (dx >= 0) {//m>1
			 k1 = 2 * dx;
			 k2 = 2 * (dx - dy);   
			 p = 2 * dx - dy;

			setPixel(x, y, color);

			while (y < y2) {
				y++;

				if (p > 0) {
					x++;
					p += k2;
				}
				else {
					p += k1;
				}
				setPixel(x, y, color);
			}
		}

		else {
			 k1 = 2 * dx;
			 k2 = 2 * (dx + dy);   
			 p = 2 * dx + dy;

			setPixel(x, y, color);
			
			while (y < y2) {
				y++;
				

				if (p < 0) {
					x--;
					p += k2;
				}
				else {
					p += k1;
				}
				setPixel(x, y, color);
			}

		}


	}

}
void ViewerWidget::drawCircle(QPoint start, QPoint end, QColor color) {

	int cx = start.x();  
	int cy = start.y();
	int rx = end.x();
	int ry = end.y();


	int dx = rx - cx;
	int dy = ry - cy;
	int r = sqrt(dx * dx + dy * dy)+0.5;

	
	setDrawCircleBegin(start);
	setCircleRadius(r);

	int p = 1 - r;
	int x = 0;
	int y = r;
	int dvaX = 3;
	int dvaY = (2 * r) - 2;

	setPixel(cx + x, cy + y, color);
	setPixel(cx - x, cy + y, color);
	setPixel(cx + x, cy - y, color);
	setPixel(cx - x, cy - y, color);
	setPixel(cx + y, cy + x, color);
	setPixel(cx - y, cy + x, color);
	setPixel(cx + y, cy - x, color);
	setPixel(cx - y, cy - x, color);

	while (x <= y) {

		if (p > 0) {
			p = p - dvaY;
			y--;
			dvaY = dvaY - 2;
		}
		p = p + dvaX;
		dvaX = dvaX + 2;
		x++;

		setPixel(cx + x, cy + y, color);
		setPixel(cx - x, cy + y, color);
		setPixel(cx + x, cy - y, color);
		setPixel(cx - x, cy - y, color);
		setPixel(cx + y, cy + x, color);
		setPixel(cx - y, cy + x, color);
		setPixel(cx + y, cy - x, color);
		setPixel(cx - y, cy - x, color);
	}

}

void ViewerWidget::drawPolygon(QVector<QPoint> points, QColor color, int algType)
{
	if (points.size() < 2) return;

	for (int i = 0; i < points.size() - 1; i++) {
		drawLine(points[i], points[i + 1], color, algType);
	}
	if (points.size() >= 3 && points.first() != points.last()) {
		drawLine(points.last(), points.first(), color, algType); 
	}
}

void ViewerWidget::rotate(double angle, bool clockwise, int algType, QColor color)
{
	double rad = angle * M_PI / 180.0;
	double cosA = cos(rad);
	double sinA = sin(rad);

	// LINE
	if (getDrawLineActivated() == false && getDrawLineBegin() != QPoint() && lineEnd != QPoint()) {
		QPoint center = getDrawLineBegin();
		QPoint end = getLineEnd();

		int x = end.x() - center.x();
		int y = end.y() - center.y();

		int xNew, yNew;
		if (clockwise) {
			// V smere hodinových ručičiek 
			xNew = round(x * cosA + y * sinA);
			yNew = round(-x * sinA + y * cosA);
		}
		else {
			// Proti smeru 
			xNew = round(x * cosA - y * sinA);
			yNew = round(x * sinA + y * cosA);
		}

		lineEnd = QPoint(center.x() + xNew, center.y() + yNew);
		clear();
		drawLine(getDrawLineBegin(), lineEnd, color, algType);
	}
	// POLYGÓN
	else if (polygonPoints.size() >= 3) {
		QPoint center = polygonPoints.first();
		QVector<QPoint> rotatedPoints;
		rotatedPoints.append(center);

		for (int i = 1; i < polygonPoints.size(); i++) {
			const QPoint& p = polygonPoints[i];
			int x = p.x() - center.x();
			int y = p.y() - center.y();

			int xNew, yNew;
			if (clockwise) {
				
				xNew = round(x * cosA - y * sinA);   
				yNew = round(x * sinA + y * cosA);
			}
			else {
				
				xNew = round(x * cosA + y * sinA);     
				yNew = round(-x * sinA + y * cosA);
			}

			rotatedPoints.append(QPoint(center.x() + xNew, center.y() + yNew));
		}

		clear();
		polygonPoints = rotatedPoints;
		drawPolygon(polygonPoints, color, algType);
	}

	applyClippingForCurrentObject(algType, color);
}

void ViewerWidget::mirrorOverLine(QPoint A, QPoint B, QColor color, int algType)
{
	if (polygonPoints.isEmpty() && (getDrawLineBegin() == QPoint() || lineEnd == QPoint())) return;

	// Smerový vektor úsečky AB
	int u = B.x() - A.x();
	int v = B.y() - A.y();

	int a = v;
	int b = -u;

	int c = -a * A.x() - b * A.y();

	int denominator = a * a + b * b;
	if (denominator == 0) return;

	// PRE ÚSEČKU
	if (getDrawLineActivated() == false && getDrawLineBegin() != QPoint() && lineEnd != QPoint()) {
		QPoint center = getDrawLineBegin();
		QPoint end = lineEnd;


		QPoint newBegin = mirrorPoint(center, a, b, c, denominator);
		QPoint newEnd = mirrorPoint(end, a, b, c, denominator);

		setDrawLineBegin(newBegin);
		setLineEnd(newEnd);

		clear();
		drawLine(getDrawLineBegin(), getLineEnd(), color, algType);
	}
	// PRE POLYGÓN
	else if (polygonPoints.size() >= 3) {
		QVector<QPoint> mirroredPoints;

		for (const QPoint& p : polygonPoints) {
			mirroredPoints.append(mirrorPoint(p, a, b, c, denominator));
		}

		polygonPoints = mirroredPoints;

		clear();
		drawPolygon(polygonPoints, color, algType);
	}

	applyClippingForCurrentObject(algType, color);

}

QPoint ViewerWidget::mirrorPoint(QPoint p, int a, int b, int c, int denominator)
{
	double d = (a * p.x() + b * p.y() + c) / (double)denominator;
	int xNew = round(p.x() - 2 * d * a);
	int yNew = round(p.y() - 2 * d * b);

	return QPoint(xNew, yNew);
}

void ViewerWidget::scale(double scaleX, double scaleY, int algType, QColor color)
{
	// KRUŽNICA
	if (getDrawCircleActivated() == false && circleRadius > 0) {
		double avgScale = (scaleX + scaleY) / 2.0;
		int newRadius = round(circleRadius * avgScale);

		QPoint center = getDrawCircleBegin();

		clear();
		drawCircle(center, QPoint(center.x() + newRadius, center.y()), color);
		return;
	}

	// LINE
	if (getDrawLineActivated() == false && getDrawLineBegin() != QPoint() && lineEnd != QPoint()) {
		QPoint center = getDrawLineBegin();  // prvý bod je stred

		QPoint end = lineEnd;
		int x = end.x() - center.x();
		int y = end.y() - center.y();

		int xNew = round(center.x() + x * scaleX);
		int yNew = round(center.y() + y * scaleY);

		setLineEnd(QPoint(xNew, yNew));

		clear();
		drawLine(getDrawLineBegin(), getLineEnd(), color, algType);
	}
	// POLYGÓN
	else if (polygonPoints.size() >= 3) {
		QPoint center = polygonPoints.first();
		QVector<QPoint> scaledPoints;
		scaledPoints.append(center);

		for (int i = 1; i < polygonPoints.size(); i++) {
			const QPoint& p = polygonPoints[i];
			int x = p.x() - center.x();
			int y = p.y() - center.y();

			int xNew = round(center.x() + x * scaleX);
			int yNew = round(center.y() + y * scaleY);

			scaledPoints.append(QPoint(xNew, yNew));
		}

		polygonPoints = scaledPoints;

		clear();
		drawPolygon(polygonPoints, color, algType);
	}

	applyClippingForCurrentObject(algType, color);

}

void ViewerWidget::shearX(double shearFactor, int algType, QColor color)
{
	// KRUŽNICA 
	if (getDrawCircleActivated() == false && circleRadius > 0) {
		return;  
	}
	// POLYGÓN
	else if (polygonPoints.size() >= 3) {
		QPoint center = polygonPoints.first();  // prvý bod je stred
		QVector<QPoint> shearedPoints;
		shearedPoints.append(center);  // prvý bod ostáva

		for (int i = 1; i < polygonPoints.size(); i++) {
			const QPoint& p = polygonPoints[i];
			int x = p.x() - center.x();
			int y = p.y() - center.y();

			
			int xNew = round(center.x() + x + y * shearFactor);
			int yNew = p.y();  

			shearedPoints.append(QPoint(xNew, yNew));
		}

		polygonPoints = shearedPoints;
		clear();
		drawPolygon(polygonPoints, color, algType);
	}
	// LINE
	else if (polygonPoints.size() < 3 && getDrawLineActivated() == false && getDrawLineBegin() != QPoint() && lineEnd != QPoint()) {
		QPoint center = getDrawLineBegin();
		QPoint end = lineEnd;

		int x = end.x() - center.x();
		int y = end.y() - center.y();

		// Skosenie koncového bodu
		int xNew = round(center.x() + x + y * shearFactor);
		int yNew = end.y(); 

		setLineEnd(QPoint(xNew, yNew));
		clear();
		drawLine(getDrawLineBegin(), getLineEnd(), color, algType);
	}

	applyClippingForCurrentObject(algType, color);

}

void ViewerWidget::moveObject(int dx, int dy, int algType, QColor color)
{
	// POLYGÓN
	if (polygonPoints.size() >= 3) {
		for (int i = 0; i < polygonPoints.size(); i++) {
			polygonPoints[i] = QPoint(polygonPoints[i].x() + dx, polygonPoints[i].y() + dy);
		}
		clear();
		drawPolygon(polygonPoints, color, algType);
	}
	// LINE
	else if (getDrawLineBegin() != QPoint() && lineEnd != QPoint()) {
		setDrawLineBegin(QPoint(getDrawLineBegin().x() + dx, getDrawLineBegin().y() + dy));
		setLineEnd(QPoint(lineEnd.x() + dx, lineEnd.y() + dy));
		clear();
		drawLine(getDrawLineBegin(), lineEnd, color, algType);
	}
	// CIRCLE
	else if (circleRadius > 0) {
		QPoint center = getDrawCircleBegin();  
		QPoint newCenter(center.x() + dx, center.y() + dy);
		setDrawCircleBegin(newCenter);
		clear();
		drawCircle(newCenter, QPoint(newCenter.x() + circleRadius, newCenter.y()), color);
	}
	applyClippingForCurrentObject(algType, color);

}

bool ViewerWidget::clipLineCyrusBeck(QPoint& P1, QPoint& P2, int xmin, int ymin, int xmax, int ymax)
{
	double tL = 0.0;
	double tU = 1.0;
	QPoint d = P2 - P1;
	QPoint P1orig = P1;

	
	QVector<QPoint> E = {
		QPoint(xmin, ymin),  
		QPoint(xmax, ymin), 
		QPoint(xmax, ymax),  
		QPoint(xmin, ymax)  
	};

	
	QVector<QPoint> normals = {
		QPoint(1, 0), 
		QPoint(0, 1),  
		QPoint(-1, 0),  
		QPoint(0, -1)   
	};

	for (int i = 0; i < 4; i++) {
		QPoint n = normals[i];
		QPoint w = P1 - E[i];

		double dn = d.x() * n.x() + d.y() * n.y();
		double wn = w.x() * n.x() + w.y() * n.y();

		if (dn != 0) {
			double t = -wn / dn;

			if (dn > 0) {
				if (t > tL) tL = t;
			}
			else {
				if (t < tU) tU = t;
			}
		}
	}

	if (tL > tU) return false;

	// Orezanie
	if (tL > 0.0) {
		int xNew = (int)(P1orig.x() + d.x() * tL + 0.5);
		int yNew = (int)(P1orig.y() + d.y() * tL + 0.5);
		P1 = QPoint(xNew, yNew);
	}
	if (tU < 1.0) {
		int xNew = (int)(P1orig.x() + d.x() * tU + 0.5);
		int yNew = (int)(P1orig.y() + d.y() * tU + 0.5);
		P2 = QPoint(xNew, yNew);
	}

	return true;

}


QVector<QPoint> ViewerWidget::clipPolygonSutherlandHodgman(QVector<QPoint> polygon, int xmin, int ymin, int xmax, int ymax)
{
	QVector<QPoint> output = polygon;

	//  polygón celý vnútri
	bool allInside = true;
	for (QPoint p : polygon) {
		if (p.x() < xmin || p.x() > xmax || p.y() < ymin || p.y() > ymax) {
			allInside = false;
			break;
		}
	}
	if (allInside) return polygon;

	
	for (int edge = 0; edge < 4; edge++) {
		QVector<QPoint> input = output;
		output.clear();
		if (input.isEmpty()) continue;

		QPoint S = input.last();

		for (int i = 0; i < input.size(); i++) {
			QPoint P = input[i];

			bool P_inside, S_inside;
			switch (edge) {
			case 0: P_inside = P.x() >= xmin; S_inside = S.x() >= xmin; break;
			case 1: P_inside = P.y() >= ymin; S_inside = S.y() >= ymin; break;
			case 2: P_inside = P.x() <= xmax; S_inside = S.x() <= xmax; break;
			case 3: P_inside = P.y() <= ymax; S_inside = S.y() <= ymax; break;
			}

			if (P_inside) {
				if (S_inside) {
					output.append(P);
				}
				else {
					double t;
					QPoint I;
					if (edge == 0 || edge == 2) {
						int x = (edge == 0) ? xmin : xmax;
						t = (double)(x - S.x()) / (P.x() - S.x());
						I = QPoint(x, round(S.y() + t * (P.y() - S.y())));
					}
					else {
						int y = (edge == 1) ? ymin : ymax;
						t = (double)(y - S.y()) / (P.y() - S.y());
						I = QPoint(round(S.x() + t * (P.x() - S.x())), y);
					}
					output.append(I);
					output.append(P);
				}
			}
			else {
				if (S_inside) {
					double t;
					QPoint I;
					if (edge == 0 || edge == 2) {
						int x = (edge == 0) ? xmin : xmax;
						t = (double)(x - S.x()) / (P.x() - S.x());
						I = QPoint(x, round(S.y() + t * (P.y() - S.y())));
					}
					else {
						int y = (edge == 1) ? ymin : ymax;
						t = (double)(y - S.y()) / (P.y() - S.y());
						I = QPoint(round(S.x() + t * (P.x() - S.x())), y);
					}
					output.append(I);
				}
			}
			S = P;
		}
	}

	return output;
}

void ViewerWidget::applyClippingForCurrentObject(int algType, QColor color)
{
	int offset = 50;
	int xmin = offset;
	int ymin = offset;
	int xmax = this->width() - offset;
	int ymax = this->height() - offset;

	// LINE
	if (getDrawLineActivated() == false && getDrawLineBegin() != QPoint() && lineEnd != QPoint()) {
		QPoint P1 = getDrawLineBegin();
		QPoint P2 = lineEnd;

		clear();

		if (clipLineCyrusBeck(P1, P2, xmin, ymin, xmax, ymax)) {
			drawLine(P1, P2, color, algType);
		}

		update();
	}
	// POLYGÓN 
	else if (polygonPoints.size() >= 3) {
		QVector<QPoint> clipped = clipPolygonSutherlandHodgman(polygonPoints, xmin, ymin, xmax, ymax);

		clear();
		if (clipped.size() >= 3) {
			drawPolygon(clipped, color, algType);
		}

		update();
	}
}


void ViewerWidget::fillPolygonScanline(QColor color)
{
	if (polygonPoints.size() < 3) return;

	struct Edge {
		int yz;     
		int dy;     
		double x;   
		double w; 
	};

	QVector<Edge> edges;
	int ymin = INT_MAX, ymax = INT_MIN;

	for (int i = 0; i < polygonPoints.size(); ++i) {
		QPoint p1 = polygonPoints[i];
		QPoint p2 = polygonPoints[(i + 1) % polygonPoints.size()];

		if (p1.y() == p2.y()) continue;

		if (p1.y() > p2.y()) { std::swap(p1, p2); }

		int yz = p1.y();
		int yk = p2.y() - 1;   

		if (yz > yk) continue;

		int dy = yk - yz;
		double xz = p1.x();
		double m = (double)(p2.x() - p1.x()) / (p2.y() - p1.y());
	
		double w = m;


		edges.append({ yz, dy, xz, w });

		if (yz < ymin) ymin = yz;
		if (yk > ymax) ymax = yk;
	}

	if (edges.isEmpty()) return;

	
	std::sort(edges.begin(), edges.end(),
		[](const Edge& a, const Edge& b) { return a.yz < b.yz; });

	
	int tableSize = ymax - ymin + 1;
	QVector<QVector<Edge>> TH(tableSize);

	for (const Edge& e : edges) {
		int i = e.yz - ymin;
		if (i >= 0 && i < tableSize) {
			TH[i].append(e);
		}
	}

	
	QVector<Edge> ZAH;
	int y = ymin;

	for (int i = 0; i < tableSize; ++i) {
		
		for (const Edge& e : TH[i]) {
			ZAH.append(e);
		}

		
		std::sort(ZAH.begin(), ZAH.end(),
			[](const Edge& a, const Edge& b) { return a.x < b.x; });

		
		for (int j = 0; j < ZAH.size() - 1; j += 2) {
			int xStart = (int)ceil(ZAH[j].x);
			int xEnd = (int)floor(ZAH[j + 1].x);
			if (xStart <= xEnd) {
				for (int x = xStart; x <= xEnd; ++x) {
					setPixel(x, y, color);
				}
			}
		}

		
		for (int j = ZAH.size() - 1; j >= 0; --j) {
			if (ZAH[j].dy == 0) {
				ZAH.removeAt(j);
			}
		}

		
		for (Edge& e : ZAH) {
			e.dy--;
			e.x += e.w;
		}

		++y;
	}

	update();
}


bool ViewerWidget::isPointInsidePolygon(QPoint point)
{
	if (polygonPoints.size() < 3) return false;

	bool inside = false;
	int n = polygonPoints.size();
	for (int i = 0, j = n - 1; i < n; j = i++) {
		QPoint pi = polygonPoints[i];
		QPoint pj = polygonPoints[j];

		bool intersect = ((pi.y() > point.y()) != (pj.y() > point.y())) &&
			(point.x() < static_cast<double>(pj.x() - pi.x()) * static_cast<double>(point.y() - pi.y()) / static_cast<double>(pj.y() - pi.y()) + pi.x());
		if (intersect) inside = !inside;
	}
	return inside;
}
void ViewerWidget::fillTriangleBarycentric(QPoint p0, QPoint p1, QPoint p2,
	QColor c0, QColor c1, QColor c2)
{
	
	
	if (p0.y() > p1.y()) { std::swap(p0, p1); std::swap(c0, c1); }
	if (p0.y() > p2.y()) { std::swap(p0, p2); std::swap(c0, c2); }
	if (p1.y() > p2.y()) { std::swap(p1, p2); std::swap(c1, c2); }

	
	double area = abs((p1.x() - p0.x()) * (p2.y() - p0.y()) - (p2.x() - p0.x()) * (p1.y() - p0.y())) / 2.0;
	if (area <= 0) return;

	
	int yStart = p0.y();
	int yEnd = p2.y();

	for (int y = yStart; y <= yEnd; y++) {
		int xStart, xEnd;

		if (y < p1.y()) {
			xStart = p0.x() + (double)(y - p0.y()) * (p1.x() - p0.x()) / (p1.y() - p0.y()) + 0.5;
			xEnd = p0.x() + (double)(y - p0.y()) * (p2.x() - p0.x()) / (p2.y() - p0.y()) + 0.5;
		}
		else {
			xStart = p1.x() + (double)(y - p1.y()) * (p2.x() - p1.x()) / (p2.y() - p1.y()) + 0.5;
			xEnd = p0.x() + (double)(y - p0.y()) * (p2.x() - p0.x()) / (p2.y() - p0.y()) + 0.5;
		}

		if (xStart > xEnd) std::swap(xStart, xEnd);


		for (int x = xStart; x <= xEnd; x++) {
			double w0 = abs((p1.x() - x) * (p2.y() - y) - (p2.x() - x) * (p1.y() - y)) / 2.0;
			double w1 = abs((p0.x() - x) * (p2.y() - y) - (p2.x() - x) * (p0.y() - y)) / 2.0;
			double w2 = abs((p0.x() - x) * (p1.y() - y) - (p1.x() - x) * (p0.y() - y)) / 2.0;

			w0 /= area;
			w1 /= area;
			w2 /= area;

			int r = c0.red() * w0 + c1.red() * w1 + c2.red() * w2 + 0.5;
			int g = c0.green() * w0 + c1.green() * w1 + c2.green() * w2 + 0.5;
			int b = c0.blue() * w0 + c1.blue() * w1 + c2.blue() * w2 + 0.5;

			setPixel(x, y, QColor(r, g, b));
		}
	}
	update();
}

double ViewerWidget::triangleArea(QPoint a, QPoint b, QPoint c)
{
	double val = (a.x() * (b.y() - c.y()) +
		b.x() * (c.y() - a.y()) +
		c.x() * (a.y() - b.y())) / 2.0;
	return fabs(val);
}

void ViewerWidget::addBezierPoint(QPoint p)
{
	bezierPoints.append(p);
	setPixel(p.x(), p.y(), Qt::red);
	update();
}

void ViewerWidget::drawBezierCurve()
{
	if (bezierPoints.size() < 2) return;

	int n = bezierPoints.size();
	const double dt = 1.0 / bezierSegments;

	// Počiatočný bod
	QPoint Q0 = bezierPoints[0];

	for (double t = dt; t < 1.0; t += dt) {
		QVector<QPoint> points = bezierPoints;

		for (int r = 1; r < n; ++r) {
			for (int i = 0; i < n - r; ++i) {
				int x = (1 - t) * points[i].x() + t * points[i + 1].x();
				int y = (1 - t) * points[i].y() + t * points[i + 1].y();
				points[i] = QPoint(round(x), round(y));
			}
		}

		QPoint Q1 = points[0];
		drawLine(Q0, Q1, Qt::blue, 0);

		Q0 = Q1;
	}

	drawLine(Q0, bezierPoints.last(), Qt::blue, 0);

	//aby bolo vidno
	for (const QPoint& p : bezierPoints) {
		setPixel(p.x(), p.y(), Qt::red);
	}

	update();
}

//Slots
void ViewerWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	if (!img || img->isNull()) return;

	QRect area = event->rect();
	painter.drawImage(area, *img, area);
}