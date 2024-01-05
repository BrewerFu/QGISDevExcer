/**************************ѡ��ͼ��Ҫ�ع���*********************
*****************************************************************/
#include "MapToolSelect.h"
#include <QgsProject.h>

//�������������
MapToolSelect::MapToolSelect(QgsMapCanvas *Mapcanvas):QgsMapTool(Mapcanvas)
{
   pLayer=NULL;
   mRubberBand = new QgsRubberBand(Mapcanvas, QgsWkbTypes::PolygonGeometry);

   // ������ɫ�Ϳ��
   mRubberBand->setColor(QColor(255, 0, 0));
   mRubberBand->setWidth(2);

   mCursor=Qt::ArrowCursor;
   mCanvas=Mapcanvas;
   StatusFlag=true;
}

MapToolSelect::~MapToolSelect(void)
{

}
//���õ�ǰ��ѡ��(�)��ͼ��
void MapToolSelect::SetSelectLayer(QgsVectorLayer *Layer)
{
   pLayer=Layer;
}

void MapToolSelect::canvasPressEvent(QgsMapMouseEvent* e)
{

	//�õ������¼��İ�ť��Ϣ
	Qt::MouseButton mButton = e->button();
	//���������ť����
	if (mButton != Qt::MouseButton::LeftButton) {
		return;
	}


	// ��¼�������λ��
	mpressPoint = e->mapPoint();

	// ���һ���㵽 QgsRubberBand ����
	mRubberBand->addPoint(mpressPoint);

	//��¼һ�ε����ʼ
	isClicked = true;
}


void MapToolSelect::canvasMoveEvent(QgsMapMouseEvent* e)
{
	
	if (isClicked)
	{
		// ���� QgsRubberBand �������״
		mRubberBand->reset(QgsWkbTypes::PolygonGeometry);
		mRubberBand->addPoint(mpressPoint);
		mRubberBand->addPoint(QgsPointXY(mpressPoint.x(), e->mapPoint().y()));
		mRubberBand->addPoint(e->mapPoint());
		mRubberBand->addPoint(QgsPointXY(e->mapPoint().x(), mpressPoint.y()));
		//��β����
		mRubberBand->addPoint(mpressPoint);
	}
}

//��갴ť�ͷ�ʱ,ѡ��������λ�õ�ͼԪ
void MapToolSelect::canvasReleaseEvent(QgsMapMouseEvent * e )
{
	//һ�ε������
	isClicked = false;

	//���ֻ��һ�ε����û���϶�
	if (e->mapPoint() == mpressPoint)
	{
		ExpandSingleClicked();
	}

   if(mCanvas==NULL){
	   return;
   }
   if(pLayer==NULL){
	   QMessageBox::about(mCanvas,QString::fromLocal8Bit("����"),QString::fromLocal8Bit("��ѡ��ͼ��"));
	   return;
   }
   if(StatusFlag==false){
	   return;
   }

   //�õ������¼��İ�ť��Ϣ
   Qt::MouseButton mButton=e->button();
   //���������ť����
   if(mButton!=Qt::MouseButton::LeftButton){
	   return;
   }


   QgsGeometry selectGeom = mRubberBand->asGeometry();
   //���������Ч���򷵻�
   if (selectGeom.isNull()) {
	   return;
   }

   QgsRectangle rect = selectGeom.boundingBox();


   //ȷ���Ƿ���ctrl��
   bool doDifference=e->modifiers()&Qt::ControlModifier ? true:false;
   //��ͼ����ѡ��������ζ��������
   SetSelectFeatures(selectGeom,false,doDifference,true);

   //rubberBand.reset();

   // ��� QgsRubberBand ����
   mRubberBand->reset(QgsWkbTypes::PolygonGeometry);
	
}


