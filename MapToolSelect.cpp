/**************************选择图层要素工具*********************
*****************************************************************/
#include "MapToolSelect.h"
#include <QgsProject.h>

// 构造和析构函数
MapToolSelect::MapToolSelect(QgsMapCanvas *Mapcanvas) : QgsMapToolSelect(Mapcanvas)
{
	pLayer = NULL;
	mRubberBand = new QgsRubberBand(Mapcanvas, QgsWkbTypes::PolygonGeometry);

	mCursor = Qt::ArrowCursor;
	mCanvas = Mapcanvas;
	StatusFlag = true;
}

MapToolSelect::~MapToolSelect(void)
{
}
// 设置当前被选择(活动)的图层
void MapToolSelect::SetSelectLayer(QgsVectorLayer *Layer)
{
	pLayer = Layer;
}

void MapToolSelect::canvasPressEvent(QgsMapMouseEvent *e)
{

	// 得到产生事件的按钮信息
	Qt::MouseButton mButton = e->button();
	// 如果不是左按钮返回
	if (mButton != Qt::MouseButton::LeftButton)
	{
		return;
	}

	// 记录鼠标点击的位置
	mpressPoint = e->mapPoint();

	// 添加一个点到 QgsRubberBand 对象
	mRubberBand->addPoint(mpressPoint);

	// 记录一次点击开始
	isClicked = true;
}

void MapToolSelect::canvasMoveEvent(QgsMapMouseEvent *e)
{

	if (isClicked)
	{
		// 更新 QgsRubberBand 对象的形状
		mRubberBand->reset(QgsWkbTypes::PolygonGeometry);
		mRubberBand->addPoint(mpressPoint);
		mRubberBand->addPoint(QgsPointXY(mpressPoint.x(), e->mapPoint().y()));
		mRubberBand->addPoint(e->mapPoint());
		mRubberBand->addPoint(QgsPointXY(e->mapPoint().x(), mpressPoint.y()));
		// 首尾相连
		mRubberBand->addPoint(mpressPoint);
	}
}

// 鼠标按钮释放时,选择包含鼠标位置的图元
void MapToolSelect::canvasReleaseEvent(QgsMapMouseEvent *e)
{
	// 一次点击结束
	isClicked = false;

	// 如果只是一次点击而没有拖动
	if (e->mapPoint() == mpressPoint)
	{
		ExpandSingleClicked();
	}

	if (mCanvas == NULL)
	{
		return;
	}
	if (pLayer == NULL)
	{
		QMessageBox::about(mCanvas, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("请选择图层"));
		return;
	}
	if (StatusFlag == false)
	{
		return;
	}

	// 得到产生事件的按钮信息
	Qt::MouseButton mButton = e->button();
	// 如果不是左按钮返回
	if (mButton != Qt::MouseButton::LeftButton)
	{
		return;
	}

	QgsGeometry selectGeom = mRubberBand->asGeometry();
	// 如果几何无效，则返回
	if (selectGeom.isNull())
	{
		return;
	}

	QgsRectangle rect = selectGeom.boundingBox();


	// 在图层中选择最靠近几何对象的特征
	SetSelectFeatures(selectGeom, e->modifiers());

	// 清除 QgsRubberBand 对象
	mRubberBand->reset(QgsWkbTypes::PolygonGeometry);
}

// 提取鼠标位置一定范围作为选择区域
void MapToolSelect::ExpandSingleClicked()
{
	int boxSize = 0;
	// 如果图层不是面图元类型
	if (pLayer->geometryType() != QgsWkbTypes::PolygonGeometry)
	{
		boxSize = 5;
	}
	else
	{
		boxSize = 1;
	}

	// 将点击点的设备坐标转换为地图坐标
	const QgsMapToPixel *transform = mCanvas->getCoordinateTransform();

	// 获取视图坐标上下左右boxSize范围的点
	QPoint topLeft(mpressPoint.x() - boxSize, mpressPoint.y() - boxSize);
	QPoint bottomRight(mpressPoint.x() + boxSize, mpressPoint.y() + boxSize);

	// 将视图坐标范围点转换为地图坐标点并添加到mRubberBand
	mRubberBand->addPoint(transform->toMapCoordinates(topLeft));
	mRubberBand->addPoint(transform->toMapCoordinates(QPoint(bottomRight.x(), topLeft.y())));
	mRubberBand->addPoint(transform->toMapCoordinates(bottomRight));
	mRubberBand->addPoint(transform->toMapCoordinates(QPoint(topLeft.x(), bottomRight.y())));
}


