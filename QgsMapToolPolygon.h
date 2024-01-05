#pragma once

#include <QObject>
#include<qgsmaptool.h>

class QgsMapToolPolygon  :  public QgsMapTool
{
	Q_OBJECT

public slots:
	void canvasPressEvent(QgsMapMouseEvent *e) override;
	//void canvasMoveEvent(QgsMapMouseEvent *e) override;
	//void canvasReleaseEvent(QgsMapMouseEvent *e) override;

public:
	QgsMapToolPolygon(QgsMapCanvas* mapCanvas);
	~QgsMapToolPolygon();

	enum class State
	{

	};

private:

};
