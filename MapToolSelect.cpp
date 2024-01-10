/**************************ѡ��ͼ��Ҫ�ع���*********************
*****************************************************************/
#include "MapToolSelect.h"
#include <QgsProject.h>

// �������������
MapToolSelect::MapToolSelect(QgsMapCanvas *Mapcanvas) : QgsMapToolSelect(Mapcanvas)
{
	mvecLayer = NULL;
	mRubberBand = new QgsRubberBand(Mapcanvas, QgsWkbTypes::PolygonGeometry);

	mCursor = Qt::ArrowCursor;
	mCanvas = Mapcanvas;
	StatusFlag = true;
}

MapToolSelect::~MapToolSelect(void)
{
}


void MapToolSelect::canvasPressEvent(QgsMapMouseEvent *e)
{
	mvecLayer = qobject_cast<QgsVectorLayer*>(mCanvas->currentLayer());

	if (!mvecLayer)
	{
		// ���û��ͼ�㱻ѡ�У�����ѡ�еĲ���ʸ��ͼ�㣬��ʾ������Ϣ
		QMessageBox::warning(nullptr, QObject::tr("Layer Error"), QObject::tr("No vector layer selected."));
		return;
	}

	// �õ������¼��İ�ť��Ϣ
	Qt::MouseButton mButton = e->button();
	// ���������ť����
	if (mButton != Qt::MouseButton::LeftButton)
	{
		return;
	}

	// ��¼�������λ��
	mpressPoint = e->mapPoint();

	// ���һ���㵽 QgsRubberBand ����
	mRubberBand->addPoint(mpressPoint);

	// ��¼һ�ε����ʼ
	isClicked = true;
}

void MapToolSelect::canvasMoveEvent(QgsMapMouseEvent *e)
{

	if (isClicked)
	{
		// ���� QgsRubberBand �������״
		mRubberBand->reset(QgsWkbTypes::PolygonGeometry);
		mRubberBand->addPoint(mpressPoint);
		mRubberBand->addPoint(QgsPointXY(mpressPoint.x(), e->mapPoint().y()));
		mRubberBand->addPoint(e->mapPoint());
		mRubberBand->addPoint(QgsPointXY(e->mapPoint().x(), mpressPoint.y()));
		// ��β����
		mRubberBand->addPoint(mpressPoint);
	}
}

// ��갴ť�ͷ�ʱ,ѡ��������λ�õ�ͼԪ
void MapToolSelect::canvasReleaseEvent(QgsMapMouseEvent *e)
{
	// һ�ε������
	isClicked = false;

	// ���ֻ��һ�ε����û���϶�
	if (e->mapPoint() == mpressPoint)
	{
		ExpandSingleClicked();
	}

	if (mCanvas == NULL)
	{
		return;
	}
	if (mvecLayer == NULL)
	{
		QMessageBox::about(mCanvas, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("��ѡ��ͼ��"));
		return;
	}
	if (StatusFlag == false)
	{
		return;
	}

	// �õ������¼��İ�ť��Ϣ
	Qt::MouseButton mButton = e->button();
	// ���������ť����
	if (mButton != Qt::MouseButton::LeftButton)
	{
		return;
	}

	QgsGeometry selectGeom = mRubberBand->asGeometry();
	// ���������Ч���򷵻�
	if (selectGeom.isNull())
	{
		return;
	}

	QgsRectangle rect = selectGeom.boundingBox();


	// ��ͼ����ѡ��������ζ��������
	SetSelectFeatures(selectGeom, e->modifiers());

	// ��� QgsRubberBand ����
	mRubberBand->reset(QgsWkbTypes::PolygonGeometry);
}

// ��ȡ���λ��һ����Χ��Ϊѡ������
void MapToolSelect::ExpandSingleClicked()
{
	int boxSize = 0;
	// ���ͼ�㲻����ͼԪ����
	if (mvecLayer->geometryType() != QgsWkbTypes::PolygonGeometry)
	{
		boxSize = 5;
	}
	else
	{
		boxSize = 1;
	}

	// ���������豸����ת��Ϊ��ͼ����
	const QgsMapToPixel *transform = mCanvas->getCoordinateTransform();

	// ��ȡ��ͼ������������boxSize��Χ�ĵ�
	QPoint topLeft(mpressPoint.x() - boxSize, mpressPoint.y() - boxSize);
	QPoint bottomRight(mpressPoint.x() + boxSize, mpressPoint.y() + boxSize);

	// ����ͼ���귶Χ��ת��Ϊ��ͼ����㲢��ӵ�mRubberBand
	mRubberBand->addPoint(transform->toMapCoordinates(topLeft));
	mRubberBand->addPoint(transform->toMapCoordinates(QPoint(bottomRight.x(), topLeft.y())));
	mRubberBand->addPoint(transform->toMapCoordinates(bottomRight));
	mRubberBand->addPoint(transform->toMapCoordinates(QPoint(topLeft.x(), bottomRight.y())));
}


