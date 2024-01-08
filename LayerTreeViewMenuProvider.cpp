#include "LayerTreeViewMenuProvider.h"
#include <QgsRasterShader.h>
#include <QgsSingleBandPseudoColorRenderer.h>
#include <QgsRasterRendererWidget.h>
#include <QgsGui.h>
#include <QgsSingleBandPseudoColorRendererWidget.h>
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

	//如果没有选中图层管理器图层	
	if (!idx.isValid())
	{
		menu->addAction(defaultActions->actionAddGroup(menu));
	}
	//如果选中了图层管理器中图层
	else if (QgsLayerTreeNode* node = m_view->layerTreeModel()->index2node(idx))
	{
		menu->addAction(defaultActions->actionRenameGroupOrLayer(menu));
		menu->addAction(defaultActions->actionRemoveGroupOrLayer(menu));

		//如果选择则的是图层管理器中的组,添加组选择命令
		if (QgsLayerTree::isGroup(node))
		{
			menu->addAction(defaultActions->actionZoomToGroup(m_canvas, menu));

			if (m_view->selectedNodes(true).count() > 2)
			{
				menu->addAction(defaultActions->actionGroupSelected(menu));
			}
			menu->addAction(defaultActions->actionAddGroup(menu));
		}


		//如果选择的是图层管理器中的图层
		else if (QgsLayerTree::isLayer(node))
		{
			QgsMapLayer* layer = QgsLayerTree::toLayer(node)->layer();
			//添加缩放至图层
			menu->addAction(defaultActions->actionZoomToLayer(m_canvas, menu));
			//添加在鹰眼显示
			menu->addAction(defaultActions->actionShowInOverview(menu));


			//如果选择的是矢量图层
			if (layer->type() == QgsMapLayerType::VectorLayer)
			{
				//添加另存为命令
				m_SaveAsFileVecor->setData(QVariant::fromValue(layer));
				m_menu->addAction(m_SaveAsFileVecor); //给子菜单添加命令——要素另存为
				m_menu->addAction(m_LayerStyle); //给子菜单添加命令——另存为QGIS图层样式文件
				menu->addMenu(m_menu);

				m_SymbolizeAction->setData(QVariant::fromValue(layer)); // 设置图层数据
				//添加符号化命令
				menu->addAction(m_SymbolizeAction);

				m_AttributeTableAction->setData(QVariant::fromValue(layer)); // 设置图层数据
				//添加属性表命令
				menu->addAction(m_AttributeTableAction);

				m_AttributeFieldAction->setData(QVariant::fromValue(layer)); // 设置图层数据
				//添加属性字段命令
				menu->addAction(m_AttributeFieldAction);

				m_SetCurrentLayer->setData(QVariant::fromValue(layer)); // 设置图层数据
				//添加启用当前图层命令
				menu->addAction(m_SetCurrentLayer);

				m_Projectvector->setData(QVariant::fromValue(layer));		//设置图层数据
				//添加启用投影命令
				menu->addAction(m_Projectvector);

			}

			if (layer->type() == QgsMapLayerType::RasterLayer)
			{
				m_SaveAsFileRaster->setData(QVariant::fromValue(layer));

				//添加另存为命令
				menu->addAction(m_SaveAsFileRaster);

				m_ProjectRaster->setData(QVariant::fromValue(layer));		//设置图层数据
				//添加启用投影命令
				menu->addAction(m_ProjectRaster);

				//从这里添加栅格图层分层赋色
				m_LayeredColoring->setData(QVariant::fromValue(layer));   //设置栅格图层数据
				//添加栅格图层分层赋色命令
				menu->addAction(m_LayeredColoring);
			}
		}
	}

	return menu;
}

