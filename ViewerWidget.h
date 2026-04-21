#pragma once
#include <QtWidgets>
#include "WingedEdge.h"
class ViewerWidget :public QWidget {
	Q_OBJECT
private:
	QSize areaSize = QSize(0, 0);
	QImage* img = nullptr;
	uchar* data = nullptr;

	bool drawLineActivated = false;
	QPoint drawLineBegin = QPoint(0, 0);
	QPoint lineEnd= QPoint(0, 0);


	QPoint drawCircleBegin;
	bool drawCircleActivated = false;

	bool drawPolygonActivated = false;
	QVector<QPoint> polygonPoints;
	
     
	int circleRadius = 0;

	bool isDragging = false;
	QPoint lastMousePos;

	QVector<QPoint> bezierPoints;          
	bool bezierModeActive = false;
	int bezierSegments = 100;

	WingedEdge* wingedEdge = nullptr;

public:
	ViewerWidget(QSize imgSize, QWidget* parent = Q_NULLPTR);
	~ViewerWidget();
	void resizeWidget(QSize size);

	WingedEdge* getWingedEdge() {
		if (!wingedEdge) wingedEdge = new WingedEdge();
		return wingedEdge;
	}


	//Image functions
	bool setImage(const QImage& inputImg);
	QImage* getImage() { return img; };
	bool isEmpty();
	bool changeSize(int width, int height);

	void setPixel(int x, int y, int r, int g, int b, int a = 255);
	void setPixel(int x, int y, double valR, double valG, double valB, double valA = 1.);
	void setPixel(int x, int y, const QColor& color);
	bool isInside(int x, int y);

	//Draw functions
	//line
	void drawLine(QPoint start, QPoint end, QColor color, int algType = 0);
	void setDrawLineBegin(QPoint begin) { drawLineBegin = begin; }
	QPoint getDrawLineBegin() { return drawLineBegin; }
	void setLineEnd(QPoint end) { lineEnd = end; }
	QPoint getLineEnd() { return lineEnd; }
	void setDrawLineActivated(bool state) { drawLineActivated = state; }
	bool getDrawLineActivated() { return drawLineActivated; }

	//circle
	void drawCircle(QPoint start, QPoint end, QColor color);
	void setDrawCircleBegin(QPoint begin) { drawCircleBegin = begin; }
	QPoint getDrawCircleBegin() { return drawCircleBegin; }
	void setDrawCircleActivated(bool state) { drawCircleActivated = state; }
	bool getDrawCircleActivated() { return drawCircleActivated; }
	int getCircleRadius() { return circleRadius; }
	void setCircleRadius(int radius) {circleRadius = radius;}
	
	
	//polygon
	void drawPolygon(QVector<QPoint> points, QColor color, int algType);
	void setDrawPolygonActivated(bool state) { drawPolygonActivated = state; }
	bool getDrawPolygonActivated() { return drawPolygonActivated; }
	void addPolygonPoint(QPoint point) { polygonPoints.append(point); }
	void clearPolygonPoints() { polygonPoints.clear(); }
	QVector<QPoint> getPolygonPoints() { return polygonPoints; }
	void finishPolygon(QColor color, int algType);

	//krivky
	void addBezierPoint(QPoint p);
	void drawBezierCurve();
	QVector<QPoint> getBezierPoints() { return bezierPoints; }
	void clearBezierPoints() { bezierPoints.clear(); }

	//Get/Set functions
	uchar* getData() { return data; }
	void setDataPtr() { data = img ? img->bits() : nullptr; }

	int getImgWidth() { return img ? img->width() : 0; };
	int getImgHeight() { return img ? img->height() : 0; };

	bool getIsDragging() { return isDragging; }
	void setIsDragging(bool state) { isDragging = state; }
	void setLastMousePos(QPoint pos) { lastMousePos = pos; }
	QPoint getLastMousePos() { return lastMousePos; }

	void clear();

	void clearAll();

	//Algorithms
	void drawLineDDA(QPoint start, QPoint end, QColor color);
	void drawLineBresenham(QPoint start, QPoint end, QColor color);

	
	void fillPolygonScanline(QColor color);

	bool isPointInsidePolygon(QPoint point);

	void fillTriangleBarycentric(QPoint p0, QPoint p1, QPoint p2, QColor c0, QColor c1, QColor c2);

	double triangleArea(QPoint a, QPoint b, QPoint c);


	//Transform
	void rotate(double angle, bool clockwise, int algType, QColor color);
	void mirrorOverLine(QPoint A, QPoint B, QColor color, int algType);
	QPoint mirrorPoint(QPoint p, int a, int b, int c, int denominator);
	void scale(double scaleX, double scaleY, int algType, QColor color);
	void shearX(double shearFactor, int algType, QColor color);
	void moveObject(int dx, int dy, int algType, QColor color);
	bool clipLineCyrusBeck(QPoint& P1, QPoint& P2, int xmin, int ymin, int xmax, int ymax);

	QVector<QPoint> clipPolygonSutherlandHodgman(QVector<QPoint> polygon, int xmin, int ymin, int xmax, int ymax);

	void applyClippingForCurrentObject(int algType, QColor color);

public slots:
	void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
};