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
        // ���û��ͼ�㱻ѡ�У�����ѡ�еĲ���ʸ��ͼ�㣬��ʾ������Ϣ
        QMessageBox::warning(nullptr, QObject::tr("Layer Error"), QObject::tr("No vector layer selected."));
        return;
    }

    if (!mLayer->isEditable())
        return;

    // ��ȡѡ�е�Ҫ�أ�������һ��Ҫ�ظ�ֵ��mSelectedFeatures
    mSelectedFeatures = mLayer->selectedFeatures();

    // ���û��ѡ�е�Ҫ�أ���ʾ������Ϣ
    if (mSelectedFeatures.isEmpty())
    {
        QMessageBox::warning(nullptr, QObject::tr("Feature Error"), QObject::tr("No feature selected."));
        return;
    }

    if (e->button() == Qt::LeftButton)
    {
        if (!mIsMoving)
        {
            // ��һ�ε��
            mIsMoving = true;
            mStartPointMapCoords = toMapCoordinates(e->pos());
            mCursor = QCursor(Qt::CrossCursor);
            QApplication::setOverrideCursor(mCursor);

            // �½��и���
            mpCutiingLine =  new QgsRubberBand(mCanvas);
            mpCutiingLine->setColor(Qt::red);

            // ����cutline�����
            mpCutiingLine->addPoint(mStartPointMapCoords);
        }
        else
        {
          //�ڶ��μ��Ժ�����������cutline���յ�
            mIsCutting = true;
			QgsPointXY currentPoint = toMapCoordinates(e->pos());
			mpCutiingLine->addPoint(currentPoint);
        }
	}
    else if (e->button() == Qt::RightButton)    //�Ҽ����
    {
        // �Ѿ��γ���cutline���Ҽ������ʼ�ü�
        if (mIsMoving && mIsCutting)
        {
            mIsMoving = false;
            QApplication::restoreOverrideCursor();

            // ��cutline��RubberתΪPolyLine
            QgsPolylineXY cutline = mpCutiingLine->asGeometry().asPolyline();

            // ����ÿ��ѡ�е�Ҫ��
            for (QgsFeature& feature : mSelectedFeatures)
            {
                // ��ȡҪ�صļ�����
                QgsGeometry geometry = feature.geometry();


                // ����һ���б����洢�ָ��ļ�����
                QVector<QgsGeometry> newGeometries;
                // ��cutlineת��ΪQgsPointSequence

                // �ָ����
                QgsGeometry::OperationResult result = geometry.splitGeometry(cutline, newGeometries, false, cutline);

                if (result == QgsGeometry::Success)
                {
                    // ����Ҫ�صļ�����
                    feature.setGeometry(geometry);
                    mLayer->updateFeature(feature);

                    // ���µļ�������ӵ�ͼ����
                    for (const QgsGeometry& newGeometry : newGeometries)
                    {
                        QgsFeature newFeature;
                        newFeature.setGeometry(newGeometry);
                        newFeature.setAttributes(feature.attributes()); // ����ԭʼҪ�ص�����

                        mLayer->addFeature(newFeature);
                    }
                }
                else
                {
                    // ����ָ�ʧ�ܵ����
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
        // ��ȡ��ǰ����λ��
        QgsPointXY currentPoint = toMapCoordinates(e->pos());

        // ����cutline���յ�
        mpCutiingLine->movePoint(currentPoint);
    }
}

