class QgsSingleBandColorDataRenderer: QgsRasterRenderer
{
%TypeHeaderCode
    #include "qgssinglebandcolordatarenderer.h"
%End
  public:
    QgsSingleBandColorDataRenderer( QgsRasterDataProvider* provider, int band );
    ~QgsSingleBandColorDataRenderer();
    QgsRasterInterface * clone();

    static QgsRasterRenderer* create( const QDomElement& elem, QgsRasterDataProvider* provider );

    void * readBlock( int bandNo, const QgsRectangle & extent, int width, int height );

    void writeXML( QDomDocument& doc, QDomElement& parentElem ) const;
};
