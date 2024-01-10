#include "MapToolRotate.h"

MapToolRotate::MapToolRotate(QgsMapCanvas* mapcanvas)
	:QgsMapTool(mapcanvas)
    ,mpvecLayer(nullptr)
{

}

MapToolRotate::~MapToolRotate()
{

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
            // 第一次点击左键，开始移动要素
            mIsMoving = true;

            //获取原本的要素IDs
            mOriSelectedFeatureIds = mpvecLayer->selectedFeatureIds();

            // 获取选中的要素，复制要素到mCopiedFeatures
            for (const auto& feature : mpvecLayer->selectedFeatures())
            {
                // 创建一个新的要素
                QgsFeature newFeature;

                // 复制原始要素的几何和属性到新的要素
                newFeature.setGeometry(feature.geometry());
                newFeature.setAttributes(feature.attributes());

                // 将新的要素添加到图层
                mpvecLayer->addFeature(newFeature);

                mCopiedFeatures << newFeature;
            }

            // 如果没有选中的要素，显示错误信息
            if (mCopiedFeatures.isEmpty())
            {
                QMessageBox::warning(nullptr, QObject::tr("Feature Error"), QObject::tr("No feature selected."));
                return;
            }

            //计算选中要素的几何中心
            mRotateCenter = calculateCenterofFeatures(mCopiedFeatures);
            mLastPointMapCoords = toMapCoordinates(e->pos());
            mCursor = QCursor(Qt::ClosedHandCursor);
            QApplication::setOverrideCursor(mCursor);
        }
        else
        {
            // 第二及点击左键，放置复制的要素并删除原先的要素
            mIsMoving = false;
            QApplication::restoreOverrideCursor();
            mpvecLayer->deleteFeatures(mOriSelectedFeatureIds);

            //刷新显示
            mCanvas->refresh();
        }
    }
    else if (e->button() == Qt::RightButton)    //右键点击
    {
        if (mIsMoving)
        {
            //第二次及以后点击右键，取消变换，删除复制的要素
            mIsMoving = false;  //取消移动
            QApplication::restoreOverrideCursor();

            //删除用于旋转的要素
            for (QgsFeatureList::iterator iter = mCopiedFeatures.begin(); iter != mCopiedFeatures.end(); iter++)
            {
                mpvecLayer->deleteFeature(iter->id());
            }

            mCopiedFeatures.clear();    //清除选中要素
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
        for (auto& feature : mCopiedFeatures)
        {
            // 获取要素的几何
            QgsGeometry geom = feature.geometry();

            // 计算新的位置
            QgsPointXY newPointMapCoords = toMapCoordinates(e->pos());

            // 计算从中心点到第一个点的方位角
            double angle1 = mRotateCenter.azimuth(mLastPointMapCoords);

            // 计算从中心点到第二个点的方位角
            double angle2 = mRotateCenter.azimuth(newPointMapCoords);

            // 计算两个方位角的差
            double rotationAngle = angle2 - angle1;


            // 变换几何
            geom.rotate(rotationAngle,mRotateCenter);

            // 更新要素
            feature.setGeometry(geom);
            mpvecLayer->updateFeature(feature);
        }

        // 刷新地图
        mCanvas->refresh();

        // 更新开始点的坐标，以便下次移动时使用
        mLastPointMapCoords = toMapCoordinates(e->pos());
    }
}

QgsPointXY calculateCenterofFeatures(const QgsFeatureList& features)
{
    double centerX = 0.0, centerY = 0.0;
    //迭代器
    for (QgsFeatureList::const_iterator iter = features.constBegin(); iter != features.constEnd(); iter++)
    {
        const QgsGeometry geom = iter->geometry();
        const QgsPointXY centroid = geom.centroid().asPoint();
        centerX += centroid.x();
        centerY += centroid.y();
    }
    centerX /= features.size();
    centerY /= features.size();

    return QgsPointXY(centerX, centerY);
}