#include <QtWidgets/QApplication>
#include <qgsapplication.h>

#include "dataviewer.h"

int main(int argc, char *argv[])
{
	QgsApplication a(argc, argv, true);
	QgsApplication::setPrefixPath("D:\\osgeo4w - qgis - dev\\QGIS-ltr", true); // !!!这个路径参数需要自行修改
	QgsApplication::init();
	QgsApplication::initQgis();
	DataViewer w;
	w.showMaximized();
	return a.exec();
}
