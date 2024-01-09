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
	bool flag = newLayerPtr->commitChanges(); // ����
	newLayerPtr->editingStopped();
	return newLayerPtr;
}

DataViewer::DataViewer(QWidget *parent)
	: QMainWindow(parent)
{
	m_mapCanvas = nullptr;
	// ͼ�������
	m_layerTreeView = nullptr;
	m_layerTreeCanvasBridge = nullptr;

	// ͼ��������Ҽ��˵�	/*����*/
	m_layertreemenuProvider = nullptr;

	// ӥ��ͼ�ؼ�
	m_overviewCanvas = nullptr;
	// ��ͼ���Ź���
	m_zoomInTool = nullptr;
	m_zoomOutTool = nullptr;
	// ��ͼ�������
	m_panTool = nullptr;
	// ��ͼ���⹤��
	m_measureLineTool = nullptr;
	m_measureAreaTool = nullptr;

	// ���ƹ���
	m_addPointTool = nullptr;
	m_addLineTool = nullptr;
	m_addPolygonTool = nullptr;

	// ��ǩ����
	m_bookmarkDlg = nullptr;

	ui.setupUi(this);

	m_mapCanvas = new QgsMapCanvas();
	this->setCentralWidget(m_mapCanvas);
	// ��ʼ��ͼ�������
	m_layerTreeView = new QgsLayerTreeView(this);
	initLayerTreeView();
	// ��ʼ��ӥ��ͼ�ؼ�
	m_overviewCanvas = new QgsMapOverviewCanvas(this, m_mapCanvas);
	initMapOverviewCanvas();
	// ��ʼ����ͼ����
	m_zoomInTool = new QgsMapToolZoom(m_mapCanvas, false);
	m_zoomOutTool = new QgsMapToolZoom(m_mapCanvas, true);
	m_panTool = new QgsMapToolPan(m_mapCanvas);
	m_measureLineTool = new MeasureTool(m_mapCanvas, false);
	m_measureAreaTool = new MeasureTool(m_mapCanvas, true);
	// ���ƹ���
	m_pDrawLineTool = new QgsMapToolDrawLine(m_mapCanvas);

	//--------------------------------------------------------------------------------------------
	m_pSelectTool = new MapToolSelect(m_mapCanvas);
	m_pMoveTool = new MapToolMove(m_mapCanvas);
	m_pCopyThenMove = new MapToolCopyThenMove(m_mapCanvas);

	// ��ʼ����ǩ����
	m_bookmarkDlg = new BookMarkDialog(m_mapCanvas);

	// ע�����еĽ��̹��ߣ�zhuyh_20221101��
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
	QString filename = QFileDialog::getOpenFileName(this, QStringLiteral("ѡ�񹤳��ļ�"), "", "QGIS project (*.qgs *.qgz)");
	QFileInfo fi(filename);
	if (!fi.exists())
	{
		return;
	}

	QgsProject::instance()->read(filename);
}

void DataViewer::on_actionSaveProject_triggered()
{
	QString filename = QFileDialog::getSaveFileName(this, QStringLiteral("���̱���Ϊ"), "", "QGIS project (*.qgs)");
	QFileInfo file(filename);
	QgsProject::instance()->write(filename);
}

void DataViewer::on_actionSaveAsProject_triggered()
{
	QString filename = QFileDialog::getSaveFileName(this, QStringLiteral("�������Ϊ"), "", "QGIS project (*.qgs)");
	QFileInfo file(filename);
	QgsProject::instance()->write(filename);
}

