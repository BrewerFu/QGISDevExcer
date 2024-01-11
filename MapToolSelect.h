/**************************选择图层要素工具*********************

*****************************************************************/
#pragma once
// Qgis头文件
#include <qgsmaptool.h>
#include <qgsmaptoolselect.h>
#include <qgsmapcanvas.h>
#include <qgsvectorlayer.h>
#include <qgsrubberband.h>

// Qt头文件
#include <QgsMapMouseEvent.h>
#include <QPoint>
#include <QRect>
#include <QMessageBox>
#include <QApplication>

#include <limits>

class MapToolSelect : public QgsMapToolSelect
{
	Q_OBJECT;

public:
	MapToolSelect(QgsMapCanvas *);
	~MapToolSelect(void);

public:

	// 重载鼠标释放事件函数
	void canvasReleaseEvent(QgsMapMouseEvent *e) override;

	void canvasMoveEvent(QgsMapMouseEvent *e) override;

	void canvasPressEvent(QgsMapMouseEvent *e) override;

	//重写激活和取消激活函数
	void activate() override;


	void deactivate() override;


private:
	QgsVectorLayer *mvecLayer;		// 当前被选择(活动)的图层
	QgsRubberBand *mRubberBand; // 框选范围
	QgsPointXY mpressPoint;		// 鼠标点击位置
	bool isClicked = false;		// 是否已经点击过，用于判断是否是单击

	QgsFeatureIds layerSelectedFeatures; // 图层被选择的要素
	bool StatusFlag;					 // 工具状态

private:
	// 提取鼠标单次点击位置一定范围作为选择区域
	void ExpandSingleClicked();

	// 选择图层特征
	void  SetSelectFeatures(QgsGeometry& selectGeometry,Qt::KeyboardModifiers modifiers);
};
