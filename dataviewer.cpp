#include "S3dmStyleManager.h"

#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qinputdialog.h>
#include <QDomDocument>
#include "MeasureTool.h"
#include "dataviewer.h"

#include "qgsgui.h"
#include "qgsapplication.h"
#include "qgsprocessingregistry.h"
#include "qgsprocessingguiregistry.h"
#include "qgsnativealgorithms.h"
#include "QgsProcessingAlgorithmConfigurationWidget.h"
#include "qgsprocessingalgorithmdialogbase.h"
#include "qgsogrprovider.h"
#include "qgsvectorlayereditbuffer.h"
#include "QgsAttributeTableDialog.h"
#include "QgsAttributeForm.h"
#include "qgssourcefieldsproperties.h"
#include "QgsVectorLayerProperties.h"
#include "qgisapp.h"
#include "qgsmapcanvas.h"

#include <QgsStyleManagerDialog.h>

#include <QgsEditorWidgetRegistry.h>
#include <QgsVectorLayerCache.h>
#pragma execution_character_set("utf-8")

QgsVectorLayer *createTempLayer(QgsWkbTypes::GeometryType geomType, QString crs, QgsFields fields, QgsFeatureList features)
{
	QgsVectorLayer *newLayerPtr = nullptr;

	if (geomType == QgsWkbTypes::GeometryType::PointGeometry)
	{
		QString layerDef = "Point?crs=" + crs;
		newLayerPtr = new QgsVectorLayer(layerDef, QStringLiteral("PointLayer"), QStringLiteral("memory"));
	}
	else if (geomType == QgsWkbTypes::GeometryType::LineGeometry)
	{
		QString layerDef = "LineString?crs=" + crs;
		newLayerPtr = new QgsVectorLayer(layerDef, QStringLiteral("LineLayer"), QStringLiteral("memory"));
	}
	else if (geomType == QgsWkbTypes::GeometryType::PolygonGeometry)
	{
		QString layerDef = "MultiPolygon?crs=" + crs;
		newLayerPtr = new QgsVectorLayer(layerDef, QStringLiteral("PolyLayer"), QStringLiteral("memory"));
	}

	if (newLayerPtr == nullptr && !newLayerPtr->isValid())
	{
		return newLayerPtr;
	}

	newLayerPtr->commitChanges();
	if (!newLayerPtr->isEditable())
	{
		newLayerPtr->startEditing();
	}
	QgsFields sourceFlds = fields;
	for (int i = 0; i < sourceFlds.size(); i++)
	{
		newLayerPtr->editBuffer()->addAttribute(sourceFlds[i]);
	}
	newLayerPtr->commitChanges();

	if (!newLayerPtr->isEditable())
	{
		newLayerPtr->startEditing();
	}
	newLayerPtr->editBuffer()->addFeatures(features);
	bool flag = newLayerPtr->commitChanges(); // 保存
	newLayerPtr->editingStopped();
	return newLayerPtr;
}

