/**************************ѡ��ͼ��Ҫ�ع���*********************

*****************************************************************/
#pragma once
// Qgisͷ�ļ�
#include <qgsmaptool.h>
#include <qgsmaptoolselect.h>
#include <qgsmapcanvas.h>
#include <qgsvectorlayer.h>
#include <qgsrubberband.h>

// Qtͷ�ļ�
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

	// ��������ͷ��¼�����
	void canvasReleaseEvent(QgsMapMouseEvent *e) override;

	void canvasMoveEvent(QgsMapMouseEvent *e) override;

	void canvasPressEvent(QgsMapMouseEvent *e) override;

	//��д�����ȡ�������
	void activate() override;


	void deactivate() override;


private:
	QgsVectorLayer *mvecLayer;		// ��ǰ��ѡ��(�)��ͼ��
	QgsRubberBand *mRubberBand; // ��ѡ��Χ
	QgsPointXY mpressPoint;		// �����λ��
	bool isClicked = false;		// �Ƿ��Ѿ�������������ж��Ƿ��ǵ���

	QgsFeatureIds layerSelectedFeatures; // ͼ�㱻ѡ���Ҫ��
	bool StatusFlag;					 // ����״̬

private:
	// ��ȡ��굥�ε��λ��һ����Χ��Ϊѡ������
	void ExpandSingleClicked();

	// ѡ��ͼ������
	void  SetSelectFeatures(QgsGeometry& selectGeometry,Qt::KeyboardModifiers modifiers);
};