void DataViewer::on_actionAddVectorData_triggered()
{
	QStringList layerPathList = QFileDialog::getOpenFileNames(this, QStringLiteral("ѡ��ʸ������"), "", "shapefile (*.shp)");
	QList<QgsMapLayer *> layerList;
	for each (QString layerPath in layerPathList)
	{
		QFileInfo fi(layerPath);
		if (!fi.exists())
		{
			return;
		}
		QString layerBaseName = fi.baseName(); // ͼ������

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
	QStringList layerPathList = QFileDialog::getOpenFileNames(this, QStringLiteral("ѡ��դ������"), "", "Image (*.img *.tif *.tiff)");
	QList<QgsMapLayer *> layerList;
	for each (QString layerPath in layerPathList)
	{
		QFileInfo fi(layerPath);
		if (!fi.exists())
		{
			return;
		}
		QString layerBaseName = fi.baseName(); // ͼ������

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
	// shp�ļ�ת��
	QString sourceShpFile = QStringLiteral("D:\\ScholarData\\GeographicData\\�й�����\\�й���ͼARCGIS\\ʡ��������.shp");
	QString targetFile = QStringLiteral("D:\\ScholarData\\GeographicData\\�й�����\\�й���ͼARCGIS\\ʡ��������.shp");

	QgsVectorLayer *layer = new QgsVectorLayer(sourceShpFile);
	if (layer == nullptr || !layer->isValid())
	{
		return;
	}

	// ��༭�������Ҫ��
	layer->startEditing();
	m_mapCanvas->setLayers(QList<QgsMapLayer *>() << layer);
	m_mapCanvas->setCurrentLayer(layer);
	QgsMapToolAddFeature *addFeatureTool = new QgsMapToolAddFeature(m_mapCanvas, QgsMapToolCapture::CapturePolygon);
	m_mapCanvas->setMapTool(addFeatureTool);
}

void DataViewer::on_actionCoordTrans_triggered()
{
	// shp�ļ�ת��
	QString sourceShpFile = QStringLiteral("E:\\code_src\\Qtitan��ʽ��\\QGISDevExcer\\QGISDevExcer\\data\\G4906D01D_WP.shp");
	QString targetFile = QStringLiteral("E:\\code_src\\Qtitan��ʽ��\\QGISDevExcer\\QGISDevExcer\\data\\output.shp");

	QgsVectorLayer *layer = new QgsVectorLayer(sourceShpFile);
	if (layer == nullptr || !layer->isValid())
	{
		return;
	}
	// ȷ��ԭʼͼ��Ĳο�ϵ��Ŀ��ͼ��Ĳο�ϵ
	QString targetCrsCode = QStringLiteral("EPSG:3857");
	QgsCoordinateReferenceSystem shpCrs = layer->crs();									  // ԭʼͼ��ο�ϵ
	QgsCoordinateReferenceSystem targetCrs = QgsCoordinateReferenceSystem(targetCrsCode); // Ŀ��ͼ��ο�ϵ
	// ��������ת������
	QgsCoordinateTransform *pTransform = new QgsCoordinateTransform(shpCrs, targetCrs, QgsProject::instance());
	if (pTransform == nullptr || !pTransform->isValid())
	{
		return;
	}
	// ��ͼ����ÿһ��Ҫ�ؽ���ת������ת����Ľ������postRansfeatureLIst��
	QgsFeatureList postTransfeatureList;
	QgsFeatureIterator iter = layer->getFeatures();
	QgsFeature feature;
	while ((iter.nextFeature(feature)))
	{
		QgsGeometry g = feature.geometry();
		QgsAttributes f = feature.attributes();
		// transform��������ת����������ÿһ��geometry����ת��
		// QgsGeometry�ǻ��࣬transform�����Ǹ��麯��
		// ���ݾ�������͵��ø���Geometry�е�transfrom
		// �����ת�����Բο�QgsGeometry�������transform
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

	// ����һ����shp,��ת�����postTransfeatureList�����µ�shp��
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

// ��ʾͼ���Ҫ�����Ա�
void DataViewer::on_actionOpenAttrTable_triggered()
{
	QString sourceShpFile = QStringLiteral("D:\\ScholarData\\GeographicData\\�人����\\�人�и���POI����\\������ʳ\\������ʳ.shp");

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
	// tm->loadLayer(); //һ����Ҫ���ǣ�����mode1����û��ͼ�����������

	// QgsAttributeTableFilterModel* tfm = new QgsAttributeTableFilterModel(m_mapCanvas, tm, tm);
	// tfm->setFilterMode(QgsAttributeTableFilterModel::ShowAll);
	// tv->setModel(tfm);

	// tv->show();
}

void DataViewer::on_actionOpenFields_triggered()
{
	QString sourceShpFile = QStringLiteral("D:\\ScholarData\\GeographicData\\�人����\\�人�и���POI����\\������ʳ\\������ʳ.shp");

	QgsVectorLayer *layer = new QgsVectorLayer(sourceShpFile);
	if (layer == nullptr || !layer->isValid())
	{
		return;
	}

	// ��ʾ���Խṹ�ֶ�
	QgsSourceFieldsProperties *pWidget = new QgsSourceFieldsProperties(layer, nullptr);
	pWidget->loadRows();
	pWidget->show();
}

void DataViewer::on_actionNewBookmark_triggered()
{
	bool ok;
	QString text = QInputDialog::getText(this, QStringLiteral("������ǩ����"), "", QLineEdit::Normal, "", &ok);
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
	// ��ͼ�������
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
	// ��ӥ��ͼ
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

	// ��ʾ���Ա�
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

	// ���������
	QAction *actionAddGroup = new QAction(QStringLiteral("�����"), this);
	actionAddGroup->setIcon(QIcon(QStringLiteral(":/Resources/mActionAddGroup.svg")));
	actionAddGroup->setToolTip(QStringLiteral("�����"));
	connect(actionAddGroup, &QAction::triggered, m_layerTreeView->defaultActions(), &QgsLayerTreeViewDefaultActions::addGroup);

	// ��չ������ͼ����
	QAction *actionExpandAll = new QAction(QStringLiteral("չ��������"), this);
	actionExpandAll->setIcon(QIcon(QStringLiteral(":/Resources/mActionExpandTree.svg")));
	actionExpandAll->setToolTip(QStringLiteral("չ��������"));
	connect(actionExpandAll, &QAction::triggered, m_layerTreeView, &QgsLayerTreeView::expandAllNodes);
	QAction *actionCollapseAll = new QAction(QStringLiteral("�۵�������"), this);
	actionCollapseAll->setIcon(QIcon(QStringLiteral(":/Resources/mActionCollapseTree.svg")));
	actionCollapseAll->setToolTip(QStringLiteral("�۵�������"));
	connect(actionCollapseAll, &QAction::triggered, m_layerTreeView, &QgsLayerTreeView::collapseAllNodes);

	// �Ƴ�ͼ��
	QAction *actionRemoveLayer = new QAction(QStringLiteral("�Ƴ�ͼ��/��"));
	actionRemoveLayer->setIcon(QIcon(QStringLiteral(":/Resources/mActionRemoveLayer.svg")));
	actionRemoveLayer->setToolTip(QStringLiteral("�Ƴ�ͼ��/��"));
	connect(actionRemoveLayer, &QAction::triggered, this, &DataViewer::removeLayer);

	//-----------------------------------------------------------------------------------------------------------------
	// �Ҽ��˵�
	m_layertreemenuProvider = new DataViewerLayerTreeViewMenuProvider(m_layerTreeView, m_mapCanvas);
	m_layerTreeView->setMenuProvider(m_layertreemenuProvider);

	//�����л��༭ģʽ
	connect(m_layertreemenuProvider, &DataViewerLayerTreeViewMenuProvider::switchLayerEditable, this, &DataViewer::on_actionSwitchEdictable_triggered);

	// ����layerTreeModel��rowsInserted�ź�
	connect(m_layerTreeView->layerTreeModel(), &QAbstractItemModel::rowsInserted, 
		this, [this](const QModelIndex &parent, int first, int last) {
			// ��ȡ���ڵ�
			QgsLayerTreeNode* FatherNode = m_layerTreeView->layerTreeModel()->index2node(parent);
			//��ȡ�ӽڵ�
			QgsLayerTreeNode* ChildNode = FatherNode->children()[first];
			//���ӽڵ�ת��Ϊͼ��
			QgsLayerTreeLayer* treeLayer = qobject_cast<QgsLayerTreeLayer*>(ChildNode);

			if (treeLayer) {
				// ������ӵ�ͼ������Ϊ��ǰͼ��
				this->m_mapCanvas->setCurrentLayer(treeLayer->layer());
			}
		});


	//����ѡ��ͼ������ͼ��ΪmapCanvas��ǰͼ��
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
			// ���ͼ���Ƿ��ڱ༭ģʽ���ұ����Ĺ�
			if (vlayer->isEditable() && vlayer->isModified()) 
			{
				// ����һ���Ի���ѯ���û��Ƿ񱣴����
				QMessageBox::StandardButton reply;
				reply = QMessageBox::question(nullptr, "Save changes", "Do you want to save changes to the layer?",
					QMessageBox::Yes | QMessageBox::No);
				if (reply == QMessageBox::Yes) {
					// ����û�ѡ��"Yes"�����ύ���Ĳ�ֹͣ�༭
					// �ύ����
					bool success = vlayer->commitChanges();

					if (!success)
					{
						// ����ύʧ�ܣ���ȡ�����������Ϣ
						QStringList errors = vlayer->commitErrors();
						for (const auto& error : errors)
						{
							qDebug() << error;
						}
					}
				}
				else {
					// ����û�ѡ��"No"����������Ĳ�ֹͣ�༭
					vlayer->rollBack();
				}
			}
			else 
			{
				// ���ͼ�㲻���ڱ༭ģʽ����ʼ�༭
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

	// �����ǰ�������ģʽ�����˳����ģʽ
	if (m_mapCanvas->mapTool() != nullptr && (m_mapCanvas->mapTool() == m_addPointTool || m_mapCanvas->mapTool() == m_addLineTool || m_mapCanvas->mapTool() == m_addPolygonTool))
	{
		m_mapCanvas->unsetMapTool(m_mapCanvas->mapTool());
		return;
	}

	// ���ͼ����Ч��Ϊʸ��ͼ�㣬��������ģʽ
	if (layer->isValid() && layer->type() == QgsMapLayerType::VectorLayer)
	{
		// ת��Ϊʸ��ͼ��
		QgsVectorLayer *vecLayer = qobject_cast<QgsVectorLayer *>(layer);

		//��������ڱ༭ģʽ�򷵻�
		if (!vecLayer->isEditable())
			return;

		switch (vecLayer->geometryType()) // ����ͼ������ѡ��ͬ�Ļ��ƹ���
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

	//���ѡ�е�Ϊʸ��ͼ��
	if (layer->isValid() && layer->type() == QgsMapLayerType::VectorLayer)
	{
		// ת��Ϊʸ��ͼ��
		QgsVectorLayer* vecLayer = qobject_cast<QgsVectorLayer*>(layer);

		if (!vecLayer->isEditable())
			return;

		/*��ʼ*/
		QgsFeatureIds selectedFeatureIds = vecLayer->selectedFeatureIds();		//��ȡѡ��Ҫ��ID

		if(selectedFeatureIds.isEmpty())
			QMessageBox::warning(nullptr,QStringLiteral("Feature Error"), QStringLiteral("No feature selected."), QMessageBox::Ok);

		vecLayer->deleteFeatures(selectedFeatureIds);	//ɾ��ѡ��Ҫ��
		m_mapCanvas->refresh();		//ˢ����ʾ
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