DataViewer::DataViewer(QWidget *parent)
	: QMainWindow(parent)
{
	m_mapCanvas = nullptr;
	// 图层管理器
	m_layerTreeView = nullptr;
	m_layerTreeCanvasBridge = nullptr;

	// 图层管理器右键菜单	/*增加*/
	m_layertreemenuProvider = nullptr;

	// 鹰眼图控件
	m_overviewCanvas = nullptr;
	// 地图缩放工具
	m_zoomInTool = nullptr;
	m_zoomOutTool = nullptr;
	// 地图浏览工具
	m_panTool = nullptr;
	// 地图量测工具
	m_measureLineTool = nullptr;
	m_measureAreaTool = nullptr;

	// 绘制工具
	m_addPointTool = nullptr;
	m_addLineTool = nullptr;
	m_addPolygonTool = nullptr;

	// 书签窗体
	m_bookmarkDlg = nullptr;

	ui.setupUi(this);

	m_mapCanvas = new QgsMapCanvas();
	this->setCentralWidget(m_mapCanvas);
	// 初始化图层管理器
	m_layerTreeView = new QgsLayerTreeView(this);
	initLayerTreeView();
	// 初始化鹰眼图控件
	m_overviewCanvas = new QgsMapOverviewCanvas(this, m_mapCanvas);
	initMapOverviewCanvas();
	// 初始化地图工具
	m_zoomInTool = new QgsMapToolZoom(m_mapCanvas, false);
	m_zoomOutTool = new QgsMapToolZoom(m_mapCanvas, true);
	m_panTool = new QgsMapToolPan(m_mapCanvas);
	m_measureLineTool = new MeasureTool(m_mapCanvas, false);
	m_measureAreaTool = new MeasureTool(m_mapCanvas, true);
	// 绘制工具
	m_pDrawLineTool = new QgsMapToolDrawLine(m_mapCanvas);

	//--------------------------------------------------------------------------------------------
	m_pSelectTool = new MapToolSelect(m_mapCanvas);
	m_pMoveTool = new MapToolMove(m_mapCanvas);
	m_pCopyThenMove = new MapToolCopyThenMove(m_mapCanvas);

	// 初始化书签窗口
	m_bookmarkDlg = new BookMarkDialog(m_mapCanvas);

	// 注册所有的进程工具（zhuyh_20221101）
	QgsApplication::processingRegistry()->addProvider(new QgsNativeAlgorithms(QgsApplication::processingRegistry()));
	QgsGui::editorWidgetRegistry()->initEditors(m_mapCanvas);

	QgisApp *app = QgisApp::instance();
	if (app == nullptr)
	{
		app = new QgisApp();
		app->setObjectName(QStringLiteral("QgisApp"));
	}
}

DataViewer::~DataViewer()
{
}
void DataViewer::on_actionOpenProject_triggered()
{
	QString filename = QFileDialog::getOpenFileName(this, QStringLiteral("选择工程文件"), "", "QGIS project (*.qgs *.qgz)");
	QFileInfo fi(filename);
	if (!fi.exists())
	{
		return;
	}

	QgsProject::instance()->read(filename);
}

void DataViewer::on_actionSaveProject_triggered()
{
	QString filename = QFileDialog::getSaveFileName(this, QStringLiteral("工程保存为"), "", "QGIS project (*.qgs)");
	QFileInfo file(filename);
	QgsProject::instance()->write(filename);
}

void DataViewer::on_actionSaveAsProject_triggered()
{
	QString filename = QFileDialog::getSaveFileName(this, QStringLiteral("工程另存为"), "", "QGIS project (*.qgs)");
	QFileInfo file(filename);
	QgsProject::instance()->write(filename);
}

void DataViewer::on_actionAddVectorData_triggered()
{
	QStringList layerPathList = QFileDialog::getOpenFileNames(this, QStringLiteral("选择矢量数据"), "", "shapefile (*.shp)");
	QList<QgsMapLayer *> layerList;
	for each (QString layerPath in layerPathList)
	{
		QFileInfo fi(layerPath);
		if (!fi.exists())
		{
			return;
		}
		QString layerBaseName = fi.baseName(); // 图层名称

		QgsVectorLayer *vecLayer = new QgsVectorLayer(layerPath, layerBaseName);
		if (!vecLayer)
		{
			return;
		}
		if (!vecLayer->isValid())
		{
			QMessageBox::information(0, "", "layer is invalid");
			return;
		}
		layerList << vecLayer;
	}

	QgsProject::instance()->addMapLayers(layerList);
	m_mapCanvas->refresh();
}
void DataViewer::on_actionAddRasterData_triggered()
{
	QStringList layerPathList = QFileDialog::getOpenFileNames(this, QStringLiteral("选择栅格数据"), "", "Image (*.img *.tif *.tiff)");
	QList<QgsMapLayer *> layerList;
	for each (QString layerPath in layerPathList)
	{
		QFileInfo fi(layerPath);
		if (!fi.exists())
		{
			return;
		}
		QString layerBaseName = fi.baseName(); // 图层名称

		QgsRasterLayer *rasterLayer = new QgsRasterLayer(layerPath, layerBaseName);
		if (!rasterLayer)
		{
			return;
		}
		if (!rasterLayer->isValid())
		{
			QMessageBox::information(0, "", "layer is invalid");
			return;
		}
		layerList << rasterLayer;
	}

	QgsProject::instance()->addMapLayers(layerList);
	m_mapCanvas->refresh();
}

