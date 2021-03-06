class QgsRasterLayerSaveAsDialog : QDialog
{
%TypeHeaderCode
#include <qgsrasterlayersaveasdialog.h>
%End
  public:
    enum Mode
    {
      RawDataMode,
      RenderedImageMode
    };
    enum CrsState
    {
      OriginalCrs,
      CurrentCrs,
      UserCrs
    };
    enum ExtentState
    {
      OriginalExtent,
      CurrentExtent,
      UserExtent,
    };
    enum ResolutionState
    {
      OriginalResolution,
      UserResolution
    };

    QgsRasterLayerSaveAsDialog( QgsRasterLayer* rasterLayer,
                                QgsRasterDataProvider* sourceProvider, const QgsRectangle& currentExtent,
                                const QgsCoordinateReferenceSystem& layerCrs, const QgsCoordinateReferenceSystem& currentCrs,
                                QWidget* parent = 0, Qt::WindowFlags f = 0 );
    ~QgsRasterLayerSaveAsDialog();

    Mode mode() const;
    int nColumns() const;
    int nRows() const;
    double xResolution() const;
    double yResolution() const;
    int maximumTileSizeX() const;
    int maximumTileSizeY() const;
    bool tileMode() const;
    QString outputFileName() const;
    QString outputFormat() const;
    QgsCoordinateReferenceSystem outputCrs();
    QStringList createOptions() const;
    QgsRectangle outputRectangle() const;
    QList<QgsRasterNuller::NoData> noData() const;

    QList< int > overviewList() const;
    QgsRasterDataProvider::RasterBuildPyramids buildPyramidsFlag() const;
    QString pyramidsResampling() const;
    QgsRasterDataProvider::RasterPyramidsFormat pyramidsFormat() const;

    void hideFormat();
    void hideOutput();

  public slots:
    virtual void accept();
};
