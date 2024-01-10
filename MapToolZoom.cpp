#include "MapToolZoom.h"


MapToolZoom::MapToolZoom(QgsMapCanvas* canvas)
    : QgsMapTool(canvas)
    , mpvecLayer(nullptr)
{

}

MapToolZoom::~MapToolZoom()
{
    QMap<QgsFeatureId, QgsRubberBand*>::iterator iter;
    for (iter = mMapIdToRubber.begin(); iter != mMapIdToRubber.end(); iter++)
    {
        iter.value()->reset();
        if(iter.value())
		    delete iter.value();
	}
}

void MapToolZoom::canvasPressEvent(QgsMapMouseEvent* e)
{
    mpvecLayer = dynamic_cast<QgsVectorLayer*>(canvas()->currentLayer());

    if (!mpvecLayer)
    {
        // 如果没有图层被选中，或者选中的不是矢量图层，显示错误信息
        QMessageBox::warning(nullptr, QObject::tr("Layer Error"), QObject::tr("No vector layer selected."));
        return;
    }

    if (!mpvecLayer->isEditable())
        return;

    if (e->button() == Qt::LeftButton)      //左键点击
    {
        if (!mIsMoving)
        {
            // 第一次点击左键，开始移动要素
            mIsMoving = true;

            //获取原本的要素IDs
            mOriSelectedFeatureIds = mpvecLayer->selectedFeatureIds();

            QColor fillColor = Qt::red;  // 创建一个红色
            fillColor.setAlpha(50);  // 设置透明度为100
            // 获取选中的要素，映射要素到RubberBand
            for (const auto& feature : mpvecLayer->selectedFeatures())
            {
                QgsRubberBand* rubberBand = new QgsRubberBand(mCanvas,mpvecLayer->geometryType());
                rubberBand->setColor(fillColor);
                rubberBand->addGeometry(feature.geometry());
                mMapIdToRubber.insert(feature.id(), rubberBand);
            }

            // 如果没有选中的要素，显示错误信息
            if (mOriSelectedFeatureIds.size()==0)
            {
                QMessageBox::warning(nullptr, QObject::tr("Feature Error"), QObject::tr("No feature selected."));
                return;
            }

            //计算选中要素的几何中心
            mZoomCenter = calculateCenterofFeatures(mOriSelectedFeatureIds,mpvecLayer);
            mLastPointMapCoords = toMapCoordinates(e->pos());
            mCursor = QCursor(Qt::ClosedHandCursor);
            QApplication::setOverrideCursor(mCursor);
        }
        else
        {
            // 第二及点击左键，使用rubberband更新选中要素的几何体
            mIsMoving = false;
            QApplication::restoreOverrideCursor();

            QMap<QgsFeatureId, QgsRubberBand*>::iterator iter;
            for (iter = mMapIdToRubber.begin(); iter != mMapIdToRubber.end(); iter++)
            {
                QgsGeometry geom = iter.value()->asGeometry();
                mpvecLayer->changeGeometry(iter.key(), geom);
                iter.value()->reset();
                delete iter.value();
            }
            mMapIdToRubber.clear();
            //刷新显示
            mCanvas->refresh();
        }
    }
    else if (e->button() == Qt::RightButton)    //右键点击
    {
        if (mIsMoving)
        {
            //第二次及以后点击右键，取消变换，清除rubberband
            mIsMoving = false;  //取消移动
            QApplication::restoreOverrideCursor();

            // 删除RubberBand
            QMap<QgsFeatureId, QgsRubberBand*>::iterator iter;
            for (iter = mMapIdToRubber.begin(); iter != mMapIdToRubber.end(); iter++)
            {
                iter.value()->reset();
				delete iter.value();
			}
            mMapIdToRubber.clear();
            mCanvas->refresh();
        }
    }
}


void MapToolZoom::canvasMoveEvent(QgsMapMouseEvent* e)
{
    if (mIsMoving == false)
        return;
    else
    {
        // 计算新的位置
        QgsPointXY newPointMapCoords = toMapCoordinates(e->pos());

        // 计算从中心点到第一个点的距离
        double distance1 = mZoomCenter.distance(mLastPointMapCoords);

        // 计算从中心点到第二个点的距离
        double distance2 = mZoomCenter.distance(newPointMapCoords);

        // 计算两个距离的比例
        double scaleFactor = distance2 / distance1;

        QMap<QgsFeatureId, QgsRubberBand*>::iterator iter;
        
        for (iter = mMapIdToRubber.begin(); iter != mMapIdToRubber.end(); iter++)
        {
            QgsGeometry geom = iter.value()->asGeometry();

            // 创建一个仿射变换对象，用于缩放几何体
            QTransform ct;
            ct.translate(mZoomCenter.x(), mZoomCenter.y());
            ct.scale(scaleFactor, scaleFactor);
            ct.translate(-mZoomCenter.x(), -mZoomCenter.y());

            geom.transform(ct);

            iter.value()->setToGeometry(geom);
        }

        // 刷新地图
        mCanvas->refresh();

        // 更新开始点的坐标，以便下次移动时使用
        mLastPointMapCoords = toMapCoordinates(e->pos());
    }
}


