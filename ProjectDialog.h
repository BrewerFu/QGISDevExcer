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
#include <qgsprojectionselectiondialog.h>//QGIS的投影对话框
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
        friend class QtGisLayerTreeViewMenuProvider; // 添加友元类
public:
    explicit ProjectDialog(QgsVectorLayer* layer = nullptr, QWidget* parent = nullptr);
    ~ProjectDialog();
    void getsourceCrs(QgsVectorLayer* mySource);//获得当前图层的投影类型
    QgsVectorLayer* createTempLayer_1(QgsWkbTypes::GeometryType geomType, QString crs, QgsFields fields, QgsFeatureList features);
    QgsVectorLayer* myLayer;
public slots:
    void buttonPath_clicked();//定义路径被点击时候的响应事件
    void okButton_clicked();//定义确定按钮被点击时候的响应事件
    
private:
    
    Ui::ProjectDialog* ui;


};
