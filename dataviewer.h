#pragma once

#include "BookmarkDialog.h"
#include"LayerTreeViewMenuProvider.h"
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


// ���ݲ鿴��������
class DataViewer : public QMainWindow
{
	Q_OBJECT
public:
	DataViewer(QWidget *parent = 0);
	~DataViewer();


public:
	// ��ʼ��ͼ�������
	void initLayerTreeView();

	// ��ʼ��ӥ��ͼ�ؼ�
	void initMapOverviewCanvas();

public slots:
	// �򿪹���
	void on_actionOpenProject_triggered();
	// ���湤��
	void on_actionSaveProject_triggered();
	// ���湤��
	void on_actionSaveAsProject_triggered();

	// ����ʸ������
	void on_actionAddVectorData_triggered();
	// ����դ������
	void on_actionAddRasterData_triggered();
	// ����WMSͼ��
	void on_actionAddWmsLayer_triggered();
	// ����WFSͼ��
	void on_actionAddWfsLayer_triggered();
	// ����WCSͼ��
	void on_actionAddWcsLayer_triggered();

	// ʹ�õ�ͼ�Ŵ󹤾�
	void on_actionZoomIn_triggered();
	// ʹ�õ�ͼ��С����
	void on_actionZoomOut_triggered();
	// ʹ�õ�ͼ���ι���
	void on_actionPan_triggered();
	// ���ŵ�ͼ��ʵ�ʱ���
	void on_actionZoomActual_triggered();
	// ���ŵ�ͼ��ȫͼ��Χ
	void on_actionFullExtent_triggered();
	// ��ͼ���⹤��
	void on_actionMeasureLine_triggered();
	void on_actionMeasureArea_triggered();
	void on_actionMeasureAngle_triggered();

	//����ת��
	void on_actionCoordTrans_triggered();
	//�鿴���Ա�
	void on_actionOpenAttrTable_triggered();
	//�鿴�ֶ�
	void on_actionOpenFields_triggered();
	//������Ҫ��
	void on_actionAddPolygon_triggered();
	//���ſ����
	void on_actionStylelibMng_triggered();
	void on_actionSelfStyleMng_triggered();

	//�༭ģʽ
	void on_actionActivateMode_triggered();

	// �����µĵ�ͼ��ǩ
	void on_actionNewBookmark_triggered();
	// ��ʾ��ͼ��ǩ������
	void on_actionShowBookmarks_triggered();

	// ͼ�������
	void on_actionLayerTreeControl_treggered();
	// ӥ��ͼ
	void on_actionOverviewMap_triggered();

	void on_actionPolyline_triggered();
	void on_actionSelectGeometry_triggered();

	// �Ƴ�ͼ��
	void removeLayer();

private:
	Ui::DataViewerClass ui;
	// ��ͼ����
	QgsMapCanvas* m_mapCanvas = nullptr;
	// ͼ�������
	QgsLayerTreeView* m_layerTreeView;
	QgsLayerTreeMapCanvasBridge* m_layerTreeCanvasBridge;
	// ͼ��������˵��ṩ��
	DataViewerLayerTreeViewMenuProvider* m_layertreemenuProvider;

	// ӥ��ͼ�ؼ�
	QgsMapOverviewCanvas* m_overviewCanvas;
	// ��ͼ���Ź���
	QgsMapTool* m_zoomInTool;
	QgsMapTool* m_zoomOutTool;
	// ��ͼ�������
	QgsMapTool* m_panTool;
	// ��ͼ���⹤��
	QgsMapTool* m_measureLineTool;
	QgsMapTool* m_measureAreaTool;

	//���ӵ㹤��
	QgsMapToolAddFeature* m_addPointTool;
	//�����߹���
	QgsMapToolAddFeature* m_addLineTool;
	//�����湤��
	QgsMapToolAddFeature* m_addPolygonTool;

	// ��ǩ����
	BookMarkDialog* m_bookmarkDlg;

	QgsMapToolDrawLine* m_pDrawLineTool;

	MapToolSelect *m_pSelectTool;
	
};
