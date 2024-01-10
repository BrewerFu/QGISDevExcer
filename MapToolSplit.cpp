#include "MapToolSplit.h"


MapToolSplit::MapToolSplit(QgsMapCanvas* canvas)
	:QgsMapTool(canvas)
{

}

MapToolSplit::~MapToolSplit()
{

}

void MapToolSplit::canvasPressEvent(QgsMapMouseEvent* e)
{
	
    mLayer = qobject_cast<QgsVectorLayer*>(canvas()->currentLayer());

    if (!mLayer)
    {
        // 如果没有图层被选中，或者选中的不是矢量图层，显示错误信息
        QMessageBox::warning(nullptr, QObject::tr("Layer Error"), QObject::tr("No vector layer selected."));
        return;
    }

    if (!mLayer->isEditable())
        return;

    // 获取选中的要素，并将第一个要素赋值给mSelectedFeatures
    mSelectedFeatures = mLayer->selectedFeatures();

    // 如果没有选中的要素，显示错误信息
    if (mSelectedFeatures.isEmpty())
    {
        QMessageBox::warning(nullptr, QObject::tr("Feature Error"), QObject::tr("No feature selected."));
        return;
    }

    if (e->button() == Qt::LeftButton)
    {
        if (!mIsMoving)
        {
            // 第一次点击
            mIsMoving = true;
            mStartPointMapCoords = toMapCoordinates(e->pos());
            mCursor = QCursor(Qt::CrossCursor);
            QApplication::setOverrideCursor(mCursor);

            // 新建切割线
            mpCutiingLine =  new QgsRubberBand(mCanvas);
            mpCutiingLine->setColor(Qt::red);

            // 设置cutline的起点
            mpCutiingLine->addPoint(mStartPointMapCoords);
        }
        else
        {
          //第二次及以后点击左键，添加cutline的终点
            mIsCutting = true;
			QgsPointXY currentPoint = toMapCoordinates(e->pos());
			mpCutiingLine->addPoint(currentPoint);
        }
	}
    else if (e->button() == Qt::RightButton)    //右键点击
    {
        // 已经形成了cutline，右键点击开始裁剪
        if (mIsMoving && mIsCutting)
        {
            mIsMoving = false;
            QApplication::restoreOverrideCursor();

            // 将cutline的Rubber转为PolyLine
            QgsPolylineXY cutline = mpCutiingLine->asGeometry().asPolyline();

            // 遍历每个选中的要素
            for (QgsFeature& feature : mSelectedFeatures)
            {
                // 获取要素的几何体
                QgsGeometry geometry = feature.geometry();


                // 创建一个列表来存储分割后的几何体
                QVector<QgsGeometry> newGeometries;
                // 将cutline转换为QgsPointSequence

                // 分割几何体
                QgsGeometry::OperationResult result = geometry.splitGeometry(cutline, newGeometries, false, cutline);

                if (result == QgsGeometry::Success)
                {
                    // 更新要素的几何体
                    feature.setGeometry(geometry);
                    mLayer->updateFeature(feature);

                    // 将新的几何体添加到图层中
                    for (const QgsGeometry& newGeometry : newGeometries)
                    {
                        QgsFeature newFeature;
                        newFeature.setGeometry(newGeometry);
                        newFeature.setAttributes(feature.attributes()); // 复制原始要素的属性

                        mLayer->addFeature(newFeature);
                    }
                }
                else
                {
                    // 处理分割失败的情况
                    qDebug() << "Failed to split geometry";
                }
            }
            delete mpCutiingLine;

            mLayer->removeSelection();
            mCanvas->refresh();
        }
    }
}

void MapToolSplit::canvasMoveEvent(QgsMapMouseEvent* e)
{
    if (mIsMoving)
    {
        // 获取当前鼠标的位置
        QgsPointXY currentPoint = toMapCoordinates(e->pos());

        // 更新cutline的终点
        mpCutiingLine->movePoint(currentPoint);
    }
}

