	//��������
	connect(qtID_PolyLine,SIGNAL(triggered()),this,SLOT(DrawPolyLine()));

	//���ƹ���
    pDrawLineTool=new QgsMapToolDrawLine(pMapCanvas);
	pDrawLineTool->setAction(qtID_PolyLine);


//��������
void examp2::DrawPolyLine()
{
	pMapCanvas->setMapTool(pDrawLineTool);
}




//�õ�ѡ��������������
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
//ɾ����ǰѡ���ͼԪ
void examp2::DeleteElement()
{
   QgsDeSelectFeature pDeFeature;
   pDeFeature.SetSelectLayer(pSelectLayer);
   pDeFeature.RemoveSelectFeature(); 
   pSelectTool->SetEnable(false);
   pMapCanvas->setMapTool(pSelectTool);
}