void DataViewer::on_actionAddWmsLayer_triggered()
{
}

void DataViewer::on_actionAddWfsLayer_triggered()
{
}

void DataViewer::on_actionAddWcsLayer_triggered()
{
}
void DataViewer::on_actionZoomIn_triggered()
{
	m_mapCanvas->setMapTool(m_zoomInTool);
}
void DataViewer::on_actionZoomOut_triggered()
{
	m_mapCanvas->setMapTool(m_zoomOutTool);
}
void DataViewer::on_actionPan_triggered()
{
	m_mapCanvas->setMapTool(m_panTool);
}
void DataViewer::on_actionZoomActual_triggered()
{
}
void DataViewer::on_actionFullExtent_triggered()
{
	m_mapCanvas->zoomToFullExtent();
}
void DataViewer::on_actionMeasureLine_triggered()
{
	m_mapCanvas->setMapTool(m_measureLineTool);
}
void DataViewer::on_actionMeasureArea_triggered()
{
	m_mapCanvas->setMapTool(m_measureAreaTool);
}
void DataViewer::on_actionMeasureAngle_triggered()
{
}

void DataViewer::on_actionAddPolygon_triggered()
{
	// shp文件转换
	QString sourceShpFile = QStringLiteral("D:\\ScholarData\\GeographicData\\中国数据\\中国地图ARCGIS\\省级行政区.shp");
	QString targetFile = QStringLiteral("D:\\ScholarData\\GeographicData\\中国数据\\中国地图ARCGIS\\省级行政区.shp");

	QgsVectorLayer *layer = new QgsVectorLayer(sourceShpFile);
	if (layer == nullptr || !layer->isValid())
	{
		return;
	}

	// 面编辑——添加要素
	layer->startEditing();
	m_mapCanvas->setLayers(QList<QgsMapLayer *>() << layer);
	m_mapCanvas->setCurrentLayer(layer);
	QgsMapToolAddFeature *addFeatureTool = new QgsMapToolAddFeature(m_mapCanvas, QgsMapToolCapture::CapturePolygon);
	m_mapCanvas->setMapTool(addFeatureTool);
}

void DataViewer::on_actionCoordTrans_triggered()
{
	// shp文件转换
	QString sourceShpFile = QStringLiteral("E:\\code_src\\Qtitan正式版\\QGISDevExcer\\QGISDevExcer\\data\\G4906D01D_WP.shp");
	QString targetFile = QStringLiteral("E:\\code_src\\Qtitan正式版\\QGISDevExcer\\QGISDevExcer\\data\\output.shp");

	QgsVectorLayer *layer = new QgsVectorLayer(sourceShpFile);
	if (layer == nullptr || !layer->isValid())
	{
		return;
	}
	// 确定原始图层的参考系和目标图层的参考系
	QString targetCrsCode = QStringLiteral("EPSG:3857");
	QgsCoordinateReferenceSystem shpCrs = layer->crs();									  // 原始图层参考系
	QgsCoordinateReferenceSystem targetCrs = QgsCoordinateReferenceSystem(targetCrsCode); // 目标图层参考系
	// 构造坐标转换对象
	QgsCoordinateTransform *pTransform = new QgsCoordinateTransform(shpCrs, targetCrs, QgsProject::instance());
	if (pTransform == nullptr || !pTransform->isValid())
	{
		return;
	}
	// 对图层中每一个要素进行转换，将转换后的结果放入postRansfeatureLIst中
	QgsFeatureList postTransfeatureList;
	QgsFeatureIterator iter = layer->getFeatures();
	QgsFeature feature;
	while ((iter.nextFeature(feature)))
	{
		QgsGeometry g = feature.geometry();
		QgsAttributes f = feature.attributes();
		// transform函数就是转换函数，将每一个geometry进行转换
		// QgsGeometry是基类，transform函数是个虚函数
		// 根据具体的类型调用各自Geometry中的transfrom
		// 具体的转换可以参考QgsGeometry的子类的transform
		if (g.transform(*pTransform) == 0)
		{
			feature.setGeometry(g);
		}
		else
		{
			feature.clearGeometry();
		}
		postTransfeatureList << feature;
	}

	// 创建一个新shp,将转换后的postTransfeatureList存入新的shp中
	QString errorMessage;
	const QString fileFormat = "ESRI Shapefile";
	const QString enc = "System";
	QgsFields fields = layer->fields();
	QgsVectorLayer *targetLayer = createTempLayer(layer->geometryType(), targetCrsCode, fields, postTransfeatureList);
	QString errMsg;
	QgsVectorFileWriter::SaveVectorOptions saveOptions;
	saveOptions.fileEncoding = targetLayer->dataProvider()->encoding();
	saveOptions.driverName = "ESRI Shapefile";
	QgsVectorFileWriter::WriterError err = QgsVectorFileWriter::writeAsVectorFormatV2(targetLayer, targetFile, targetLayer->transformContext(), saveOptions, nullptr, nullptr, &errMsg);
	if (err != QgsVectorFileWriter::WriterError::NoError)
	{
		return;
	}
	return;
}