//��ȡ���λ��һ����Χ��Ϊѡ������
void MapToolSelect::ExpandSingleClicked()
{
	int boxSize=0;
	//���ͼ�㲻����ͼԪ����
	if(pLayer->geometryType()!= QgsWkbTypes::PolygonGeometry){
	   boxSize=5;
	}
	else{
	   boxSize=1;
	}

	// ���������豸����ת��Ϊ��ͼ����
	const QgsMapToPixel* transform = mCanvas->getCoordinateTransform();

	// ��ȡ��ͼ������������boxSize��Χ�ĵ�
	QPoint topLeft(mpressPoint.x() - boxSize, mpressPoint.y() - boxSize);
	QPoint bottomRight(mpressPoint.x() + boxSize, mpressPoint.y() + boxSize);

	// ����ͼ���귶Χ��ת��Ϊ��ͼ����㲢��ӵ�mRubberBand
	mRubberBand->addPoint(transform->toMapCoordinates(topLeft));
	mRubberBand->addPoint(transform->toMapCoordinates(QPoint(bottomRight.x(), topLeft.y())));
	mRubberBand->addPoint(transform->toMapCoordinates(bottomRight));
	mRubberBand->addPoint(transform->toMapCoordinates(QPoint(topLeft.x(), bottomRight.y())));

}


////��ָ�����豸��������ת���ɵ�ͼ��������
//void MapToolSelect::SetRubberBand(QRect &selectRect,QgsRubberBand *pRubber)
//{
//    //�õ���ǰ����任����
//	const QgsMapToPixel* transform=mCanvas->getCoordinateTransform();
//    //�������豸����ת���ɵ�ͼ����
//	QgsPoint ll(transform->toMapCoordinates(selectRect.left(),selectRect.bottom()));
//    QgsPoint ur(transform->toMapCoordinates(selectRect.right(),selectRect.top()));
//    pRubber->reset();
//    //�������4���ǵ���ӵ�QgsRubberBand������
//	pRubber->addPoint(ll,false );
//    pRubber->addPoint(QgsPoint(ur.x(), ll.y()),false );
//    pRubber->addPoint(ur,false );
//    pRubber->addPoint(QgsPoint( ll.x(), ur.y() ),true );
//}

