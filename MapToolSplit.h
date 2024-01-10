#pragma once

#include<qgsmaptool.h>
#include<qgspointxy.h>
#include<qgsmapmouseevent.h>
#include<qgsvectorlayer.h>
#include<qgsmapcanvas.h>
#include<qmessagebox.h>
#include<qapplication.h>
#include<qgsrubberband.h>

class MapToolSplit:public QgsMapTool
{
	Q_OBJECT

public:
	MapToolSplit(QgsMapCanvas* canvas);
	~MapToolSplit();

	void canvasPressEvent(QgsMapMouseEvent* e) override;
	void canvasMoveEvent(QgsMapMouseEvent* e) override;

private:
	QgsVectorLayer* mLayer = nullptr;
	QgsFeatureList mSelectedFeatures;
	QgsRubberBand* mpCutiingLine;
	bool mIsMoving = false;
	bool mIsCutting = false;
	QCursor mCursor;
	QgsPointXY mStartPointMapCoords;
};