// 显示图层的要素属性表
void DataViewer::on_actionOpenAttrTable_triggered()
{
	QString sourceShpFile = QStringLiteral("D:\\ScholarData\\GeographicData\\武汉数据\\武汉市各类POI数据\\餐饮美食\\餐饮美食.shp");

	QgsVectorLayer *layer = new QgsVectorLayer(sourceShpFile);
	if (layer == nullptr || !layer->isValid())
	{
		return;
	}

	QgsAttributeTableDialog *dlg = new QgsAttributeTableDialog(layer);
	dlg->show();

	// QgsVectorLayerCache* lc = new QgsVectorLayerCache(layer, layer->featureCount());

	// QgsAttributeTableView* tv = new QgsAttributeTableView();
	// QgsAttributeTableModel* tm = new QgsAttributeTableModel(lc);
	// tm->loadLayer(); //一定不要忘记，否则mode1里面没有图层的属性数据

	// QgsAttributeTableFilterModel* tfm = new QgsAttributeTableFilterModel(m_mapCanvas, tm, tm);
	// tfm->setFilterMode(QgsAttributeTableFilterModel::ShowAll);
	// tv->setModel(tfm);

	// tv->show();
}

void DataViewer::on_actionOpenFields_triggered()
{
	QString sourceShpFile = QStringLiteral("D:\\ScholarData\\GeographicData\\武汉数据\\武汉市各类POI数据\\餐饮美食\\餐饮美食.shp");

	QgsVectorLayer *layer = new QgsVectorLayer(sourceShpFile);
	if (layer == nullptr || !layer->isValid())
	{
		return;
	}

	// 显示属性结构字段
	QgsSourceFieldsProperties *pWidget = new QgsSourceFieldsProperties(layer, nullptr);
	pWidget->loadRows();
	pWidget->show();
}

void DataViewer::on_actionNewBookmark_triggered()
{
	bool ok;
	QString text = QInputDialog::getText(this, QStringLiteral("输入书签名称"), "", QLineEdit::Normal, "", &ok);
	if (ok && !text.isEmpty())
	{
		m_bookmarkDlg->add(text, m_mapCanvas->extent());
	}
}
void DataViewer::on_actionShowBookmarks_triggered()
{
	m_bookmarkDlg->show();
}
void DataViewer::on_actionLayerTreeControl_treggered()
{
	// 打开图层管理器
	if (ui.LayerTreeControl->isVisible())
	{
		ui.LayerTreeControl->setFocus();
	}
	else
	{
		ui.LayerTreeControl->show();
	}
}
void DataViewer::on_actionOverviewMap_triggered()
{
	// 打开鹰眼图
	if (ui.OverviewMap->isVisible())
	{
		ui.OverviewMap->setFocus();
	}
	else
	{
		ui.OverviewMap->show();
	}
}
void DataViewer::on_actionPolyline_triggered()
{
	m_mapCanvas->setMapTool(m_pDrawLineTool);
}
void DataViewer::on_actionStylelibMng_triggered()
{

	// 显示属性表
	static QgsStyle style;

	if (style.symbolCount() == 0)
	{
		sqlite3_initialize();
		style.load(QgsApplication::userStylePath());
	}
	QgsStyleManagerDialog *dlg = new QgsStyleManagerDialog(&style);
	dlg->show();
}