//ѡ�񼸺�����
// selectGeometry:ѡ��������ѡ�񼸺���
// doContains:ѡ��������Ƿ������ѡ�񼸺����ڲ�
// singleSelect:����ѡ���ѡ�񼸺������������
void MapToolSelect::SetSelectFeatures(QgsGeometry& selectGeometry,bool doContains,
										 bool doDifference,bool singleSelect) 
{
    //���ѡ�񼸺��岻�Ƕ����
    if(selectGeometry.type()!= QgsWkbTypes::PolygonGeometry){
      return;
    }
    QgsGeometry selectGeomTrans(selectGeometry);
    //�趨ѡ�񼸺��������ϵ��ͼ�������ϵһ��


	try {
		if (pLayer)
		{
			//����ͼ�������ϵ�任��ͼ������ϵ
			QgsCoordinateTransform ct(mCanvas->mapSettings().destinationCrs(), pLayer->crs(), QgsProject::instance());
			//�趨�����������ϵ��ͼ������ϵһ��
			selectGeomTrans.transform(ct);
		}
		else
		{
			return;
		}
	}
	//�����쳣���׳��쳣
	catch (QgsCsException &cse) {
		Q_UNUSED(cse);
		//catch exception for 'invalid' point and leave existing selection unchanged
		QMessageBox::warning(mCanvas, QObject::tr("CRS Exception"),
			QObject::tr("Selection extends beyond layer's coordinate system."));
		return;
	}
    
    //���ù��
	//QApplication::setOverrideCursor(Qt::WaitCursor);
    //ѡ���ѡ�񼸺����ཻ���ڼ������ڲ�������
	QgsRectangle rect = selectGeomTrans.boundingBox();
	pLayer->selectByRect(rect);
	int nh=pLayer->selectedFeatureCount();
    QgsFeatureIds newSelectedFeatures;
	// ��ȡ����������
	QgsFeatureIterator it = pLayer->getFeatures();
	QgsFeature f;
    int closestFeatureId=0;
    bool foundSingleFeature=false;
    double closestFeatureDist=std::numeric_limits<double>::max();
	//�õ���ǰѡ�������
	while(it.nextFeature(f)){
       QgsGeometry g=f.geometry();
	   //g�Ƿ������selectGeomTrans�������ڲ�
	   if(doContains && !selectGeomTrans.contains(g)){
           continue;
       }
       if(singleSelect){ //ѡ��ͼ��������������
          foundSingleFeature=true;
          //��������������֮��ľ���
		  double distance=g.distance(selectGeomTrans);
          if(distance<=closestFeatureDist){
              closestFeatureDist=distance;
              //��������ѡ�񼸺���������id
			  closestFeatureId=f.id();
          }
       }
       else{ //�洢����Ҫ�������id
          newSelectedFeatures.insert(f.id());
       }
   }
   //ȷ����ѡ�񼸺������������id
   if(singleSelect && foundSingleFeature){
       newSelectedFeatures.insert(closestFeatureId);
   }
   //�������ctrl��,ѡ��������
   if(doDifference){
       //�õ�����ѡ��������id
	   layerSelectedFeatures = pLayer->selectedFeatureIds();
       QgsFeatureIds::const_iterator i=newSelectedFeatures.constEnd();
       while(i!=newSelectedFeatures.constBegin()){
           --i;
		   if(layerSelectedFeatures.contains(*i)){
               layerSelectedFeatures.remove( *i );
           }
           else{
               layerSelectedFeatures.insert( *i );
           }
       }
  }
  else{
      layerSelectedFeatures=newSelectedFeatures;
  }
  //�趨ѡ�������
  pLayer->selectByIds(layerSelectedFeatures);
}
//ѡ�񼸺�����,����ѡ����״��������
//selectGeometry:ѡ�񼸺���
void MapToolSelect::SetSelectFeatures(QgsGeometry& selectGeometry,bool doDifference) 
{
 //   //���ѡ�񼸺��岻�Ƕ����
 //   if(selectGeometry->type()!=QGis::Polygon){
 //     return;
 //   }
 //   QgsGeometry selectGeomTrans(*selectGeometry);
 //   //�趨ѡ�񼸺��������ϵ��ͼ�������ϵһ��
	//if(mCanvas->mapRenderer()->hasCrsTransformEnabled()){
 //      try{
 //          //����ͼ�������ϵ�任��ͼ������ϵ
	//	   QgsCoordinateTransform ct(mCanvas->mapRenderer()->destinationSrs(),pLayer->crs());
 //          //�趨�����������ϵ��ͼ������ϵһ��
	//	   selectGeomTrans.transform(ct);
 //      }
 //      //�����쳣���׳��쳣
	//   catch(QgsCsException &cse){
 //         Q_UNUSED(cse);
 //         //catch exception for 'invalid' point and leave existing selection unchanged
 //         QMessageBox::warning(mCanvas, QObject::tr("CRS Exception"),
 //                           QObject::tr( "Selection extends beyond layer's coordinate system." ) );
	//	  return;
 //      }
 //   }
 //   //���ù��
	////QApplication::setOverrideCursor(Qt::WaitCursor);
 //   //ѡ���ѡ�񼸺����ཻ���ڼ������ڲ�������
	//pLayer->select(QgsAttributeList(),selectGeomTrans.boundingBox(),true,true);
	//int nh=pLayer->selectedFeatureCount();
 //   QgsFeatureIds newSelectedFeatures;
 //   QgsFeature f;
 //   
	//int  p=0;
	////�õ���ǰѡ�������
	//while(pLayer->nextFeature(f)){
	//	p++;
 //      QgsGeometry* g=f.geometry();
 //      //ѡ��������Ƿ������ѡ�񼸺�����ڲ�
	//   //���g����selectGeomTrans������true
	//   if(!g->contains(&selectGeomTrans)){
 //          continue;
 //      }
 //      //�洢��������ͼ������id
	//   newSelectedFeatures.insert(f.id());
 //  }
 //  QgsFeatureIds layerSelectedFeatures;
 //  //�������ctrl��������ѡ��������
 //  if(doDifference){
 //      layerSelectedFeatures=pLayer->selectedFeaturesIds();
 //      QgsFeatureIds::const_iterator i = newSelectedFeatures.constEnd();
 //      while( i != newSelectedFeatures.constBegin()){
 //          --i;
 //         if( layerSelectedFeatures.contains( *i ) )
 //         {
 //            layerSelectedFeatures.remove( *i );
 //         }
 //         else
 //         {
 //            layerSelectedFeatures.insert( *i );
 //         }
 //      }
 // }
 // else{
 //    layerSelectedFeatures=newSelectedFeatures;
 // }
 // //�趨ѡ�������
 // pLayer->setSelectedFeatures(layerSelectedFeatures);
 // //QApplication::restoreOverrideCursor();
}
//�趨����״̬
void MapToolSelect::SetEnable(bool flag)
{
	StatusFlag=flag;
	if(StatusFlag){
		mCursor=Qt::CrossCursor;
	}
	else{
		mCursor=Qt::ArrowCursor;
	}
}