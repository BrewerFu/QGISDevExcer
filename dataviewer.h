#pragma once

#include "BookmarkDialog.h"
#include"LayerTreeViewMenuProvider.h"
#include"MapToolCopyThenMove.h"
#include"MapToolMove.h"
#include"MapToolRotate.h"
#include"MapToolSplit.h"
#include"MapToolZoom.h"
#include"maptoolselect.h"
#include "qgsmaptooldrawline.h"
#include "ui_dataviewer.h"
#include <qgslayertreemapcanvasbridge.h>
#include <qgslayertreemodel.h>
#include <qgsmapcanvas.h>
#include <qgsmapoverviewcanvas.h>
#include<qgsmaptooladdfeature.h>
#include<qgsmaptooledit.h>
#include <qgsmaptoolpan.h>
#include <qgsmaptoolselect.h>
#include <qgsmaptoolzoom.h>
#include <qgsrasterlayer.h>
#include <qgsrectangle.h>
#include <qgsvectorlayer.h>
#include <QtWidgets/QMainWindow>


// 数据查看器主窗口
class DataViewer : public QMainWindow
{
	Q_OBJECT
public:
	DataViewer(QWidget *parent = 0);
	~DataViewer();


public:
	// 初始化图层管理器
	void initLayerTreeView();

	// 初始化鹰眼图控件
	void initMapOverviewCanvas();

public slots:
	/*工程操作*/
	//-------------------------------------------------------------------------
	// 打开工程
	void on_actionOpenProject_triggered();
	// 保存工程
	void on_actionSaveProject_triggered();
	// 另存工程
	void on_actionSaveAsProject_triggered();

	/*数据操作*/
	//-------------------------------------------------------------------------
	// 添加矢量数据
	void on_actionAddVectorData_triggered();
	// 添加栅格数据
	void on_actionAddRasterData_triggered();
	// 添加WMS图层
	void on_actionAddWmsLayer_triggered();
	// 添加WFS图层
	void on_actionAddWfsLayer_triggered();
	// 添加WCS图层
	void on_actionAddWcsLayer_triggered();

	/*移动工具*/
	//-------------------------------------------------------------------------
	// 使用地图放大工具
	void on_actionZoomIn_triggered();
	// 使用地图缩小工具
	void on_actionZoomOut_triggered();
	// 使用地图漫游工具
	void on_actionPan_triggered();
	// 缩放地图到实际比例
	void on_actionZoomActual_triggered();
	// 缩放地图到全图范围
	void on_actionFullExtent_triggered();
	// 地图量测工具
	void on_actionMeasureLine_triggered();
	void on_actionMeasureArea_triggered();
	void on_actionMeasureAngle_triggered();

	//坐标转换
	void on_actionCoordTrans_triggered();
	//查看属性表
	void on_actionOpenAttrTable_triggered();
	//查看字段
	void on_actionOpenFields_triggered();
	//添加面要素
	void on_actionAddPolygon_triggered();
	//符号库管理
	void on_actionStylelibMng_triggered();
	void on_actionSelfStyleMng_triggered();

	//添加要素模式
	void on_actionAddGeometry_triggered();

	// 创建新的地图书签
	void on_actionNewBookmark_triggered();
	// 显示地图书签管理器
	void on_actionShowBookmarks_triggered();

	// 图层管理器
	void on_actionLayerTreeControl_treggered();
	// 鹰眼图
	void on_actionOverviewMap_triggered();

	void on_actionPolyline_triggered();

	/*几何图元编辑工具*/
	//-------------------------------------------------------------------------
	
	void on_actionSwitchEdictable_triggered();
	// 选择要素
	void on_actionSelectFetures_triggered();

	// 删除选择要素
	void on_actionDeleteFeatures_triggered();

	//移动要素
	void on_actionMoveFeatures_triggered();

	//复制并移动要素
	void on_actionCopyAndMoveFeatures_triggered();

	//旋转要素
	void on_actionRotateFeatures_triggered();

	//合并要素
	void on_actionMergeSelectedFeatures_triggered();

	//分割要素
	void on_actionSplitFeatures_triggered();

	//缩放要素
	void on_actionScaleFeatures_triggered();

	//-------------------------------------------------------------------------
	// 移除图层
	void removeLayer();

private:
	Ui::DataViewerClass ui;
	// 地图画布
	QgsMapCanvas* m_mapCanvas = nullptr;
	// 图层管理器
	QgsLayerTreeView* m_layerTreeView;
	QgsLayerTreeMapCanvasBridge* m_layerTreeCanvasBridge;
	// 图层管理器菜单提供者
	DataViewerLayerTreeViewMenuProvider* m_layertreemenuProvider;

	// 鹰眼图控件
	QgsMapOverviewCanvas* m_overviewCanvas;
	// 地图缩放工具
	QgsMapTool* m_zoomInTool;
	QgsMapTool* m_zoomOutTool;
	// 地图浏览工具
	QgsMapTool* m_panTool;
	// 地图量测工具
	QgsMapTool* m_measureLineTool;
	QgsMapTool* m_measureAreaTool;

	//添加点工具
	QgsMapToolAddFeature* m_addPointTool;
	//添加线工具
	QgsMapToolAddFeature* m_addLineTool;
	//添加面工具
	QgsMapToolAddFeature* m_addPolygonTool;

	// 书签窗体
	BookMarkDialog* m_bookmarkDlg;

	QgsMapToolDrawLine* m_pDrawLineTool;

	//----------------------------------------------------------------
	//选择工具
	MapToolSelect *m_pSelectTool;
	//移动要素工具
	MapToolMove* m_pMoveTool;
	//复制要素工具
	MapToolCopyThenMove* m_pCopyThenMoveTool;
	//旋转工具
	MapToolRotate* m_pRotateTool;
	//分割工具
	MapToolSplit* m_pSplitTool;
	//缩放工具
	MapToolZoom* m_pZoomTool;
};

