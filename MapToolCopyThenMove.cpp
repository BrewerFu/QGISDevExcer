#include"MapToolCopyThenMove.h"

MapToolCopyThenMove::MapToolCopyThenMove(QgsMapCanvas* canvas)
	:QgsMapTool(canvas)
	,mpvecLayer(nullptr)
{

}

MapToolCopyThenMove::~MapToolCopyThenMove()
{

}

void MapToolCopyThenMove::canvasPressEvent(QgsMapMouseEvent* e)
{
    mpvecLayer = dynamic_cast<QgsVectorLayer*>(canvas()->currentLayer());

    if (!mpvecLayer->isEditable())
        return;

    if (!mpvecLayer)
    {
        // 如果没有图层被选中，或者选中的不是矢量图层，显示错误信息
        QMessageBox::warning(nullptr, QObject::tr("Layer Error"), QObject::tr("No vector layer selected."));
        return;
    }

    if (e->button() == Qt::LeftButton)      //左键点击
    {
        if (!mIsMoving)
        {
            // 第一次点击左键，开始移动要素
            mIsMoving = true;

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

            mStartPointMapCoords = toMapCoordinates(e->pos());
            mCursor = QCursor(Qt::ClosedHandCursor);
            QApplication::setOverrideCursor(mCursor);
        }
        else
        {
            // 第二及以后次点击左键，放置要素
            QApplication::restoreOverrideCursor();

            // 复制一份新的要素添加当前位置的图层中
            for (auto& feature : mCopiedFeatures)
            {
                // 获取要素的几何
                QgsGeometry geom = feature.geometry();

                // 计算新的位置
                QgsPointXY newPoint = toMapCoordinates(e->pos());
                double dx = newPoint.x() - mStartPointMapCoords.x();
                double dy = newPoint.y() - mStartPointMapCoords.y();

                // 变换几何
                geom.translate(dx, dy);

                //新建复制的要素
                QgsFeature newFeature;
                newFeature.setGeometry(geom);
                newFeature.setAttributes(feature.attributes());

                // 添加复制的的要素
                mpvecLayer->addFeature(feature);
            }
        }
    }
    else if (e->button() == Qt::RightButton)    //右键点击
    {
        if (mIsMoving)
        {
            //第二次及以后点击右键
            mIsMoving = false;  //取消移动

            //删除用于移动的要素
            for (QgsFeatureList::iterator iter = mCopiedFeatures.begin(); iter !=mCopiedFeatures.end(); iter++)
            {
                mpvecLayer->deleteFeature(iter->id());
            }

            mCopiedFeatures.clear();    //清除选中要素
            mCanvas->refresh();
            QApplication::restoreOverrideCursor();
        }
    }
}

void MapToolCopyThenMove::canvasMoveEvent(QgsMapMouseEvent* e)
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
            QgsPointXY newPoint = toMapCoordinates(e->pos());
            double dx = newPoint.x() - mStartPointMapCoords.x();
            double dy = newPoint.y() - mStartPointMapCoords.y();

            // 变换几何
            geom.translate(dx, dy);

            // 更新要素
            feature.setGeometry(geom);
            mpvecLayer->updateFeature(feature);
        }

        // 刷新地图
        mCanvas->refresh();

        // 更新开始点的坐标，以便下次移动时使用
        mStartPointMapCoords = toMapCoordinates(e->pos());
    }
}