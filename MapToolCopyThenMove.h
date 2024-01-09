#pragma once
#include<qgsmaptool.h>
#include<qgsmapcanvas.h>
#include<qgsvectorlayer.h>
#include<qgsmapmouseevent.h>
#include<qgsapplication.h>
#include<qmessagebox.h>


class MapToolCopyThenMove:public QgsMapTool
{
	Q_OBJECT

public:
	MapToolCopyThenMove(QgsMapCanvas* canvas);
	~MapToolCopyThenMove() override;

	void canvasPressEvent(QgsMapMouseEvent* e) override;
	void canvasMoveEvent(QgsMapMouseEvent* e) override;


private:
	QgsVectorLayer* mpvecLayer;
	QgsFeatureList mCopiedFeatures;
	QCursor mCursor;
	bool mIsMoving = false;
	QgsPointXY mStartPointMapCoords;
};
