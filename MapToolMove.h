#ifndef MAPTOOLMOVE_H
#define MAPTOOLMOVE_H

#include <qgsmaptool.h>
#include <qgsmapcanvas.h>
#include <qgsvectorlayer.h>
#include <qgsrubberband.h>
#include <QgsMapMouseEvent.h>
#include <QCursor>
#include <qapplication.h>

class MapToolMove : public QgsMapTool
{
    Q_OBJECT

public:
    MapToolMove(QgsMapCanvas* canvas);
    ~MapToolMove() override;

    void canvasPressEvent(QgsMapMouseEvent* e) override;
    void canvasMoveEvent(QgsMapMouseEvent* e) override;
    void canvasReleaseEvent(QgsMapMouseEvent* e) override;

private:
    QgsVectorLayer* mLayer = nullptr;
    QgsRubberBand* mRubberBand = nullptr;
    QgsFeatureList mSelectedFeatures;
    bool mIsMoving = false;
    QCursor mCursor;
    QgsPointXY mStartPointMapCoords;
};

#endif // MAPTOOLMOVE_H