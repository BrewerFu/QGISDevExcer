/**************************选择图层要素工具*********************
*****************************************************************/
#include "MapToolSelect.h"
#include <QgsProject.h>

//构造和析构函数
MapToolSelect::MapToolSelect(QgsMapCanvas *Mapcanvas):QgsMapTool(Mapcanvas)
{
   pLayer=NULL;
   mRubberBand = new QgsRubberBand(Mapcanvas, QgsWkbTypes::PolygonGeometry);

   // 设置颜色和宽度
   mRubberBand->setColor(QColor(255, 0, 0));
   mRubberBand->setWidth(2);

   mCursor=Qt::ArrowCursor;
   mCanvas=Mapcanvas;
   StatusFlag=true;
}

MapToolSelect::~MapToolSelect(void)
{

}
//设置当前被选择(活动)的图层
void MapToolSelect::SetSelectLayer(QgsVectorLayer *Layer)
{
   pLayer=Layer;
}

void MapToolSelect::canvasPressEvent(QgsMapMouseEvent* e)
{

	//得到产生事件的按钮信息
	Qt::MouseButton mButton = e->button();
	//如果不是左按钮返回
	if (mButton != Qt::MouseButton::LeftButton) {
		return;
	}


	// 记录鼠标点击的位置
	mpressPoint = e->mapPoint();

	// 添加一个点到 QgsRubberBand 对象
	mRubberBand->addPoint(mpressPoint);

	//记录一次点击开始
	isClicked = true;
}


void MapToolSelect::canvasMoveEvent(QgsMapMouseEvent* e)
{
	
	if (isClicked)
	{
		// 更新 QgsRubberBand 对象的形状
		mRubberBand->reset(QgsWkbTypes::PolygonGeometry);
		mRubberBand->addPoint(mpressPoint);
		mRubberBand->addPoint(QgsPointXY(mpressPoint.x(), e->mapPoint().y()));
		mRubberBand->addPoint(e->mapPoint());
		mRubberBand->addPoint(QgsPointXY(e->mapPoint().x(), mpressPoint.y()));
		//首尾相连
		mRubberBand->addPoint(mpressPoint);
	}
}

//鼠标按钮释放时,选择包含鼠标位置的图元
void MapToolSelect::canvasReleaseEvent(QgsMapMouseEvent * e )
{
	//一次点击结束
	isClicked = false;

	//如果只是一次点击而没有拖动
	if (e->mapPoint() == mpressPoint)
	{
		ExpandSingleClicked();
	}

   if(mCanvas==NULL){
	   return;
   }
   if(pLayer==NULL){
	   QMessageBox::about(mCanvas,QString::fromLocal8Bit("警告"),QString::fromLocal8Bit("请选择图层"));
	   return;
   }
   if(StatusFlag==false){
	   return;
   }

   //得到产生事件的按钮信息
   Qt::MouseButton mButton=e->button();
   //如果不是左按钮返回
   if(mButton!=Qt::MouseButton::LeftButton){
	   return;
   }


   QgsGeometry selectGeom = mRubberBand->asGeometry();
   //如果几何无效，则返回
   if (selectGeom.isNull()) {
	   return;
   }

   QgsRectangle rect = selectGeom.boundingBox();


   //确定是否按下ctrl键
   bool doDifference=e->modifiers()&Qt::ControlModifier ? true:false;
   //在图层中选择最靠近几何对象的特征
   SetSelectFeatures(selectGeom,false,doDifference,true);

   //rubberBand.reset();

   // 清除 QgsRubberBand 对象
   mRubberBand->reset(QgsWkbTypes::PolygonGeometry);
	
}


//提取鼠标位置一定范围作为选择区域
void MapToolSelect::ExpandSingleClicked()
{
	int boxSize=0;
	//如果图层不是面图元类型
	if(pLayer->geometryType()!= QgsWkbTypes::PolygonGeometry){
	   boxSize=5;
	}
	else{
	   boxSize=1;
	}

	// 将点击点的设备坐标转换为地图坐标
	const QgsMapToPixel* transform = mCanvas->getCoordinateTransform();

	// 获取视图坐标上下左右boxSize范围的点
	QPoint topLeft(mpressPoint.x() - boxSize, mpressPoint.y() - boxSize);
	QPoint bottomRight(mpressPoint.x() + boxSize, mpressPoint.y() + boxSize);

	// 将视图坐标范围点转换为地图坐标点并添加到mRubberBand
	mRubberBand->addPoint(transform->toMapCoordinates(topLeft));
	mRubberBand->addPoint(transform->toMapCoordinates(QPoint(bottomRight.x(), topLeft.y())));
	mRubberBand->addPoint(transform->toMapCoordinates(bottomRight));
	mRubberBand->addPoint(transform->toMapCoordinates(QPoint(topLeft.x(), bottomRight.y())));

}


////将指定的设备坐标区域转换成地图坐标区域
//void MapToolSelect::SetRubberBand(QRect &selectRect,QgsRubberBand *pRubber)
//{
//    //得到当前坐标变换对象
//	const QgsMapToPixel* transform=mCanvas->getCoordinateTransform();
//    //将区域设备坐标转换成地图坐标
//	QgsPoint ll(transform->toMapCoordinates(selectRect.left(),selectRect.bottom()));
//    QgsPoint ur(transform->toMapCoordinates(selectRect.right(),selectRect.top()));
//    pRubber->reset();
//    //将区域的4个角点添加到QgsRubberBand对象中
//	pRubber->addPoint(ll,false );
//    pRubber->addPoint(QgsPoint(ur.x(), ll.y()),false );
//    pRubber->addPoint(ur,false );
//    pRubber->addPoint(QgsPoint( ll.x(), ur.y() ),true );
//}

