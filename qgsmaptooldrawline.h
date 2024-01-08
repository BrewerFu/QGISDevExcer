/**************************�������߹���*********************
*****************************************************************/
#ifndef QGSMAPTOOLDRAWLINE_H
#define QGSMAPTOOLDRAWLINE_H
#pragma once

// Qgisͷ�ļ�
#include <qgsmaptool.h>
#include <qgsmapcanvas.h>
#include <qgsvectorlayer.h>
#include <qgsrubberband.h>
#include <qgspoint.h>

// Qtͷ�ļ�
#include <QgsMapMouseEvent.h>
#include <QPoint>
#include <QRect>
#include <QMessageBox>
#include <QApplication>
#include <QList>

class QgsMapToolDrawLine : public QgsMapTool
{
public:
	QgsMapToolDrawLine(QgsMapCanvas *);
	~QgsMapToolDrawLine(void);

public:
	// ��������ƶ��¼�
	virtual void canvasMoveEvent(QgsMapMouseEvent *e);
	// �������˫���¼�
	virtual void canvasDoubleClickEvent(QgsMapMouseEvent *e);
	// ������굥�����¼�
	virtual void canvasPressEvent(QgsMapMouseEvent *e);
	// ���û�����ɫ���߿�
	void SetColorAndWidth(QColor, int);
	// �õ��������
	bool GetCoord(int index, double &x, double &y);
	// ���ع����߶ε����ݵ���
	int GetVertexCount();
	// �����ߵ�ѡ���ɾ��
	void deleteLine(int index)
	{
		if (index >= 0 && index < lines.size())
		{
			delete lines[index];
			lines.removeAt(index);
		}
	}
	void deleteLines(QList<int> indices)
	{
		std::sort(indices.begin(), indices.end(), std::greater<int>());
		for (int index : indices)
		{
			deleteLine(index);
		}
	}

private:
	QgsRubberBand *pRubBand;
	// ������˫���ı�־
	bool ButtonClickFlag;
	QgsMapCanvas *pMapCanvas;
	QColor mColor;
	int LineWidth;
	// �洢�������
	QList<QgsPoint> mPointSet;
	// �洢���е���
	QList<QgsRubberBand *> lines;
};
#endif