void DataViewerLayerTreeViewMenuProvider::initProvider()
{
	m_SymbolizeAction = new QAction(QStringLiteral("符号化窗口"));
	m_AttributeTableAction = new QAction(QStringLiteral("打开属性表"));
	m_AttributeFieldAction = new QAction(QStringLiteral("打开属性字段"));
	m_SetCurrentLayer = new QAction(QStringLiteral("启用或禁用编辑"));
	m_SaveAsFileVecor = new QAction(QStringLiteral("要素另存为"));
	m_SaveAsFileRaster = new QAction(QStringLiteral("另存为"));
	m_Projectvector = new QAction(QStringLiteral("投影变换"));
	m_ProjectRaster = new QAction(QStringLiteral("投影变换"));
	m_LayeredColoring = new QAction(QStringLiteral("栅格分层赋色"));
	m_menu = new QMenu(QStringLiteral("导出")); //子菜单
	m_LayerStyle = new QAction(QStringLiteral("另存为QGIS图层样式文件"));
	
	connect(m_SymbolizeAction, &QAction::triggered, this, &DataViewerLayerTreeViewMenuProvider::openSymbolizeWdt);
	connect(m_AttributeTableAction, &QAction::triggered, this, &DataViewerLayerTreeViewMenuProvider::openAttributeTable);
	connect(m_AttributeFieldAction, &QAction::triggered, this, &DataViewerLayerTreeViewMenuProvider::openAttributeField);
	connect(m_SetCurrentLayer, &QAction::triggered, this, &DataViewerLayerTreeViewMenuProvider::setActivateMode);
	connect(m_SaveAsFileVecor, &QAction::triggered, this, &DataViewerLayerTreeViewMenuProvider::saveAsFileVector);
	connect(m_SaveAsFileRaster, &QAction::triggered, this, &DataViewerLayerTreeViewMenuProvider::saveAsFileRaster);
	connect(m_LayerStyle, &QAction::triggered, this, &DataViewerLayerTreeViewMenuProvider::saveAsLayerStyle);//另存为图层样式文件的信号和槽
	connect(m_Projectvector, &QAction::triggered, this, &DataViewerLayerTreeViewMenuProvider::openProjectVectorDlg);
	connect(m_ProjectRaster, &QAction::triggered, this, &DataViewerLayerTreeViewMenuProvider::openProjectRasterDlg);
	connect(m_LayeredColoring, &QAction::triggered, this, &DataViewerLayerTreeViewMenuProvider::applyLayeredColoring);
}

