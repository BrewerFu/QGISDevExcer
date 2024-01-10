#pragma once

#include<qgsmaptool.h>
#include<qgspointxy.h>
#include<qgsmapmouseevent.h>
#include<qgsvectorlayer.h>
#include<qgsmapcanvas.h>
#include<qmessagebox.h>
#include<qapplication.h>
#include<qgsmultipolygon.h>
#include<qgspolygon.h>
#include<qgslinestring.h>
#include<qgsmultipoint.h>
#include<qgsmultilinestring.h>
#include<qgsmultipolygon.h>
#include<qgsrubberband.h>

class MapToolZoom :public QgsMapTool
{
	Q_OBJECT

public:
	MapToolZoom(QgsMapCanvas* canvas);
	~MapToolZoom();

	void canvasPressEvent(QgsMapMouseEvent* e) override;
	void canvasMoveEvent(QgsMapMouseEvent* e) override;

private:
	QgsVectorLayer* mpvecLayer;
	QgsFeatureIds mOriSelectedFeatureIds;
	QMap<QgsFeatureId, QgsRubberBand*> mMapIdToRubber;
	bool mIsMoving = false;
	QgsPointXY mLastPointMapCoords;
	QgsPointXY mZoomCenter;
};

QgsPointXY calculateCenterofFeatures(const QgsFeatureIds& featureIds, const QgsVectorLayer* layer);
