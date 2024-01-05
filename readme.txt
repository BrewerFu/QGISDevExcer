	//绘制折线
	connect(qtID_PolyLine,SIGNAL(triggered()),this,SLOT(DrawPolyLine()));

	//绘制工具
    pDrawLineTool=new QgsMapToolDrawLine(pMapCanvas);
	pDrawLineTool->setAction(qtID_PolyLine);


//绘制折线
void examp2::DrawPolyLine()
{
	pMapCanvas->setMapTool(pDrawLineTool);
}




//得到选择特征属性数据
void examp2::GetAttribute()
{
   QStringList strlist;
   int i,n;
   QgsSelectFeatureAttribute mAttribute;
   mAttribute.SetSelectLayer(pSelectLayer);
   strlist=mAttribute.GetSelectAttribute(tr("Name"));
   if(!strlist.isEmpty()){
	   n=strlist.size();
	   for(i=0;i<n;i++){
	     QMessageBox::about(this,tr("ok"),strlist.at(i));
	   }
   }
}
//删除当前选择的图元
void examp2::DeleteElement()
{
   QgsDeSelectFeature pDeFeature;
   pDeFeature.SetSelectLayer(pSelectLayer);
   pDeFeature.RemoveSelectFeature(); 
   pSelectTool->SetEnable(false);
   pMapCanvas->setMapTool(pSelectTool);
}