void DataViewer::on_actionSelfStyleMng_triggered()
{
	Smart3dMap::S3d_StyleManagerLib::getSingletonPtr()->excuteSysStyleManager();
}

void DataViewer::on_actionSelectGeometry_triggered()
{
	if (m_mapCanvas->mapTool() != nullptr && m_mapCanvas->mapTool() == m_pSelectTool)
	{
		m_mapCanvas->unsetMapTool(m_mapCanvas->mapTool());
	}

	 QgsVectorLayer *pSelectLayer = nullptr;
	 if (m_mapCanvas->layerCount() > 0)
	{
		pSelectLayer = (QgsVectorLayer*)m_mapCanvas->layer(0);
		m_pSelectTool->SetSelectLayer(pSelectLayer);
	 }
	 m_pSelectTool->SetEnable(true);

	m_layerTreeView->setCurrentLayer(m_mapCanvas->currentLayer());
	m_mapCanvas->setMapTool(m_pSelectTool);
}

void DataViewer::removeLayer()
{
	if (!m_layerTreeView)
	{
		return;
	}

	QModelIndexList indexes;
	while ((indexes = m_layerTreeView->selectionModel()->selectedRows()).size())
	{
		m_layerTreeView->model()->removeRow(indexes.first().row());
	}
}

void DataViewer::initLayerTreeView()
{
	QgsLayerTreeModel *model = new QgsLayerTreeModel(QgsProject::instance()->layerTreeRoot(), this);
	qDebug() << QgsProject::instance()->layerTreeRoot();
	model->setFlag(QgsLayerTreeModel::AllowNodeReorder);
	model->setFlag(QgsLayerTreeModel::AllowNodeRename);
	model->setFlag(QgsLayerTreeModel::AllowNodeChangeVisibility);
	model->setFlag(QgsLayerTreeModel::ShowLegendAsTree);
	model->setFlag(QgsLayerTreeModel::UseEmbeddedWidgets);
	model->setAutoCollapseLegendNodes(10);
	m_layerTreeView->setModel(model);
	m_layerTreeCanvasBridge = new QgsLayerTreeMapCanvasBridge(QgsProject::instance()->layerTreeRoot(), m_mapCanvas, this);
	connect(QgsProject::instance(), SIGNAL(writeProject(QDomDocument &)), m_layerTreeCanvasBridge, SLOT(writeProject(QDomDocument &)));
	connect(QgsProject::instance(), SIGNAL(readProject(QDomDocument)), m_layerTreeCanvasBridge, SLOT(readProject(QDomDocument)));

	// 添加组命令
	QAction *actionAddGroup = new QAction(QStringLiteral("添加组"), this);
	actionAddGroup->setIcon(QIcon(QStringLiteral(":/Resources/mActionAddGroup.svg")));
	actionAddGroup->setToolTip(QStringLiteral("添加组"));
	connect(actionAddGroup, &QAction::triggered, m_layerTreeView->defaultActions(), &QgsLayerTreeViewDefaultActions::addGroup);

	// 扩展和收缩图层树
	QAction *actionExpandAll = new QAction(QStringLiteral("展开所有组"), this);
	actionExpandAll->setIcon(QIcon(QStringLiteral(":/Resources/mActionExpandTree.svg")));
	actionExpandAll->setToolTip(QStringLiteral("展开所有组"));
	connect(actionExpandAll, &QAction::triggered, m_layerTreeView, &QgsLayerTreeView::expandAllNodes);
	QAction *actionCollapseAll = new QAction(QStringLiteral("折叠所有组"), this);
	actionCollapseAll->setIcon(QIcon(QStringLiteral(":/Resources/mActionCollapseTree.svg")));
	actionCollapseAll->setToolTip(QStringLiteral("折叠所有组"));
	connect(actionCollapseAll, &QAction::triggered, m_layerTreeView, &QgsLayerTreeView::collapseAllNodes);

	// 移除图层
	QAction *actionRemoveLayer = new QAction(QStringLiteral("移除图层/组"));
	actionRemoveLayer->setIcon(QIcon(QStringLiteral(":/Resources/mActionRemoveLayer.svg")));
	actionRemoveLayer->setToolTip(QStringLiteral("移除图层/组"));
	connect(actionRemoveLayer, &QAction::triggered, this, &DataViewer::removeLayer);

	//-----------------------------------------------------------------------------------------------------------------
	// 右键菜单
	m_layertreemenuProvider = new DataViewerLayerTreeViewMenuProvider(m_layerTreeView, m_mapCanvas);
	m_layerTreeView->setMenuProvider(m_layertreemenuProvider);

	//链接切换编辑模式
	connect(m_layertreemenuProvider, &DataViewerLayerTreeViewMenuProvider::switchLayerEditable, this, &DataViewer::on_actionSwitchEdictable_triggered);

	// 连接layerTreeModel的rowsInserted信号
	connect(m_layerTreeView->layerTreeModel(), &QAbstractItemModel::rowsInserted, 
		this, [this](const QModelIndex &parent, int first, int last) {
			// 获取父节点
			QgsLayerTreeNode* FatherNode = m_layerTreeView->layerTreeModel()->index2node(parent);
			//获取子节点
			QgsLayerTreeNode* ChildNode = FatherNode->children()[first];
			//将子节点转换为图层
			QgsLayerTreeLayer* treeLayer = qobject_cast<QgsLayerTreeLayer*>(ChildNode);

			if (treeLayer) {
				// 将新添加的图层设置为当前图层
				this->m_mapCanvas->setCurrentLayer(treeLayer->layer());
			}
		});


	//设置选中图层数中图层为mapCanvas当前图层
	connect(m_layerTreeView, &QgsLayerTreeView::currentLayerChanged,
		[this](QgsMapLayer* layer) {
			this->m_mapCanvas->setCurrentLayer(layer);
		});

	//---------------------------------------------------------------------------------------------------------------

	QToolBar *toolbar = new QToolBar();
	toolbar->setIconSize(QSize(16, 16));
	toolbar->addAction(actionAddGroup);
	toolbar->addAction(actionExpandAll);
	toolbar->addAction(actionCollapseAll);
	toolbar->addAction(actionRemoveLayer);

	QVBoxLayout *vBoxLayout = new QVBoxLayout();
	vBoxLayout->setMargin(0);
	vBoxLayout->setContentsMargins(0, 0, 0, 0);
	vBoxLayout->setSpacing(0);
	vBoxLayout->addWidget(toolbar);
	vBoxLayout->addWidget(m_layerTreeView);

	QWidget *w = new QWidget;
	w->setLayout(vBoxLayout);
	this->ui.LayerTreeControl->setWidget(w);
}


