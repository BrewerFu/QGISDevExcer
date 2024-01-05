#pragma once
#include<qgsmapcanvas.h>
#include <qgslayertree.h>
#include <qgslayertreeview.h>
#include<qgslayertreemodel.h>
#include <qgslayertreeviewdefaultactions.h>
#include<qmenu.h>
#include<qaction.h>
#include<qgssymbol.h>
#include<qmessagebox.h>
#include<qinputdialog.h>
#include<qfiledialog.h>
#include<qgsstyle.h>
#include<qgssymbolselectordialog.h>
#include<qgssinglesymbolrenderer.h>
#include<qgsvectorlayer.h>
#include<qgsrasterlayer.h>
#include<qgsattributedialog.h>
#include<qgssourcefieldsproperties.h>
#include<qgsattributetableview.h>
#include<qgsattributetablemodel.h>
#include<qgsattributetablefiltermodel.h>
#include<qgsattributetabledialog.h>
#include<qgsvectorlayersaveasdialog.h>
#include<qgsprojectionselectiondialog.h>
#include<qgsrasterprojector.h>
#include<qgsrasterfilewriter.h>

class DataViewerLayerTreeViewMenuProvider : public QObject, public QgsLayerTreeViewMenuProvider
{
	Q_OBJECT
public:
	DataViewerLayerTreeViewMenuProvider(QgsLayerTreeView* view, QgsMapCanvas* canvas);	// 构造函数
	~DataViewerLayerTreeViewMenuProvider() {};	// 析构函数

	virtual QMenu* createContextMenu() override;	// 创建右键菜单，重写父类的虚函数
	void initProvider();	// 初始化右键菜单

signals:
	void activeMode();	//激活编辑信号

public slots:
	void openSymbolizeWdt();	// 打开符号化窗口
	void openAttributeTable();	// 打开属性表
	void openAttributeField();	// 打开字段属性表
	void setActivateMode();	// 激活编辑模式
	void saveAsFileVector();	// 另存为矢量文件
	void saveAsFileRaster();	// 另存为栅格文件
	void openProjectVectorDlg();	// 打开矢量投影转换窗口
	void openProjectRasterDlg();	// 打开栅格投影转换窗口

protected:
	QAction* m_SymbolizeAction = nullptr;
	QAction* m_AttributeTableAction = nullptr;
	QAction* m_AttributeFieldAction = nullptr;
	QAction* m_SetCurrentLayer = nullptr;
	QAction* m_SaveAsFileVecor = nullptr;
	QAction* m_SaveAsFileRaster = nullptr;
	QAction* m_Projectvector = nullptr;
	QAction* m_ProjectRaster = nullptr;
	QgsLayerTreeView* m_view;
	QgsMapCanvas* m_canvas;
};

