#include "MapToolRotate.h"

MapToolRotate::MapToolRotate(QgsMapCanvas* mapcanvas)
	:QgsMapTool(mapcanvas)
    ,mpvecLayer(nullptr)
{

}

MapToolRotate::~MapToolRotate()
{
    QMap<QgsFeatureId, QgsRubberBand*>::iterator iter;
    for (iter = mMapIdToRubber.begin(); iter != mMapIdToRubber.end(); iter++)
    {
        iter.value()->reset();
        if (iter.value())
            delete iter.value();
    }
}

void MapToolRotate::canvasPressEvent(QgsMapMouseEvent* e)
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
            // 第一次点击左键，开始旋转要素
            mIsMoving = true;

            //获取原本的要素IDs
            mOriSelectedFeatureIds = mpvecLayer->selectedFeatureIds();


            QColor fillColor = Qt::red;  // 创建一个红色
            fillColor.setAlpha(50);  // 设置透明度为100

            // 获取选中的要素，映射要素到RubberBand
            for (const auto& feature : mpvecLayer->selectedFeatures())
            {
                QgsRubberBand* rubberBand = new QgsRubberBand(mCanvas, mpvecLayer->geometryType());
                rubberBand->setColor(fillColor);
                rubberBand->addGeometry(feature.geometry());
                mMapIdToRubber.insert(feature.id(), rubberBand);
            }

            // 如果没有选中的要素，显示错误信息
            if (mOriSelectedFeatureIds.size() == 0)
            {
                QMessageBox::warning(nullptr, QObject::tr("Feature Error"), QObject::tr("No feature selected."));
                return;
            }

            //计算选中要素的几何中心
            mRotateCenter = calculateCenterofFeatures(mOriSelectedFeatureIds,mpvecLayer);
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
            //第二次及以后点击右键，取消变换，删除RubberBand
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


void MapToolRotate::canvasMoveEvent(QgsMapMouseEvent* e)
{
    if (mIsMoving == false)
        return;
    else
    {
        // 计算新的位置
        QgsPointXY newPointMapCoords = toMapCoordinates(e->pos());

        // 计算从中心点到第一个点的方位角
        double angle1 = mRotateCenter.azimuth(mLastPointMapCoords);

        // 计算从中心点到第二个点的方位角
        double angle2 = mRotateCenter.azimuth(newPointMapCoords);

        // 计算两个方位角的差
        double rotationAngle = angle2 - angle1;

        //更新RubberBand
        QMap<QgsFeatureId, QgsRubberBand*>::iterator iter;
        for (iter = mMapIdToRubber.begin(); iter != mMapIdToRubber.end(); iter++)
        {
			QgsGeometry geom = iter.value()->asGeometry();
			geom.rotate(rotationAngle, mRotateCenter);
			iter.value()->setToGeometry(geom);
		}

        // 刷新地图
        mCanvas->refresh();

        // 更新开始点的坐标，以便下次移动时使用
        mLastPointMapCoords = toMapCoordinates(e->pos());
    }
}

QgsPointXY calculateCenterofFeatures(const QgsFeatureIds& featureIds, const QgsVectorLayer* layer)
{
    double centerX = 0.0, centerY = 0.0;
    //迭代器
    QgsFeatureIds::const_iterator iter;
    for (iter = featureIds.constBegin(); iter != featureIds.constEnd(); iter++)
    {
		const QgsFeature feature = layer->getFeature(*iter);
		const QgsGeometry geom = feature.geometry();
		const QgsPointXY centroid = geom.centroid().asPoint();
		centerX += centroid.x();
		centerY += centroid.y();
	}
    centerX /= featureIds.size();
    centerY /= featureIds.size();

    return QgsPointXY(centerX, centerY);
}