void DataViewer::initMapOverviewCanvas()
{
	m_overviewCanvas->setBackgroundColor(QColor(180, 180, 180));
	QVBoxLayout *vBoxLayout = new QVBoxLayout();
	vBoxLayout->setMargin(0);
	vBoxLayout->setContentsMargins(0, 0, 0, 0);
	vBoxLayout->setSpacing(0);
	vBoxLayout->addWidget(m_overviewCanvas);

	QWidget *w = new QWidget;
	w->setLayout(vBoxLayout);
	w->setMinimumHeight(100);

	this->ui.OverviewMap->setWidget(w);
	m_layerTreeCanvasBridge->setOverviewCanvas(m_overviewCanvas);
	m_overviewCanvas->setLayers(m_mapCanvas->layers());
}

void DataViewer::on_actionSwitchEdictable_triggered()
{
	QgsMapLayer *layer = m_mapCanvas->currentLayer();
	if (layer == nullptr)
		return;
	if (layer->type()==QgsMapLayerType::VectorLayer)
	{
		QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer*>(layer);
		if (vlayer)
		{
			// 检查图层是否处于编辑模式并且被更改过
			if (vlayer->isEditable() && vlayer->isModified()) 
			{
				// 弹出一个对话框，询问用户是否保存更改
				QMessageBox::StandardButton reply;
				reply = QMessageBox::question(nullptr, "Save changes", "Do you want to save changes to the layer?",
					QMessageBox::Yes | QMessageBox::No);
				if (reply == QMessageBox::Yes) {
					// 如果用户选择"Yes"，则提交更改并停止编辑
					// 提交更改
					bool success = vlayer->commitChanges();

					if (!success)
					{
						// 如果提交失败，获取并输出错误信息
						QStringList errors = vlayer->commitErrors();
						for (const auto& error : errors)
						{
							qDebug() << error;
						}
					}
				}
				else {
					// 如果用户选择"No"，则放弃更改并停止编辑
					vlayer->rollBack();
				}
			}
			else 
			{
				// 如果图层不处于编辑模式，则开始编辑
				vlayer->startEditing();
			}
		}
	}
}

