/**************************绘制折线工具*********************
*****************************************************************/
#include "qgsmaptooldrawline.h"

QgsMapToolDrawLine::QgsMapToolDrawLine(QgsMapCanvas *canvas) : QgsMapTool(canvas)
{
	pMapCanvas = canvas;
	// 设置QgsRubberBand对象,绘制折线
	pRubBand = new QgsRubberBand(pMapCanvas);
	mColor = QColor(255, 0, 0);
	LineWidth = 2;
	ButtonClickFlag = false;
}

QgsMapToolDrawLine::~QgsMapToolDrawLine(void)
{
	/*if(pRubBand){
	   delete pRubBand ;
	}*/
}

// 设置绘制颜色和线宽
void QgsMapToolDrawLine::SetColorAndWidth(QColor color, int nWidth)
{
	mColor = color;
	LineWidth = nWidth;
}

// 返回构成线段的数据点数
int QgsMapToolDrawLine::GetVertexCount()
{
	return mPointSet.size();
}

// 得到点的坐标
bool QgsMapToolDrawLine::GetCoord(int index, double &x, double &y)
{
	QgsPoint mPoint;
	if (index >= mPointSet.size())
	{
		return true;
	}
	mPoint = mPointSet.at(index);
	x = mPoint.x();
	y = mPoint.y();
}

// 重载鼠标移动事件
void QgsMapToolDrawLine::canvasMoveEvent(QgsMapMouseEvent *e)
{
	int xc, yc;
	// 如果鼠标左键没有双击
	if (!ButtonClickFlag)
	{
		return;
	}
	pRubBand->setColor(mColor);
	pRubBand->setWidth(LineWidth);
	xc = e->x();
	yc = e->y();
	// 得到当前坐标变换对象
	const QgsMapToPixel *pTransform = pMapCanvas->getCoordinateTransform();
	// 转换成地图坐标
	QgsPoint mPoint(pTransform->toMapCoordinates(xc, yc));
	if (pRubBand->numberOfVertices() > 1)
	{
		pRubBand->removeLastPoint();
	}
	// 把当前点添加到QgsRubberBand对象中用于绘制
	pRubBand->addPoint(mPoint);
}

// 重载鼠标双击事件
void QgsMapToolDrawLine::canvasDoubleClickEvent(QgsMapMouseEvent *e)
{
	int xc, yc;
	pRubBand->setColor(mColor);
	pRubBand->setWidth(LineWidth);
	// 得到当前坐标变换对象
	const QgsMapToPixel *pTransform = pMapCanvas->getCoordinateTransform();
	// 得到产生事件的按钮信息
	Qt::MouseButton mButton = e->button();
	xc = e->x();
	yc = e->y();
	QgsPoint mPoint(pTransform->toMapCoordinates(xc, yc));
	// 如果是左按钮
	if (mButton == Qt::MouseButton::LeftButton)
	{
		ButtonClickFlag = true;
		// 把当前点添加到QgsRubberBand对象中用于绘制
		pRubBand->addPoint(mPoint);
		mPointSet.append(mPoint);
	}
	else if (mButton == Qt::MouseButton::RightButton)
	{
		pRubBand->addPoint(mPoint);
		mPointSet.append(mPoint);
		ButtonClickFlag = false;
		// 转化为几何体
		pRubBand->asGeometry();
		// 将完成的线添加到lines列表中
		lines.append(pRubBand);
		// 创建一个新的QgsRubberBand对象，用于绘制下一条线
		pRubBand = new QgsRubberBand(pMapCanvas);
	}
}

// 重载鼠标单击击事件
void QgsMapToolDrawLine::canvasPressEvent(QgsMapMouseEvent *e)
{
	if (ButtonClickFlag)
	{
		return;
	}
	// 得到产生事件的按钮信息
	Qt::MouseButton mButton = e->button();
	// 如果是左按钮
	if (mButton == Qt::MouseButton::LeftButton)
	{
		pRubBand->reset();
	}
}
