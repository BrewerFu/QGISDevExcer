#include "MapToolMove.h"
#include <QMessageBox>
#include <QgsGeometry.h>
#include <QgsFeatureIterator.h>

MapToolMove::MapToolMove(QgsMapCanvas *canvas)
    : QgsMapTool(canvas), mLayer(nullptr), mIsMoving(false)
{
    mCursor = QCursor(Qt::OpenHandCursor);
}

MapToolMove::~MapToolMove()
{
}

void MapToolMove::canvasPressEvent(QgsMapMouseEvent *e)
{

    if (e->button() == Qt::LeftButton)
    {
        mLayer = qobject_cast<QgsVectorLayer *>(canvas()->currentLayer());



        if (!mLayer)
        {
            // 如果没有图层被选中，或者选中的不是矢量图层，显示错误信息
            QMessageBox::warning(nullptr, QObject::tr("Layer Error"), QObject::tr("No vector layer selected."));
            return;
        }

        if (!mLayer->isEditable())
            return;

        // 获取选中的要素，并将要素赋值给mSelectedFeatures
        mSelectedFeatures = mLayer->selectedFeatures();

       

        // 如果没有选中的要素，显示错误信息
        if (mSelectedFeatures.isEmpty())
        {
            QMessageBox::warning(nullptr, QObject::tr("Feature Error"), QObject::tr("No feature selected."));
            return;
        }

        if (!mIsMoving)
        {
            // 第一次点击，开始移动要素
            mIsMoving = true;
            mStartPointMapCoords = toMapCoordinates(e->pos());
            mCursor = QCursor(Qt::ClosedHandCursor);
            QApplication::setOverrideCursor(mCursor);
        }
        else
        {
            // 第二次点击，结束移动
            mIsMoving = false;
            QApplication::restoreOverrideCursor();
        }
    }
}

void MapToolMove::canvasMoveEvent(QgsMapMouseEvent *e)
{
    if (mIsMoving)
    {

        for (auto &feature : mSelectedFeatures)
        {
            // 获取要素的几何
            QgsGeometry geom = feature.geometry();

            // 计算新的位置
            QgsPointXY newPoint = toMapCoordinates(e->pos());
            double dx = newPoint.x() - mStartPointMapCoords.x();
            double dy = newPoint.y() - mStartPointMapCoords.y();

            // 变换几何
            geom.translate(dx, dy);

            // 更新要素和图层
            feature.setGeometry(geom);
            mLayer->updateFeature(feature);
        }

        // 刷新地图
        mCanvas->refresh();

        // 更新开始点的坐标，以便下次移动时使用
        mStartPointMapCoords = toMapCoordinates(e->pos());
    }
}

void MapToolMove::canvasReleaseEvent(QgsMapMouseEvent *e)
{
    // 可以在这里添加释放鼠标按钮时的逻辑，如果需要的话
}