void DataViewer::on_actionAddGeometry_triggered()
{
	QgsMapLayer *layer = m_mapCanvas->currentLayer();
	if (layer == nullptr)
		return;

	// 如果当前处于添加模式，则退出添加模式
	if (m_mapCanvas->mapTool() != nullptr && (m_mapCanvas->mapTool() == m_addPointTool || m_mapCanvas->mapTool() == m_addLineTool || m_mapCanvas->mapTool() == m_addPolygonTool))
	{
		m_mapCanvas->unsetMapTool(m_mapCanvas->mapTool());
		return;
	}

	// 如果图层有效且为矢量图层，则进入添加模式
	if (layer->isValid() && layer->type() == QgsMapLayerType::VectorLayer)
	{
		// 转换为矢量图层
		QgsVectorLayer *vecLayer = qobject_cast<QgsVectorLayer *>(layer);

		//如果不处于编辑模式则返回
		if (!vecLayer->isEditable())
			return;

		switch (vecLayer->geometryType()) // 根据图层类型选择不同的绘制工具
		{
		case QgsWkbTypes::GeometryType::PointGeometry:
			if (m_addPointTool == nullptr)
				m_addPointTool = new QgsMapToolAddFeature(m_mapCanvas, QgsMapToolCapture::CapturePoint);
			m_mapCanvas->setMapTool(m_addPointTool);

			break;
		case QgsWkbTypes::GeometryType::LineGeometry:
			if (m_addLineTool == nullptr)
				m_addLineTool = new QgsMapToolAddFeature(m_mapCanvas, QgsMapToolCapture::CaptureLine);
			m_mapCanvas->setMapTool(m_addLineTool);

			break;
		case QgsWkbTypes::GeometryType::PolygonGeometry:
			if (m_addPolygonTool == nullptr)
				m_addPolygonTool = new QgsMapToolAddFeature(m_mapCanvas, QgsMapToolCapture::CapturePolygon);
			m_mapCanvas->setMapTool(m_addPolygonTool);
			break;
		default:
			break;
		}
	}
}

void DataViewer::on_actionDeleteFeatures_triggered()
{
	QgsMapLayer* layer = m_mapCanvas->currentLayer();
	if (layer == nullptr)
		return;

	//如果选中的为矢量图层
	if (layer->isValid() && layer->type() == QgsMapLayerType::VectorLayer)
	{
		// 转换为矢量图层
		QgsVectorLayer* vecLayer = qobject_cast<QgsVectorLayer*>(layer);

		if (!vecLayer->isEditable())
			return;

		/*开始*/
		QgsFeatureIds selectedFeatureIds = vecLayer->selectedFeatureIds();		//获取选中要素ID

		if(selectedFeatureIds.isEmpty())
			QMessageBox::warning(nullptr,QStringLiteral("Feature Error"), QStringLiteral("No feature selected."), QMessageBox::Ok);

		vecLayer->deleteFeatures(selectedFeatureIds);	//删除选中要素
		m_mapCanvas->refresh();		//刷新显示
	}
}

void DataViewer::on_actionMoveFeatures_triggered()
{
	if (m_mapCanvas->mapTool() != m_pMoveTool)
	{
		m_mapCanvas->setMapTool(m_pMoveTool);
	}
	else
	{
		m_mapCanvas->unsetMapTool(m_pMoveTool);
	}
}

void DataViewer::on_actionCopyAndMoveFeatures_triggered()
{
	if (m_mapCanvas->mapTool() != m_pCopyThenMove)
	{
		m_mapCanvas->setMapTool(m_pCopyThenMove);
	}
	else
	{
		m_mapCanvas->unsetMapTool(m_pCopyThenMove);
	}
}