void DataViewerLayerTreeViewMenuProvider::openSymbolizeWdt()
{
	QAction* action = qobject_cast<QAction*>(sender());	// 获取信号发送者
	if (action)
	{
		QgsVectorLayer* layer = qobject_cast<QgsVectorLayer*>(action->data().value<QgsMapLayer*>());	// 获取图层
		if (layer)
		{
			QgsSymbol* symbol = nullptr;
			symbol = QgsSymbol::defaultSymbol(layer->geometryType());
			// 创建符号选择对话框
			QgsSymbolSelectorDialog symbolDialog(symbol, QgsStyle::defaultStyle(), layer, nullptr);
			if (symbolDialog.exec() == QDialog::Accepted)
			{
				// 获取选择的符号
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
			// 创建属性表对话框
			QgsAttributeTableDialog* dlg = new QgsAttributeTableDialog(layer);
			dlg->show();


 			//QgsVectorLayerCache* lc = new QgsVectorLayerCache(layer, layer->featureCount());

 			//QgsAttributeTableView* tv = new QgsAttributeTableView();
 			//QgsAttributeTableModel* tm = new QgsAttributeTableModel(lc);
 			//tm->loadLayer(); //一定不要忘记，否则mode1里面没有图层的属性数据

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
			 //显示属性结构字段
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
			 // 发射激活信号
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
			 // 创建另存为对话框
			QgsVectorLayerSaveAsDialog dialog(layer);
			if (dialog.exec() == QDialog::Accepted) {
				QString filename = dialog.filename();
				QString encoding = dialog.encoding();
				QString format = dialog.format();
				QgsCoordinateReferenceSystem crs(dialog.crs());	//获取投影
				
				QgsVectorFileWriter::SaveVectorOptions options;
				options.driverName = format;
				options.fileEncoding = encoding;
				options.ct = QgsCoordinateTransform(layer->crs(), crs, QgsProject::instance());

				//新建Writer对象
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
			// 打开另存为对话框
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
				 // 创建坐标变换对象
				 QgsCoordinateTransform ct(layer->crs(), crs, QgsProject::instance());

				 // 遍历图层的所有特征
				 QgsFeatureIterator it = layer->getFeatures();
				 QgsFeature feature;
				 while (it.nextFeature(feature))
				 {
					 // 获取特征的几何体
					 QgsGeometry geom = feature.geometry();

					 // 进行坐标变换
					 try
					 {
						 geom.transform(ct);
					 }
					 catch (QgsCsException& e)
					 {
						 qDebug() << "Coordinate transform error:" << e.what();
						 continue;
					 }

					 // 更新几何数据
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

                // 创建坐标变换对象
                QgsCoordinateTransform ct(layer->crs(), crs, QgsProject::instance());

                // 创建栅格投影器
                QgsRasterProjector projector;
                projector.setCrs(layer->crs(), crs, QgsProject::instance()->transformContext());

                // 获取栅格提供者
                QgsRasterDataProvider* provider = layer->dataProvider();

                // 获取栅格数据
                QgsRasterBlock* block = provider->block(1, provider->extent(), provider->xSize(), provider->ySize());

                // 进行投影变换
                QgsRasterBlock* projectedBlock = projector.block(1, provider->extent(), provider->xSize(), provider->ySize());


				// 创建新的栅格管道，并添加投影变换后的数据
				QgsRasterPipe* pipe = new QgsRasterPipe();
				pipe->set(provider->clone());
				pipe->insert(0, &projector);

				//保存为新的栅格文件
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

                // 释放资源
                delete block;
                delete projectedBlock;
            }
        }
    }
}


 // 实现栅格分层赋色的槽函数
 void DataViewerLayerTreeViewMenuProvider::applyLayeredColoring() {
	 // 获取当前选中的图层
	 QgsMapLayer* layer = m_view->currentLayer();
	 if (!layer || layer->type() != QgsMapLayerType::RasterLayer) {
		 QMessageBox::warning(nullptr, tr("Warning"), tr("Please select a raster layer."));
		 return;
	 }

	 QgsRasterLayer* rasterLayer = qobject_cast<QgsRasterLayer*>(layer);
	 if (!rasterLayer)
		 return;

	 // 创建栅格渲染器小部件
	 QgsSingleBandPseudoColorRendererWidget* rendererWidget = new QgsSingleBandPseudoColorRendererWidget(rasterLayer);
	 //QgsRasterRendererWidget* rendererWidget = new QgsRasterRendererWidget(rasterLayer, rasterLayer->extent());

	 // 创建对话框并添加栅格渲染器小部件
	 QDialog dialog;
	 dialog.setLayout(new QVBoxLayout);
	 dialog.layout()->addWidget(rendererWidget);

	 // 如果用户点击了"OK"按钮，就从小部件中获取新的渲染器，并应用到栅格图层上
	 QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
	 dialog.layout()->addWidget(buttonBox);
	 connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
	 connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

	 if (dialog.exec() == QDialog::Accepted) {
		 QgsRasterRenderer* newRenderer = rendererWidget->renderer();
		 if (newRenderer) {
			 rasterLayer->setRenderer(newRenderer);
			 rasterLayer->triggerRepaint();
		 }
	 }
 }
	 
 void DataViewerLayerTreeViewMenuProvider::saveAsLayerStyle() {
	 QAction* action = qobject_cast<QAction*>(sender());	// 获取信号发送者
	 if (action)
	 {
		 QgsVectorLayer* layer = qobject_cast<QgsVectorLayer*>(action->data().value<QgsMapLayer*>());	// 获取图层
		 if (layer)
		 {
			 QgsSymbol* symbol = nullptr;
			 symbol = QgsSymbol::defaultSymbol(layer->geometryType());
			 // 创建符号选择对话框
			 QgsSymbolSelectorDialog symbolDialog(symbol, QgsStyle::defaultStyle(), layer, nullptr);
			 if (symbolDialog.exec() == QDialog::Accepted)
			 {
				 // 获取选择的符号
				 QgsSymbol* selectedSymbol = symbolDialog.symbol();
				 QgsSingleSymbolRenderer* renderer = new QgsSingleSymbolRenderer(selectedSymbol);
				 layer->setRenderer(renderer);
				 layer->triggerRepaint();
			 }
		 }
	 }
 }