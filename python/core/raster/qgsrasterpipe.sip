
/**  Raster pipe */

class QgsRasterPipe
{
%TypeHeaderCode
#include <qgsrasterpipe.h>
#include <qgsrasterresamplefilter.h>
#include <qgsrasterprojector.h>
%End

  public:
    // Role of known interfaces
    enum Role
    {
      UnknownRole   = 0,
      ProviderRole  = 1,
      RendererRole  = 2,
      ResamplerRole = 3,
      ProjectorRole = 4,
      NullerRole = 5,
    };

    QgsRasterPipe();
    QgsRasterPipe( const QgsRasterPipe& thePipe );

    ~QgsRasterPipe();

    /** Try to insert interface at specified index and connect
     * if connection would fail, the interface is not inserted and false is returned */
    bool insert( int idx, QgsRasterInterface* theInterface /Transfer/ );

    /** Try to replace interface at specified index and connect
     * if connection would fail, the interface is not inserted and false is returned */
    bool replace( int idx, QgsRasterInterface* theInterface /Transfer/ );

    /** Insert a new known interface in default place or replace interface of the same
     * role if it already exists. Known interfaces are: QgsRasterDataProvider,
     * QgsRasterRenderer, QgsRasterResampleFilter, QgsRasterProjector and their
     * subclasses. For unknown interfaces it mus be explicitly specified position
     * where it should be inserted using insert() method.
     */
    bool set( QgsRasterInterface * theInterface /Transfer/ );

    /** Remove and delete interface at given index if possible */
    bool remove( int idx );

    /** Remove and delete interface from pipe if possible */
    bool remove( QgsRasterInterface * theInterface );

    int size() const;
    QgsRasterInterface * at( int idx ) const;
    QgsRasterInterface * last() const;

    /** Set interface at index on/off
     *  Returns true on success */
    bool setOn( int idx, bool on );

    /** Test if interface at index may be swithed on/off */
    bool canSetOn( int idx, bool on );

    // Getters for special types of interfaces
    QgsRasterDataProvider * provider() const;
    QgsRasterRenderer * renderer() const;
    QgsRasterResampleFilter * resampleFilter() const;
    QgsRasterProjector * projector() const;
    QgsRasterNuller * nuller() const;

    /** Set on/off collection of statistics */
    void setStatsOn( bool on );
};
