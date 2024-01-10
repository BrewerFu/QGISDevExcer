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
            // ���û��ͼ�㱻ѡ�У�����ѡ�еĲ���ʸ��ͼ�㣬��ʾ������Ϣ
            QMessageBox::warning(nullptr, QObject::tr("Layer Error"), QObject::tr("No vector layer selected."));
            return;
        }

        if (!mLayer->isEditable())
            return;

        // ��ȡѡ�е�Ҫ�أ�����Ҫ�ظ�ֵ��mSelectedFeatures
        mSelectedFeatures = mLayer->selectedFeatures();

       

        // ���û��ѡ�е�Ҫ�أ���ʾ������Ϣ
        if (mSelectedFeatures.isEmpty())
        {
            QMessageBox::warning(nullptr, QObject::tr("Feature Error"), QObject::tr("No feature selected."));
            return;
        }

        if (!mIsMoving)
        {
            // ��һ�ε������ʼ�ƶ�Ҫ��
            mIsMoving = true;
            mStartPointMapCoords = toMapCoordinates(e->pos());
            mCursor = QCursor(Qt::ClosedHandCursor);
            QApplication::setOverrideCursor(mCursor);
        }
        else
        {
            // �ڶ��ε���������ƶ�
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
            // ��ȡҪ�صļ���
            QgsGeometry geom = feature.geometry();

            // �����µ�λ��
            QgsPointXY newPoint = toMapCoordinates(e->pos());
            double dx = newPoint.x() - mStartPointMapCoords.x();
            double dy = newPoint.y() - mStartPointMapCoords.y();

            // �任����
            geom.translate(dx, dy);

            // ����Ҫ�غ�ͼ��
            feature.setGeometry(geom);
            mLayer->updateFeature(feature);
        }

        // ˢ�µ�ͼ
        mCanvas->refresh();

        // ���¿�ʼ������꣬�Ա��´��ƶ�ʱʹ��
        mStartPointMapCoords = toMapCoordinates(e->pos());
    }
}

void MapToolMove::canvasReleaseEvent(QgsMapMouseEvent *e)
{
    // ��������������ͷ���갴ťʱ���߼��������Ҫ�Ļ�
}