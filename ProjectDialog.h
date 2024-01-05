#pragma once
#include "ui_Project.h"
#include <qgsmapoverviewcanvas.h>
#include <qgsvectorlayer.h>
#include <qgsrasterlayer.h>
#include <QgsLayerStylingwidget.h>
#include <qgsrectangle.h>
#include<qmenu.h>
#include <QObject>
#include <qgsrenderer.h>
#include <qgssinglesymbolrendererwidget.h>
#include<QgsSourceFieldsProperties.h>
#include<QgsVectorLayerCache.h>
#include <qgsprojectionselectiondialog.h>//QGIS��ͶӰ�Ի���
#include <qgsmapcanvas.h>
#include <qgslayertreeview.h>
#include <qgslayertreemodel.h>
#include <qgslayertreemapcanvasbridge.h>
#include <qgslayertreeviewdefaultactions.h>
#include <qgslayertree.h>
#include <qgsmapoverviewcanvas.h>
#include<QgsVectorFileWriter.h>
class ProjectDialog : public QDialog {
    Q_OBJECT
        friend class QtGisLayerTreeViewMenuProvider; // �����Ԫ��
public:
    explicit ProjectDialog(QgsVectorLayer* layer = nullptr, QWidget* parent = nullptr);
    ~ProjectDialog();
    void getsourceCrs(QgsVectorLayer* mySource);//��õ�ǰͼ���ͶӰ����
    QgsVectorLayer* createTempLayer_1(QgsWkbTypes::GeometryType geomType, QString crs, QgsFields fields, QgsFeatureList features);
    QgsVectorLayer* myLayer;
public slots:
    void buttonPath_clicked();//����·�������ʱ�����Ӧ�¼�
    void okButton_clicked();//����ȷ����ť�����ʱ�����Ӧ�¼�
    
private:
    
    Ui::ProjectDialog* ui;


};
