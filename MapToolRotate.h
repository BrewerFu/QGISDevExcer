#pragma once

#include<qgsmaptool.h>
#include<qgspointxy.h>
#include<qgsmapmouseevent.h>
#include<qgsvectorlayer.h>
#include<qgsmapcanvas.h>
#include<qmessagebox.h>
#include<qapplication.h>
#include<qgsrubberband.h>


class MapToolRotate:public QgsMapTool
{
	Q_OBJECT

public:
	MapToolRotate(QgsMapCanvas* mapcanvas);
	~MapToolRotate();

	void canvasPressEvent(QgsMapMouseEvent* e) override;
	void canvasMoveEvent(QgsMapMouseEvent* e) override;


private:
	QgsVectorLayer* mpvecLayer;
	QgsFeatureIds mOriSelectedFeatureIds;
	QMap<QgsFeatureId, QgsRubberBand*> mMapIdToRubber;

	QgsPointXY mRotateCenter;
	QCursor mCursor;
	bool mIsMoving = false;
	QgsPointXY mLastPointMapCoords;
};

QgsPointXY calculateCenterofFeatures(const QgsFeatureIds& featureIds,const QgsVectorLayer* layer);
