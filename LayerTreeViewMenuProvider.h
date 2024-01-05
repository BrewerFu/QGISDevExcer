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
	DataViewerLayerTreeViewMenuProvider(QgsLayerTreeView* view, QgsMapCanvas* canvas);	// ���캯��
	~DataViewerLayerTreeViewMenuProvider() {};	// ��������

	virtual QMenu* createContextMenu() override;	// �����Ҽ��˵�����д������麯��
	void initProvider();	// ��ʼ���Ҽ��˵�

signals:
	void activeMode();	//����༭�ź�

public slots:
	void openSymbolizeWdt();	// �򿪷��Ż�����
	void openAttributeTable();	// �����Ա�
	void openAttributeField();	// ���ֶ����Ա�
	void setActivateMode();	// ����༭ģʽ
	void saveAsFileVector();	// ���Ϊʸ���ļ�
	void saveAsFileRaster();	// ���Ϊդ���ļ�
	void openProjectVectorDlg();	// ��ʸ��ͶӰת������
	void openProjectRasterDlg();	// ��դ��ͶӰת������

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

