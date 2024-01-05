#include "LayerTreeViewMenuProvider.h"

DataViewerLayerTreeViewMenuProvider::DataViewerLayerTreeViewMenuProvider(QgsLayerTreeView* view, QgsMapCanvas* canvas)
	: m_view(view)
	, m_canvas(canvas)
{
}

QMenu* DataViewerLayerTreeViewMenuProvider::createContextMenu()
{
	initProvider();
	QMenu* menu = new QMenu();
	QgsLayerTreeViewDefaultActions* defaultActions = m_view->defaultActions();
	QModelIndex idx = m_view->currentIndex();

	//���û��ѡ��ͼ�������ͼ��	
	if (!idx.isValid())
	{
		menu->addAction(defaultActions->actionAddGroup(menu));
	}
	//���ѡ����ͼ���������ͼ��
	else if (QgsLayerTreeNode* node = m_view->layerTreeModel()->index2node(idx))
	{
		menu->addAction(defaultActions->actionRenameGroupOrLayer(menu));
		menu->addAction(defaultActions->actionRemoveGroupOrLayer(menu));

		//���ѡ�������ͼ��������е���,�����ѡ������
		if (QgsLayerTree::isGroup(node))
		{
			menu->addAction(defaultActions->actionZoomToGroup(m_canvas, menu));

			if (m_view->selectedNodes(true).count() > 2)
			{
				menu->addAction(defaultActions->actionGroupSelected(menu));
			}
			menu->addAction(defaultActions->actionAddGroup(menu));
		}


		//���ѡ�����ͼ��������е�ͼ��
		else if (QgsLayerTree::isLayer(node))
		{
			QgsMapLayer* layer = QgsLayerTree::toLayer(node)->layer();
			//���������ͼ��
			menu->addAction(defaultActions->actionZoomToLayer(m_canvas, menu));
			//�����ӥ����ʾ
			menu->addAction(defaultActions->actionShowInOverview(menu));


			//���ѡ�����ʸ��ͼ��
			if (layer->type() == QgsMapLayerType::VectorLayer)
			{
				//������Ϊ����
				m_SaveAsFileVecor->setData(QVariant::fromValue(layer));
				menu->addAction(m_SaveAsFileVecor);

				m_SymbolizeAction->setData(QVariant::fromValue(layer)); // ����ͼ������
				//��ӷ��Ż�����
				menu->addAction(m_SymbolizeAction);

				m_AttributeTableAction->setData(QVariant::fromValue(layer)); // ����ͼ������
				//������Ա�����
				menu->addAction(m_AttributeTableAction);

				m_AttributeFieldAction->setData(QVariant::fromValue(layer)); // ����ͼ������
				//��������ֶ�����
				menu->addAction(m_AttributeFieldAction);

				m_SetCurrentLayer->setData(QVariant::fromValue(layer)); // ����ͼ������
				//������õ�ǰͼ������
				menu->addAction(m_SetCurrentLayer);

				m_Projectvector->setData(QVariant::fromValue(layer));		//����ͼ������
				//�������ͶӰ����
				menu->addAction(m_Projectvector);

			}

			if (layer->type() == QgsMapLayerType::RasterLayer)
			{
				m_SaveAsFileRaster->setData(QVariant::fromValue(layer));

				//������Ϊ����
				menu->addAction(m_SaveAsFileRaster);

				m_ProjectRaster->setData(QVariant::fromValue(layer));		//����ͼ������
				//�������ͶӰ����
				menu->addAction(m_ProjectRaster);
			}
		}
	}

	return menu;
}

void DataViewerLayerTreeViewMenuProvider::initProvider()
{
	m_SymbolizeAction = new QAction(QStringLiteral("���Ż�����"));
	m_AttributeTableAction = new QAction(QStringLiteral("�����Ա�"));
	m_AttributeFieldAction = new QAction(QStringLiteral("�������ֶ�"));
	m_SetCurrentLayer = new QAction(QStringLiteral("���û���ñ༭"));
	m_SaveAsFileVecor = new QAction(QStringLiteral("���Ϊ"));
	m_SaveAsFileRaster = new QAction(QStringLiteral("���Ϊ"));
	m_Projectvector = new QAction(QStringLiteral("ͶӰ�任"));
	m_ProjectRaster = new QAction(QStringLiteral("ͶӰ�任"));
	connect(m_SymbolizeAction, &QAction::triggered, this, &DataViewerLayerTreeViewMenuProvider::openSymbolizeWdt);
	connect(m_AttributeTableAction, &QAction::triggered, this, &DataViewerLayerTreeViewMenuProvider::openAttributeTable);
	connect(m_AttributeFieldAction, &QAction::triggered, this, &DataViewerLayerTreeViewMenuProvider::openAttributeField);
	connect(m_SetCurrentLayer, &QAction::triggered, this, &DataViewerLayerTreeViewMenuProvider::setActivateMode);
	connect(m_SaveAsFileVecor, &QAction::triggered, this, &DataViewerLayerTreeViewMenuProvider::saveAsFileVector);
	connect(m_SaveAsFileRaster, &QAction::triggered, this, &DataViewerLayerTreeViewMenuProvider::saveAsFileRaster);
	connect(m_Projectvector, &QAction::triggered, this, &DataViewerLayerTreeViewMenuProvider::openProjectVectorDlg);
	connect(m_ProjectRaster, &QAction::triggered, this, &DataViewerLayerTreeViewMenuProvider::openProjectRasterDlg);
}

