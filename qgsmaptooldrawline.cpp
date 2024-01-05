/**************************�������߹���*********************
*****************************************************************/
#include "qgsmaptooldrawline.h"

QgsMapToolDrawLine::QgsMapToolDrawLine(QgsMapCanvas *canvas):QgsMapTool(canvas)
{
    pMapCanvas=canvas;
    //����QgsRubberBand����,��������
	pRubBand=new QgsRubberBand(pMapCanvas);
	mColor=QColor(255,0,0);
	LineWidth=2;
	ButtonClickFlag=false;
}

QgsMapToolDrawLine::~QgsMapToolDrawLine(void)
{
	/*if(pRubBand){
	   delete pRubBand ;
	}*/
}
//��������ƶ��¼�
void QgsMapToolDrawLine::canvasMoveEvent (QgsMapMouseEvent *e)
{
	int xc,yc;
	//���������û��˫��
	if(!ButtonClickFlag){
		return;
	}
	pRubBand->setColor(mColor);
	pRubBand->setWidth(LineWidth);
	xc=e->x();
	yc=e->y();
	//�õ���ǰ����任����
	const QgsMapToPixel* pTransform=pMapCanvas->getCoordinateTransform();
	//ת���ɵ�ͼ����
	QgsPoint mPoint(pTransform->toMapCoordinates(xc,yc));
	if(pRubBand->numberOfVertices()>1){
		pRubBand->removeLastPoint();
	}
	//�ѵ�ǰ����ӵ�QgsRubberBand���������ڻ���
	pRubBand->addPoint(mPoint);
}
//�������˫���¼�
void  QgsMapToolDrawLine::canvasDoubleClickEvent (QgsMapMouseEvent *e)
{	
	int xc,yc;
	pRubBand->setColor(mColor);
	pRubBand->setWidth(LineWidth);
	//�õ���ǰ����任����
	const QgsMapToPixel* pTransform=pMapCanvas->getCoordinateTransform();
	//�õ������¼��İ�ť��Ϣ
	Qt::MouseButton mButton=e->button();
	xc=e->x();
	yc=e->y();
	QgsPoint  mPoint(pTransform->toMapCoordinates(xc,yc));
	//�������ť
	if(mButton==Qt::MouseButton::LeftButton){
		ButtonClickFlag=true;
	    //�ѵ�ǰ����ӵ�QgsRubberBand���������ڻ���
		pRubBand->addPoint(mPoint);
		mPointSet.append(mPoint);
	}
	else if(mButton==Qt::MouseButton::RightButton){
		pRubBand->addPoint(mPoint); 
		mPointSet.append(mPoint);
		ButtonClickFlag=false;
		//ת��Ϊ������
		pRubBand->asGeometry();
	}
}
//���û�����ɫ���߿�
void QgsMapToolDrawLine::SetColorAndWidth(QColor color,int nWidth)
{
    mColor=color;
	LineWidth=nWidth;
}
//������굥�����¼�
void QgsMapToolDrawLine::canvasPressEvent(QgsMapMouseEvent *e)
{
	if(ButtonClickFlag){
		return;
	}
	//�õ������¼��İ�ť��Ϣ
	Qt::MouseButton mButton=e->button();
	//�������ť
	if(mButton==Qt::MouseButton::LeftButton){
		pRubBand->reset();
	}
}
//���ع����߶ε����ݵ���
int QgsMapToolDrawLine::GetVertexCount()
{
	return mPointSet.size();
}
//�õ��������
bool QgsMapToolDrawLine::GetCoord(int index,double &x,double &y)
{
	QgsPoint mPoint;
	if(index>=mPointSet.size()){
		return true;
	}
	mPoint=mPointSet.at(index);
	x=mPoint.x();
	y=mPoint.y();
}