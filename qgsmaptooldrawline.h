/**************************绘制折线工具*********************
*****************************************************************/
#ifndef QGSMAPTOOLDRAWLINE_H
#define QGSMAPTOOLDRAWLINE_H
#pragma once
//Qgis头文件
#include <qgsmaptool.h>
#include <qgsmapcanvas.h>
#include <qgsvectorlayer.h>
#include <qgsrubberband.h>
#include <qgspoint.h>


//Qt头文件
#include <QgsMapMouseEvent.h>
#include <QPoint>
#include <QRect>
#include <QMessageBox>
#include <QApplication>
#include <QList>

class QgsMapToolDrawLine :	public QgsMapTool
{
public:
	QgsMapToolDrawLine(QgsMapCanvas *);
	~QgsMapToolDrawLine(void);

public:
    //重载鼠标移动事件
	virtual void canvasMoveEvent (QgsMapMouseEvent *e);
	//重载鼠标双击事件
	virtual void  canvasDoubleClickEvent (QgsMapMouseEvent *e);
	//重载鼠标单击击事件
	virtual void  canvasPressEvent (QgsMapMouseEvent *e);
	//设置绘制颜色和线宽
	void SetColorAndWidth(QColor,int);
	//得到点的坐标
	bool GetCoord(int index,double &x,double &y);
	//返回构成线段的数据点数
	int GetVertexCount();

private:
    QgsRubberBand *pRubBand;
	//鼠标左键双击的标志
	bool ButtonClickFlag;
	QgsMapCanvas *pMapCanvas;
	QColor mColor;
	int LineWidth;
	//存储点的坐标
	QList<QgsPoint>mPointSet;
};
#endif