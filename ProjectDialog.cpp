#include "ProjectDialog.h"
#include "qgsvectorlayereditbuffer.h"

ProjectDialog::ProjectDialog(QgsVectorLayer* layer, QWidget* parent) :
    QDialog(parent),
    ui(new Ui::ProjectDialog) {
    ui->setupUi(this);
    if (layer) {
        myLayer = layer;
        getsourceCrs(layer);
    }
	connect(ui->buttonPath, SIGNAL(clicked()), this, SLOT(buttonPath_clicked()));
	connect(ui->okButton, SIGNAL(clicked()), this, SLOT(okButton_clicked()));
}

ProjectDialog::~ProjectDialog() {
    delete ui;
}

void ProjectDialog::getsourceCrs(QgsVectorLayer* mySource) {
   

    QgsCoordinateReferenceSystem shpCrs = mySource->sourceCrs();
    QString sourceCrsCode = shpCrs.authid();
    QString my = "222";

    if (sourceCrsCode.isEmpty()) {
        sourceCrsCode = "222";
       
    }
    ui->LineEdit_curProj->setText(sourceCrsCode);

}

void ProjectDialog::buttonPath_clicked() {
	QMessageBox::information(NULL, "Title", "1111", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
	QString outputFilename = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Shapefiles (*.shp)"));
	if (!outputFilename.isEmpty()) {
		if (!outputFilename.endsWith(".shp", Qt::CaseInsensitive)) {
			outputFilename += ".shp";
		}
		ui->lineEdit_tarPath->setText(outputFilename);
	}
}

void ProjectDialog::okButton_clicked() {
    // 获取目标路径
    QString destination = ui->lineEdit_tarPath->text();
    QString targetFile = destination;

    // 确定原始图层的参考系和目标图层的参考系
    QString targetCrsCode = ui->comboBox_tarProj->currentText();
    QgsCoordinateReferenceSystem shpCrs = myLayer->crs();  // 原始图层参考系
    QgsCoordinateReferenceSystem targetCrs = QgsCoordinateReferenceSystem(targetCrsCode);  // 目标图层参考系

    // 检查参考系是否有效
    if (!shpCrs.isValid() || !targetCrs.isValid()) {
        QMessageBox::information(NULL, "Invalid CRS", "The provided source CRS is invalid.", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        return;
    }

    // 构造坐标转换对象
    QgsProject* project = QgsProject::instance();
    QgsCoordinateTransformContext transformContext = QgsProject::instance()->transformContext();
    QgsCoordinateTransform pTransform(shpCrs, targetCrs, QgsProject::instance());

    // 创建存储转换后要素的列表        
    QgsFeatureList postTransfeatureList;
    QgsFeatureIterator iter = myLayer->getFeatures();
    QgsFeature feature;
    while ((iter.nextFeature(feature))) {
        QgsGeometry g = feature.geometry();
        QgsAttributes f = feature.attributes();

        // 转换几何对象
        if (g.transform(pTransform) == 0) {
            feature.setGeometry(g);
        }
        else {
            feature.clearGeometry();
        }
        postTransfeatureList << feature;  // 将转换后的要素添加到列表中
    }

    // 创建一个新的Shapefile图层，将转换后的要素存入其中
    QString errorMessage;
    const QString fileFormat = "ESRI Shapefile";
    const QString enc = "System";
    QgsFields fields = myLayer->fields();
    QgsVectorLayer* targetLayer = createTempLayer_1(myLayer->geometryType(), targetCrsCode, fields, postTransfeatureList);

    // 保存转换后的图层为Shapefile文件
    QString errMsg;
    QgsVectorFileWriter::SaveVectorOptions saveOptions;
    saveOptions.fileEncoding = targetLayer->dataProvider()->encoding();
    saveOptions.driverName = "ESRI Shapefile";
    QgsVectorFileWriter::WriterError err = QgsVectorFileWriter::writeAsVectorFormatV2(targetLayer, targetFile, targetLayer->transformContext(), saveOptions, nullptr, nullptr, &errMsg);

    // 检查保存过程中是否出现错误
    if (err != QgsVectorFileWriter::WriterError::NoError) {
        return;
    }
    return;
}
QgsVectorLayer* ProjectDialog::createTempLayer_1(QgsWkbTypes::GeometryType geomType, QString crs, QgsFields fields, QgsFeatureList features)
{
    QgsVectorLayer* newLayerPtr = nullptr;

    if (geomType == QgsWkbTypes::GeometryType::PointGeometry)
    {
        QString layerDef = "Point?crs=" + crs;
        newLayerPtr = new QgsVectorLayer(layerDef, QStringLiteral("PointLayer"), QStringLiteral("memory"));
    }
    else if (geomType == QgsWkbTypes::GeometryType::LineGeometry)
    {
        QString layerDef = "LineString?crs=" + crs;
        newLayerPtr = new QgsVectorLayer(layerDef, QStringLiteral("LineLayer"), QStringLiteral("memory"));
    }
    else if (geomType == QgsWkbTypes::GeometryType::PolygonGeometry)
    {
        QString layerDef = "MultiPolygon?crs=" + crs;
        newLayerPtr = new QgsVectorLayer(layerDef, QStringLiteral("PolyLayer"), QStringLiteral("memory"));
    }

    if (newLayerPtr == nullptr && !newLayerPtr->isValid())
    {
        return newLayerPtr;
    }

    newLayerPtr->commitChanges();
    if (!newLayerPtr->isEditable())
    {
        newLayerPtr->startEditing();
    }
    QgsFields sourceFlds = fields;
    for (int i = 0; i < sourceFlds.size(); i++)
    {
        newLayerPtr->editBuffer()->addAttribute(sourceFlds[i]);
    }
    newLayerPtr->commitChanges();

    if (!newLayerPtr->isEditable())
    {
        newLayerPtr->startEditing();
    }
    newLayerPtr->editBuffer()->addFeatures(features);
    bool flag = newLayerPtr->commitChanges();//保存
    newLayerPtr->editingStopped();
    return newLayerPtr;
}