// 选择几何特征
// selectGeometry:选择特征的选择几何体
// modifiers:点击鼠标时按下的案件
void MapToolSelect::SetSelectFeatures(QgsGeometry &selectGeometry, Qt::KeyboardModifiers modifiers)
{
	// 如果选择几何体不是多边形
	if (selectGeometry.type() != QgsWkbTypes::PolygonGeometry)
	{
		return;
	}

	// 设定选择几何体的坐标系和图层的坐标系一致
	QgsGeometry selectGeomTrans(selectGeometry);

	try
	{
		if (pLayer)
		{
			// 将地图绘板坐标系变换到图层坐标系
			QgsCoordinateTransform ct(mCanvas->mapSettings().destinationCrs(), pLayer->crs(), QgsProject::instance());
			// 设定几何体的坐标系和图层坐标系一致
			selectGeomTrans.transform(ct);
		}
		else
		{
			return;
		}
	}
	// 对于异常点抛出异常
	catch (QgsCsException &cse)
	{
		Q_UNUSED(cse);
		// catch exception for 'invalid' point and leave existing selection unchanged
		QMessageBox::warning(mCanvas, QObject::tr("CRS Exception"),
							 QObject::tr("Selection extends beyond layer's coordinate system."));
		return;
	}

	// 设置光标
	QApplication::setOverrideCursor(Qt::WaitCursor);

	// 新选择的要素
	QgsFeatureIds newSelectedFeatures;
	// 获取特征迭代器
	QgsFeatureIterator it = pLayer->getFeatures();
	QgsFeature f;

	// 得到当前选择的特征
	while (it.nextFeature(f))
	{
		QgsGeometry g = f.geometry();
		// g是否包含在selectGeomTrans几何体内部
		if (!g.intersects(selectGeomTrans))
		{
			continue;
		}
		// 存储符合要求的特征id
		newSelectedFeatures.insert(f.id());
	}

	//---------------------------------------------------------------------------------------------------------
	/*处理键盘事件*/

	// 得到所有已经选择了的要素的ids
	QgsFeatureIds selectedFeatures = pLayer->selectedFeatureIds();
	
	// 如果按下shift键,可以加选要素
	if (modifiers==Qt::ShiftModifier)
	{
		// 遍历新选择中符合要求的要素
		for (QgsFeatureIds::const_iterator iter = newSelectedFeatures.constBegin(); iter != newSelectedFeatures.constEnd(); iter++)
		{
			//如果没有选中则选中
			if (!selectedFeatures.contains(*iter))
				selectedFeatures.insert(*iter);
		}
		// 更新选择集合
		layerSelectedFeatures = selectedFeatures;
	}
	//按下alt键减选
	else if (modifiers == Qt::AltModifier)
	{
		// 遍历新选择中符合要求的要素
		for (QgsFeatureIds::const_iterator iter = newSelectedFeatures.constBegin(); iter != newSelectedFeatures.constEnd(); iter++)
		{
			//如果已经选中则取消选中
			if (selectedFeatures.contains(*iter))
				selectedFeatures.remove(*iter);
		}
		// 更新选择集合
		layerSelectedFeatures = selectedFeatures;
	}
	else
	{
		layerSelectedFeatures = newSelectedFeatures;
	}

	// 设定选择的特征
	pLayer->selectByIds(layerSelectedFeatures);
	QApplication::restoreOverrideCursor();
}


// 设定工具状态
void MapToolSelect::SetEnable(bool flag)
{
	StatusFlag = flag;
	if (StatusFlag)
	{
		mCursor = Qt::CrossCursor;
	}
	else
	{
		mCursor = Qt::ArrowCursor;
	}
}