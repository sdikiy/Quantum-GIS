/***************************************************************************
    qgsrasterface.cpp - Internal raster processing modules interface
     --------------------------------------
    Date                 : Jun 21, 2012
    Copyright            : (C) 2012 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <limits>
#include <typeinfo>

#include <QByteArray>
#include <QTime>

#include "qgslogger.h"
#include "qgsrasterinterface.h"

#include "cpl_conv.h"

QgsRasterInterface::QgsRasterInterface( QgsRasterInterface * input )
    : mInput( input )
    , mOn( true )
    , mStatsOn( false )
{
}

QgsRasterInterface::~QgsRasterInterface()
{
}

bool QgsRasterInterface::typeIsNumeric( DataType dataType ) const
{
  switch ( dataType )
  {
    case Byte:
    case UInt16:
    case Int16:
    case UInt32:
    case Int32:
    case Float32:
    case CInt16:
    case Float64:
    case CInt32:
    case CFloat32:
    case CFloat64:
      return true;

    case UnknownDataType:
    case ARGB32:
    case ARGB32_Premultiplied:
    case TypeCount:
      return false;
  }
  return false;
}

bool QgsRasterInterface::typeIsColor( DataType dataType ) const
{
  switch ( dataType )
  {
    case ARGB32:
    case ARGB32_Premultiplied:
      return true;

    case UnknownDataType:
    case Byte:
    case UInt16:
    case Int16:
    case UInt32:
    case Int32:
    case Float32:
    case CInt16:
    case Float64:
    case CInt32:
    case CFloat32:
    case CFloat64:
    case TypeCount:
      return false;
  }
  return false;
}

QgsRasterInterface::DataType QgsRasterInterface::typeWithNoDataValue( DataType dataType, double *noDataValue )
{
  DataType newDataType;

  switch ( dataType )
  {
    case QgsRasterInterface::Byte:
      *noDataValue = -32768.0;
      newDataType = QgsRasterInterface::Int16;
      break;
    case QgsRasterInterface::Int16:
      *noDataValue = -2147483648.0;
      newDataType = QgsRasterInterface::Int32;
      break;
    case QgsRasterInterface::UInt16:
      *noDataValue = -2147483648.0;
      newDataType = QgsRasterInterface::Int32;
      break;
    case QgsRasterInterface::UInt32:
    case QgsRasterInterface::Int32:
    case QgsRasterInterface::Float32:
    case QgsRasterInterface::Float64:
      *noDataValue = std::numeric_limits<double>::max() * -1.0;
      newDataType = QgsRasterInterface::Float64;
    default:
      QgsDebugMsg( QString( "Unknow data type %1" ).arg( dataType ) );
      return UnknownDataType;
      break;
  }
  QgsDebugMsg( QString( "newDataType = %1 noDataValue = %2" ).arg( newDataType ).arg( *noDataValue ) );
  return newDataType;
}

inline bool QgsRasterInterface::isNoDataValue( int bandNo, double value ) const
{
  // More precise would be qIsNaN(value) && qIsNaN(noDataValue(bandNo)), but probably
  // not important and slower
  if ( qIsNaN( value ) ||
       doubleNear( value, noDataValue( bandNo ) ) )
  {
    return true;
  }
  return false;
}

// To give to an image preallocated memory is the only way to avoid memcpy
// when we want to keep data but delete QImage
QImage * QgsRasterInterface::createImage( int width, int height, QImage::Format format )
{
  // Qt has its own internal function depthForFormat(), unfortunately it is not public

  QImage img( 1, 1, format );

  // We ignore QImage::Format_Mono and QImage::Format_MonoLSB ( depth 1)
  int size = width * height * img.bytesPerLine();
  uchar * data = ( uchar * ) malloc( size );
  return new QImage( data, width, height, format );
}

void * QgsRasterInterface::block( int bandNo, QgsRectangle  const & extent, int width, int height )
{
  QTime time;
  time.start();

  void * b = 0;

  if ( !mOn )
  {
    // Switched off, pass input data unchanged
    if ( mInput )
    {
      b = mInput->block( bandNo, extent, width, height );
    }
  }
  else
  {
    b =  readBlock( bandNo, extent, width, height );
  }

  if ( mStatsOn )
  {
    if ( mTime.size() <= bandNo )
    {
      mTime.resize( bandNo + 1 );
    }
    // QTime counts only in miliseconds
    // We are adding time until next clear, this way the time may be collected
    // for the whole QgsRasterLayer::draw() for example, not just for a single part
    mTime[bandNo] += time.elapsed();
    QgsDebugMsg( QString( "bandNo = %2 time = %3" ).arg( bandNo ).arg( mTime[bandNo] ) );
  }
  return b;
}

void QgsRasterInterface::setStatsOn( bool on )
{
  if ( on )
  {
    mTime.clear();
  }
  if ( mInput ) mInput->setStatsOn( on );
  mStatsOn = on;
}

double QgsRasterInterface::time( bool cumulative )
{
  // We can calculate total time only, because we have to subtract time of previous
  // interface(s) and we don't know how to assign bands to each other
  double t = 0;
  for ( int i = 1; i < mTime.size(); i++ )
  {
    t += mTime[i];
  }
  if ( cumulative ) return t;

  if ( mInput )
  {
    QgsDebugMsgLevel( QString( "%1 cumulative time = %2  time = %3" ).arg( typeid( *( this ) ).name() ).arg( t ).arg( t -  mInput->time( true ) ), 3 );
    t -= mInput->time( true );
  }
  return t;
}

QString QgsRasterInterface::printValue( double value )
{
  /*
   *  IEEE 754 double has 15-17 significant digits. It specifies:
   *
   * "If a decimal string with at most 15 significant decimal is converted to
   *  IEEE 754 double precision and then converted back to the same number of
   *  significant decimal, then the final string should match the original;
   *  and if an IEEE 754 double precision is converted to a decimal string with at
   *  least 17 significant decimal and then converted back to double, then the final
   *  number must match the original."
   *
   * If printing only 15 digits, some precision could be lost. Printing 17 digits may
   * add some confusing digits.
   *
   * Default 'g' precision on linux is 6 digits, not all significant digits like
   * some sprintf manuals say.
   *
   * We need to ensure that the number printed and used in QLineEdit or XML will
   * give the same number when parsed.
   *
   * Is there a better solution?
   */

  QString s;

  for ( int i = 15; i <= 17; i++ )
  {
    s.setNum( value, 'g', i );
    if ( s.toDouble() == value )
    {
      return s;
    }
  }
  // Should not happen
  QgsDebugMsg( "Cannot correctly parse printed value" );
  return s;
}

void * QgsRasterInterface::convert( void *srcData, QgsRasterInterface::DataType srcDataType, QgsRasterInterface::DataType destDataType, int size )
{
  int destDataTypeSize = typeSize( destDataType ) / 8;
  void *destData = VSIMalloc( destDataTypeSize * size );
  for ( int i = 0; i < size; i++ )
  {
    double value = readValue( srcData, srcDataType, i );
    writeValue( destData, destDataType, i, value );
    //double newValue = readValue( destData, destDataType, i );
    //QgsDebugMsg( QString("convert type %1 to %2: %3 -> %4").arg(srcDataType).arg(destDataType).arg( value ).arg( newValue ) );
  }
  return destData;
}