//选择几何特征
// selectGeometry:选择特征的选择几何体
// doContains:选择的特征是否包含在选择几何体内部
// singleSelect:仅仅选择和选择几何体最靠近的特征
void MapToolSelect::SetSelectFeatures(QgsGeometry& selectGeometry,bool doContains,
										 bool doDifference,bool singleSelect) 
{
    //如果选择几何体不是多边形
    if(selectGeometry.type()!= QgsWkbTypes::PolygonGeometry){
      return;
    }
    QgsGeometry selectGeomTrans(selectGeometry);
    //设定选择几何体的坐标系和图层的坐标系一致


	try {
		if (pLayer)
		{
			//将地图绘板坐标系变换到图层坐标系
			QgsCoordinateTransform ct(mCanvas->mapSettings().destinationCrs(), pLayer->crs(), QgsProject::instance());
			//设定几何体的坐标系和图层坐标系一致
			selectGeomTrans.transform(ct);
		}
		else
		{
			return;
		}
	}
	//对于异常点抛出异常
	catch (QgsCsException &cse) {
		Q_UNUSED(cse);
		//catch exception for 'invalid' point and leave existing selection unchanged
		QMessageBox::warning(mCanvas, QObject::tr("CRS Exception"),
			QObject::tr("Selection extends beyond layer's coordinate system."));
		return;
	}
    
    //设置光标
	//QApplication::setOverrideCursor(Qt::WaitCursor);
    //选择和选择几何体相交或在几何体内部的特征
	QgsRectangle rect = selectGeomTrans.boundingBox();
	pLayer->selectByRect(rect);
	int nh=pLayer->selectedFeatureCount();
    QgsFeatureIds newSelectedFeatures;
	// 获取特征迭代器
	QgsFeatureIterator it = pLayer->getFeatures();
	QgsFeature f;
    int closestFeatureId=0;
    bool foundSingleFeature=false;
    double closestFeatureDist=std::numeric_limits<double>::max();
	//得到当前选择的特征
	while(it.nextFeature(f)){
       QgsGeometry g=f.geometry();
	   //g是否包含在selectGeomTrans几何体内部
	   if(doContains && !selectGeomTrans.contains(g)){
           continue;
       }
       if(singleSelect){ //选择和几何体最靠近的特征
          foundSingleFeature=true;
          //计算两个几何体之间的距离
		  double distance=g.distance(selectGeomTrans);
          if(distance<=closestFeatureDist){
              closestFeatureDist=distance;
              //计算出最靠近选择几何体特征的id
			  closestFeatureId=f.id();
          }
       }
       else{ //存储符合要求的特征id
          newSelectedFeatures.insert(f.id());
       }
   }
   //确定和选择几何体最靠近特征的id
   if(singleSelect && foundSingleFeature){
       newSelectedFeatures.insert(closestFeatureId);
   }
   //如果按下ctrl键,选择多个特征
   if(doDifference){
       //得到所有选择特征的id
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
  //设定选择的特征
  pLayer->selectByIds(layerSelectedFeatures);
}
//选择几何特征,用于选择面状几何特征
//selectGeometry:选择几何体
void MapToolSelect::SetSelectFeatures(QgsGeometry& selectGeometry,bool doDifference) 
{
 //   //如果选择几何体不是多边形
 //   if(selectGeometry->type()!=QGis::Polygon){
 //     return;
 //   }
 //   QgsGeometry selectGeomTrans(*selectGeometry);
 //   //设定选择几何体的坐标系和图层的坐标系一致
	//if(mCanvas->mapRenderer()->hasCrsTransformEnabled()){
 //      try{
 //          //将地图绘板坐标系变换到图层坐标系
	//	   QgsCoordinateTransform ct(mCanvas->mapRenderer()->destinationSrs(),pLayer->crs());
 //          //设定几何体的坐标系和图层坐标系一致
	//	   selectGeomTrans.transform(ct);
 //      }
 //      //对于异常点抛出异常
	//   catch(QgsCsException &cse){
 //         Q_UNUSED(cse);
 //         //catch exception for 'invalid' point and leave existing selection unchanged
 //         QMessageBox::warning(mCanvas, QObject::tr("CRS Exception"),
 //                           QObject::tr( "Selection extends beyond layer's coordinate system." ) );
	//	  return;
 //      }
 //   }
 //   //设置光标
	////QApplication::setOverrideCursor(Qt::WaitCursor);
 //   //选择和选择几何体相交或在几何体内部的特征
	//pLayer->select(QgsAttributeList(),selectGeomTrans.boundingBox(),true,true);
	//int nh=pLayer->selectedFeatureCount();
 //   QgsFeatureIds newSelectedFeatures;
 //   QgsFeature f;
 //   
	//int  p=0;
	////得到当前选择的特征
	//while(pLayer->nextFeature(f)){
	//	p++;
 //      QgsGeometry* g=f.geometry();
 //      //选择的特征是否包含在选择几何体的内部
	//   //如果g包含selectGeomTrans，返回true
	//   if(!g->contains(&selectGeomTrans)){
 //          continue;
 //      }
 //      //存储符合条件图层特征id
	//   newSelectedFeatures.insert(f.id());
 //  }
 //  QgsFeatureIds layerSelectedFeatures;
 //  //如果按下ctrl键，可以选择多个特征
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
 // //设定选择的特征
 // pLayer->setSelectedFeatures(layerSelectedFeatures);
 // //QApplication::restoreOverrideCursor();
}
//设定工具状态
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