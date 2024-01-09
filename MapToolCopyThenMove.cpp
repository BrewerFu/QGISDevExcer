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
        // ���û��ͼ�㱻ѡ�У�����ѡ�еĲ���ʸ��ͼ�㣬��ʾ������Ϣ
        QMessageBox::warning(nullptr, QObject::tr("Layer Error"), QObject::tr("No vector layer selected."));
        return;
    }

    if (e->button() == Qt::LeftButton)      //������
    {
        if (!mIsMoving)
        {
            // ��һ�ε���������ʼ�ƶ�Ҫ��
            mIsMoving = true;

            // ��ȡѡ�е�Ҫ�أ�����Ҫ�ص�mCopiedFeatures
            for (const auto& feature : mpvecLayer->selectedFeatures())
            {
                // ����һ���µ�Ҫ��
                QgsFeature newFeature;

                // ����ԭʼҪ�صļ��κ����Ե��µ�Ҫ��
                newFeature.setGeometry(feature.geometry());
                newFeature.setAttributes(feature.attributes());

                // ���µ�Ҫ����ӵ�ͼ��
                mpvecLayer->addFeature(newFeature);

                mCopiedFeatures << newFeature;
            }

            // ���û��ѡ�е�Ҫ�أ���ʾ������Ϣ
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
            // �ڶ����Ժ�ε�����������Ҫ��
            QApplication::restoreOverrideCursor();

            // ����һ���µ�Ҫ����ӵ�ǰλ�õ�ͼ����
            for (auto& feature : mCopiedFeatures)
            {
                // ��ȡҪ�صļ���
                QgsGeometry geom = feature.geometry();

                // �����µ�λ��
                QgsPointXY newPoint = toMapCoordinates(e->pos());
                double dx = newPoint.x() - mStartPointMapCoords.x();
                double dy = newPoint.y() - mStartPointMapCoords.y();

                // �任����
                geom.translate(dx, dy);

                //�½����Ƶ�Ҫ��
                QgsFeature newFeature;
                newFeature.setGeometry(geom);
                newFeature.setAttributes(feature.attributes());

                // ��Ӹ��Ƶĵ�Ҫ��
                mpvecLayer->addFeature(feature);
            }
        }
    }
    else if (e->button() == Qt::RightButton)    //�Ҽ����
    {
        if (mIsMoving)
        {
            //�ڶ��μ��Ժ����Ҽ�
            mIsMoving = false;  //ȡ���ƶ�

            //ɾ�������ƶ���Ҫ��
            for (QgsFeatureList::iterator iter = mCopiedFeatures.begin(); iter !=mCopiedFeatures.end(); iter++)
            {
                mpvecLayer->deleteFeature(iter->id());
            }

            mCopiedFeatures.clear();    //���ѡ��Ҫ��
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
            // ��ȡҪ�صļ���
            QgsGeometry geom = feature.geometry();

            // �����µ�λ��
            QgsPointXY newPoint = toMapCoordinates(e->pos());
            double dx = newPoint.x() - mStartPointMapCoords.x();
            double dy = newPoint.y() - mStartPointMapCoords.y();

            // �任����
            geom.translate(dx, dy);

            // ����Ҫ��
            feature.setGeometry(geom);
            mpvecLayer->updateFeature(feature);
        }

        // ˢ�µ�ͼ
        mCanvas->refresh();

        // ���¿�ʼ������꣬�Ա��´��ƶ�ʱʹ��
        mStartPointMapCoords = toMapCoordinates(e->pos());
    }
}