void DataViewerLayerTreeViewMenuProvider::openSymbolizeWdt()
{
	QAction* action = qobject_cast<QAction*>(sender());	// ��ȡ�źŷ�����
	if (action)
	{
		QgsVectorLayer* layer = qobject_cast<QgsVectorLayer*>(action->data().value<QgsMapLayer*>());	// ��ȡͼ��
		if (layer)
		{
			QgsSymbol* symbol = nullptr;
			symbol = QgsSymbol::defaultSymbol(layer->geometryType());
			// ��������ѡ��Ի���
			QgsSymbolSelectorDialog symbolDialog(symbol, QgsStyle::defaultStyle(), layer, nullptr);
			if (symbolDialog.exec() == QDialog::Accepted)
			{
				// ��ȡѡ��ķ���
				QgsSymbol* selectedSymbol = symbolDialog.symbol();
				QgsSingleSymbolRenderer* renderer = new QgsSingleSymbolRenderer(selectedSymbol);
				layer->setRenderer(renderer);
				layer->triggerRepaint();
			}
		}
	}
}

 void DataViewerLayerTreeViewMenuProvider::openAttributeTable()
 {
 	QAction* action = qobject_cast<QAction*>(sender());
 	if (action)
 	{
 		QgsVectorLayer* layer = qobject_cast<QgsVectorLayer*>(action->data().value<QgsMapLayer*>());
 		if (layer)
 		{
			// �������Ա�Ի���
			QgsAttributeTableDialog* dlg = new QgsAttributeTableDialog(layer);
			dlg->show();


 			//QgsVectorLayerCache* lc = new QgsVectorLayerCache(layer, layer->featureCount());

 			//QgsAttributeTableView* tv = new QgsAttributeTableView();
 			//QgsAttributeTableModel* tm = new QgsAttributeTableModel(lc);
 			//tm->loadLayer(); //һ����Ҫ���ǣ�����mode1����û��ͼ�����������

 			//QgsAttributeTableFilterModel* tfm = new QgsAttributeTableFilterModel(m_canvas, tm, tm);
 			//tfm->setFilterMode(QgsAttributeTableFilterModel::ShowAll);
 			//tv->setModel(tfm);

 			//tv->show();
 		}
 	}
 }

 void DataViewerLayerTreeViewMenuProvider::openAttributeField()
 {
	 QAction* action = qobject_cast<QAction*>(sender());
	 if (action)
	 {
		 QgsVectorLayer* layer = qobject_cast<QgsVectorLayer*>(action->data().value<QgsMapLayer*>());
		 if (layer)
		 {
			 //��ʾ���Խṹ�ֶ�
			 QgsSourceFieldsProperties* pWidget = new QgsSourceFieldsProperties(layer, nullptr);
			 pWidget->loadRows();
			 pWidget->show();
		 }
	 }
 }

 void DataViewerLayerTreeViewMenuProvider::setActivateMode()
 {
	 QAction* action = qobject_cast<QAction*>(sender());
	 if (action)
	 {
		 QgsVectorLayer* layer = qobject_cast<QgsVectorLayer*>(action->data().value<QgsMapLayer*>());
		 if (layer)
		 {
			 m_canvas->setCurrentLayer(layer);
			 // ���伤���ź�
			 emit activeMode();
		 }
	 }

 }

 void  DataViewerLayerTreeViewMenuProvider::saveAsFileVector()
 {
	 QAction* action = qobject_cast<QAction*>(sender());
	 if (action)
	 {
		 QgsVectorLayer* layer = qobject_cast<QgsVectorLayer*>(action->data().value<QgsMapLayer*>());
		 if (layer)
		 {
			 // �������Ϊ�Ի���
			QgsVectorLayerSaveAsDialog dialog(layer);
			if (dialog.exec() == QDialog::Accepted) {
				QString filename = dialog.filename();
				QString encoding = dialog.encoding();
				QString format = dialog.format();
				QgsCoordinateReferenceSystem crs(dialog.crs());	//��ȡͶӰ
				
				QgsVectorFileWriter::SaveVectorOptions options;
				options.driverName = format;
				options.fileEncoding = encoding;
				options.ct = QgsCoordinateTransform(layer->crs(), crs, QgsProject::instance());

				//�½�Writer����
				QgsVectorFileWriter* writer = new QgsVectorFileWriter(filename, encoding, layer->fields(), layer->wkbType(), crs, format);
				if (writer->hasError()) 
				{
					QMessageBox::critical(nullptr, QStringLiteral("Error"), writer->errorMessage());
				} 
				else 
				{
					QgsFeatureIterator it = layer->getFeatures();
					QgsFeature f;
					while (it.nextFeature(f)) 
					{
						writer->addFeature(f);
					}
				}
				delete writer;
			}
		 }
	 }
 }

 void DataViewerLayerTreeViewMenuProvider::saveAsFileRaster()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (action)
    {
        QgsRasterLayer* layer = qobject_cast<QgsRasterLayer*>(action->data().value<QgsMapLayer*>());
        if (layer)
        {	
			// �����Ϊ�Ի���
            QString filename = QFileDialog::getSaveFileName(nullptr, tr("Save As"), "", tr("GeoTIFF (*.tif)"));
            if (!filename.isEmpty())
            {
                QgsRasterFileWriter writer(filename);
                QgsRasterPipe* pipe = layer->pipe();
                if (writer.writeRaster(pipe, layer->width(), layer->height(), layer->extent(), layer->crs(), QgsProject::instance()->transformContext()) == QgsRasterFileWriter::NoError)
                {
                    qDebug() << "Raster layer saved successfully.";
                }
                else
                {
                    qDebug() << "Failed to save raster layer.";
                }
            }
        }
    }
}


 void DataViewerLayerTreeViewMenuProvider::openProjectVectorDlg()
 {
	 QAction* action = qobject_cast<QAction*>(sender());
	 if (action)
	 {

		 QgsVectorLayer* layer = qobject_cast<QgsVectorLayer*>(action->data().value<QgsMapLayer*>());

		 if (layer)
		 {
			 QgsProjectionSelectionDialog dialog;
			 if (dialog.exec())
			 {
				 QgsCoordinateReferenceSystem crs = dialog.crs();
				 // ��������任����
				 QgsCoordinateTransform ct(layer->crs(), crs, QgsProject::instance());

				 // ����ͼ�����������
				 QgsFeatureIterator it = layer->getFeatures();
				 QgsFeature feature;
				 while (it.nextFeature(feature))
				 {
					 // ��ȡ�����ļ�����
					 QgsGeometry geom = feature.geometry();

					 // ��������任
					 try
					 {
						 geom.transform(ct);
					 }
					 catch (QgsCsException& e)
					 {
						 qDebug() << "Coordinate transform error:" << e.what();
						 continue;
					 }

					 // ���¼�������
					 feature.setGeometry(geom);
					 layer->updateFeature(feature);
				 }
			 }
		 }
	 }
 }

 void DataViewerLayerTreeViewMenuProvider::openProjectRasterDlg()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (action)
    {
        QgsRasterLayer* layer = qobject_cast<QgsRasterLayer*>(action->data().value<QgsMapLayer*>());
        if (layer)
        {
            QgsProjectionSelectionDialog dialog;
            if (dialog.exec())
            {
                QgsCoordinateReferenceSystem crs = dialog.crs();

                // ��������任����
                QgsCoordinateTransform ct(layer->crs(), crs, QgsProject::instance());

                // ����դ��ͶӰ��
                QgsRasterProjector projector;
                projector.setCrs(layer->crs(), crs, QgsProject::instance()->transformContext());

                // ��ȡդ���ṩ��
                QgsRasterDataProvider* provider = layer->dataProvider();

                // ��ȡդ������
                QgsRasterBlock* block = provider->block(1, provider->extent(), provider->xSize(), provider->ySize());

                // ����ͶӰ�任
                QgsRasterBlock* projectedBlock = projector.block(1, provider->extent(), provider->xSize(), provider->ySize());


				// �����µ�դ��ܵ��������ͶӰ�任�������
				QgsRasterPipe* pipe = new QgsRasterPipe();
				pipe->set(provider->clone());
				pipe->insert(0, &projector);

				//����Ϊ�µ�դ���ļ�
				QString filename = QFileDialog::getSaveFileName(nullptr, tr("Save As"), "", tr("GeoTIFF (*.tif)"));
				if (!filename.isEmpty())
				{
					QgsRasterFileWriter writer(filename);
					QgsRasterPipe* pipe = layer->pipe();
					if (writer.writeRaster(pipe, layer->width(), layer->height(), layer->extent(), layer->crs(), QgsProject::instance()->transformContext()) == QgsRasterFileWriter::NoError)
					{
						qDebug() << "Raster layer saved successfully.";
					}
					else
					{
						qDebug() << "Failed to save raster layer.";
					}
				}

                // �ͷ���Դ
                delete block;
                delete projectedBlock;
            }
        }
    }
}