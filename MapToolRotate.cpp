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

            //����ѡ��Ҫ�صļ�������
            mRotateCenter = calculateCenterofFeatures(mCopiedFeatures);
            mLastPointMapCoords = toMapCoordinates(e->pos());
            mCursor = QCursor(Qt::ClosedHandCursor);
            QApplication::setOverrideCursor(mCursor);
        }
        else
        {
            // �ڶ��������������ø��Ƶ�Ҫ�ز�ɾ��ԭ�ȵ�Ҫ��
            mIsMoving = false;
            QApplication::restoreOverrideCursor();
            mpvecLayer->deleteFeatures(mOriSelectedFeatureIds);

            //ˢ����ʾ
            mCanvas->refresh();
        }
    }
    else if (e->button() == Qt::RightButton)    //�Ҽ����
    {
        if (mIsMoving)
        {
            //�ڶ��μ��Ժ����Ҽ���ȡ���任��ɾ�����Ƶ�Ҫ��
            mIsMoving = false;  //ȡ���ƶ�
            QApplication::restoreOverrideCursor();

            //ɾ��������ת��Ҫ��
            for (QgsFeatureList::iterator iter = mCopiedFeatures.begin(); iter != mCopiedFeatures.end(); iter++)
            {
                mpvecLayer->deleteFeature(iter->id());
            }

            mCopiedFeatures.clear();    //���ѡ��Ҫ��
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
            // ��ȡҪ�صļ���
            QgsGeometry geom = feature.geometry();

            // �����µ�λ��
            QgsPointXY newPointMapCoords = toMapCoordinates(e->pos());

            // ��������ĵ㵽��һ����ķ�λ��
            double angle1 = mRotateCenter.azimuth(mLastPointMapCoords);

            // ��������ĵ㵽�ڶ�����ķ�λ��
            double angle2 = mRotateCenter.azimuth(newPointMapCoords);

            // ����������λ�ǵĲ�
            double rotationAngle = angle2 - angle1;


            // �任����
            geom.rotate(rotationAngle,mRotateCenter);

            // ����Ҫ��
            feature.setGeometry(geom);
            mpvecLayer->updateFeature(feature);
        }

        // ˢ�µ�ͼ
        mCanvas->refresh();

        // ���¿�ʼ������꣬�Ա��´��ƶ�ʱʹ��
        mLastPointMapCoords = toMapCoordinates(e->pos());
    }
}

QgsPointXY calculateCenterofFeatures(const QgsFeatureList& features)
{
    double centerX = 0.0, centerY = 0.0;
    //������
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