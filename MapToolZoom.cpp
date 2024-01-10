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
        // ���û��ͼ�㱻ѡ�У�����ѡ�еĲ���ʸ��ͼ�㣬��ʾ������Ϣ
        QMessageBox::warning(nullptr, QObject::tr("Layer Error"), QObject::tr("No vector layer selected."));
        return;
    }

    if (!mpvecLayer->isEditable())
        return;

    if (e->button() == Qt::LeftButton)      //������
    {
        if (!mIsMoving)
        {
            // ��һ�ε���������ʼ�ƶ�Ҫ��
            mIsMoving = true;

            //��ȡԭ����Ҫ��IDs
            mOriSelectedFeatureIds = mpvecLayer->selectedFeatureIds();

            QColor fillColor = Qt::red;  // ����һ����ɫ
            fillColor.setAlpha(50);  // ����͸����Ϊ100
            // ��ȡѡ�е�Ҫ�أ�ӳ��Ҫ�ص�RubberBand
            for (const auto& feature : mpvecLayer->selectedFeatures())
            {
                QgsRubberBand* rubberBand = new QgsRubberBand(mCanvas,mpvecLayer->geometryType());
                rubberBand->setColor(fillColor);
                rubberBand->addGeometry(feature.geometry());
                mMapIdToRubber.insert(feature.id(), rubberBand);
            }

            // ���û��ѡ�е�Ҫ�أ���ʾ������Ϣ
            if (mOriSelectedFeatureIds.size()==0)
            {
                QMessageBox::warning(nullptr, QObject::tr("Feature Error"), QObject::tr("No feature selected."));
                return;
            }

            //����ѡ��Ҫ�صļ�������
            mZoomCenter = calculateCenterofFeatures(mOriSelectedFeatureIds,mpvecLayer);
            mLastPointMapCoords = toMapCoordinates(e->pos());
            mCursor = QCursor(Qt::ClosedHandCursor);
            QApplication::setOverrideCursor(mCursor);
        }
        else
        {
            // �ڶ�����������ʹ��rubberband����ѡ��Ҫ�صļ�����
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
            //ˢ����ʾ
            mCanvas->refresh();
        }
    }
    else if (e->button() == Qt::RightButton)    //�Ҽ����
    {
        if (mIsMoving)
        {
            //�ڶ��μ��Ժ����Ҽ���ȡ���任�����rubberband
            mIsMoving = false;  //ȡ���ƶ�
            QApplication::restoreOverrideCursor();

            // ɾ��RubberBand
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
        // �����µ�λ��
        QgsPointXY newPointMapCoords = toMapCoordinates(e->pos());

        // ��������ĵ㵽��һ����ľ���
        double distance1 = mZoomCenter.distance(mLastPointMapCoords);

        // ��������ĵ㵽�ڶ�����ľ���
        double distance2 = mZoomCenter.distance(newPointMapCoords);

        // ������������ı���
        double scaleFactor = distance2 / distance1;

        QMap<QgsFeatureId, QgsRubberBand*>::iterator iter;
        
        for (iter = mMapIdToRubber.begin(); iter != mMapIdToRubber.end(); iter++)
        {
            QgsGeometry geom = iter.value()->asGeometry();

            // ����һ������任�����������ż�����
            QTransform ct;
            ct.translate(mZoomCenter.x(), mZoomCenter.y());
            ct.scale(scaleFactor, scaleFactor);
            ct.translate(-mZoomCenter.x(), -mZoomCenter.y());

            geom.transform(ct);

            iter.value()->setToGeometry(geom);
        }

        // ˢ�µ�ͼ
        mCanvas->refresh();

        // ���¿�ʼ������꣬�Ա��´��ƶ�ʱʹ��
        mLastPointMapCoords = toMapCoordinates(e->pos());
    }
}