// ѡ�񼸺�����
// selectGeometry:ѡ��������ѡ�񼸺���
// modifiers:������ʱ���µİ���
void MapToolSelect::SetSelectFeatures(QgsGeometry &selectGeometry, Qt::KeyboardModifiers modifiers)
{
	// ���ѡ�񼸺��岻�Ƕ����
	if (selectGeometry.type() != QgsWkbTypes::PolygonGeometry)
	{
		return;
	}

	// �趨ѡ�񼸺��������ϵ��ͼ�������ϵһ��
	QgsGeometry selectGeomTrans(selectGeometry);

	try
	{
		if (mvecLayer)
		{
			// ����ͼ�������ϵ�任��ͼ������ϵ
			QgsCoordinateTransform ct(mCanvas->mapSettings().destinationCrs(), mvecLayer->crs(), QgsProject::instance());
			// �趨�����������ϵ��ͼ������ϵһ��
			selectGeomTrans.transform(ct);
		}
		else
		{
			return;
		}
	}
	// �����쳣���׳��쳣
	catch (QgsCsException &cse)
	{
		Q_UNUSED(cse);
		// catch exception for 'invalid' point and leave existing selection unchanged
		QMessageBox::warning(mCanvas, QObject::tr("CRS Exception"),
							 QObject::tr("Selection extends beyond layer's coordinate system."));
		return;
	}

	// ���ù��
	QApplication::setOverrideCursor(Qt::WaitCursor);

	// ��ѡ���Ҫ��
	QgsFeatureIds newSelectedFeatures;
	// ��ȡ����������
	QgsFeatureIterator it = mvecLayer->getFeatures();
	QgsFeature f;

	// �õ���ǰѡ�������
	while (it.nextFeature(f))
	{
		QgsGeometry g = f.geometry();
		// g�Ƿ������selectGeomTrans�������ڲ�
		if (!g.intersects(selectGeomTrans))
		{
			continue;
		}
		// �洢����Ҫ�������id
		newSelectedFeatures.insert(f.id());
	}

	//---------------------------------------------------------------------------------------------------------
	/*��������¼�*/

	// �õ������Ѿ�ѡ���˵�Ҫ�ص�ids
	QgsFeatureIds selectedFeatures = mvecLayer->selectedFeatureIds();
	
	// �������shift��,���Լ�ѡҪ��
	if (modifiers==Qt::ShiftModifier)
	{
		// ������ѡ���з���Ҫ���Ҫ��
		for (QgsFeatureIds::const_iterator iter = newSelectedFeatures.constBegin(); iter != newSelectedFeatures.constEnd(); iter++)
		{
			//���û��ѡ����ѡ��
			if (!selectedFeatures.contains(*iter))
				selectedFeatures.insert(*iter);
		}
		// ����ѡ�񼯺�
		layerSelectedFeatures = selectedFeatures;
	}
	//����alt����ѡ
	else if (modifiers == Qt::AltModifier)
	{
		// ������ѡ���з���Ҫ���Ҫ��
		for (QgsFeatureIds::const_iterator iter = newSelectedFeatures.constBegin(); iter != newSelectedFeatures.constEnd(); iter++)
		{
			//����Ѿ�ѡ����ȡ��ѡ��
			if (selectedFeatures.contains(*iter))
				selectedFeatures.remove(*iter);
		}
		// ����ѡ�񼯺�
		layerSelectedFeatures = selectedFeatures;
	}
	else
	{
		layerSelectedFeatures = newSelectedFeatures;
	}

	// �趨ѡ�������
	mvecLayer->selectByIds(layerSelectedFeatures);
	QApplication::restoreOverrideCursor();
}


// �趨����״̬
void MapToolSelect::SetEnable(bool flag)
{
	StatusFlag = flag;
	if (StatusFlag)
	{
		mCursor = Qt::CrossCursor;
		QApplication::setOverrideCursor(mCursor);
	}
	else
	{
		mCursor = Qt::ArrowCursor;
		QApplication::restoreOverrideCursor();
	}
}