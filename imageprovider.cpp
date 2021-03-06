#include "imageprovider.hpp"
#include <QBuffer>
#include <QScreen>
#include <QGuiApplication>
#include <math.h>

ImageProvider::ImageProvider():
  QQuickImageProvider( QQuickImageProvider::Image ),
  mDevicePixelRatio ( static_cast<double>( QGuiApplication::primaryScreen()->devicePixelRatio() ) )
{
  loadNextImage();
}

void ImageProvider::loadNextImage()
{
  mImagePath = ( mImagePath == image0 ) ? image1 : image0 ;
  QImage image{mImagePath};
  qDebug() << Q_FUNC_INFO << "Initial image:  " << mImagePath << "  " << image.width() << " x " << image.height();

  if ( image.width()*mDevicePixelRatio > mWindowWidth  ||
       image.height()*mDevicePixelRatio > mWindowHeight  ) {
    qDebug() <<  Q_FUNC_INFO << "Image size adjusted";
    auto widthDimension = std::min( static_cast<double>( mWindowWidth ), image.width() * mDevicePixelRatio ) ;
    auto heightDimension =   std::min( static_cast<double>( mWindowHeight ), image.height() * mDevicePixelRatio ) ;
    auto dimension = ( mWindowWidth > mWindowHeight ) ? widthDimension : heightDimension ;
    mImage =  image.scaled( dimension, dimension, Qt::KeepAspectRatio ) ;
  } else {
    qDebug() <<  Q_FUNC_INFO << "Image size NOT adjusted";
    mImage = QImage( image );
  }

  mImageWidth = mImage.width();
  mImageHeight =  mImage.height();
  mImagePieceWidth = mColumnRowCount == 0 ? 0 : static_cast<int>( ceil( mImageWidth / mColumnRowCount  /
                                                                        mDevicePixelRatio ) );
  mImagePieceHeight =  mColumnRowCount == 0 ? 0 :  static_cast<int>(  ceil( mImageHeight / mColumnRowCount /
                                                                            mDevicePixelRatio ) );
}

int ImageProvider::puzzlePieceCount( )
{

//  qDebug() << Q_FUNC_INFO << "Window:  " << windowWidth << " x " << windowHeight;
//  qDebug() << Q_FUNC_INFO << "Image:  " << mImageWidth << " x " << mImageHeight;
//  qDebug() << Q_FUNC_INFO << "Piece:  " << mImagePieceWidth << " x " << mImagePieceHeight;

  return ceil( mColumnRowCount * mColumnRowCount );
  //return 0;
}

bool ImageProvider::isPuzzleSolved( QVariantList xCoordinatesList, QVariantList yCoordinatesList ) const
{
  auto xCoordinatesListCount = xCoordinatesList.count();

  if ( xCoordinatesListCount == 0 ) {
    return false;
  }

  auto yCoordinatesListCount = yCoordinatesList.count();

  if ( yCoordinatesListCount == 0 ) {
    return false;
  }

  if ( xCoordinatesListCount != yCoordinatesListCount ) {
    return false;
  }

  static const int errThreshold = 8;
  int xPrior = -1;
  int yPrior = -1;
  int xCurrent = -1;
  int yCurrent = -1;
  int modulus = -1;

  for ( int index = 0; index < yCoordinatesListCount ; ++index ) {

    xCurrent = xCoordinatesList[index].toInt();
    yCurrent = yCoordinatesList[index].toInt();
    modulus =  index % mColumnRowCount ;

    if ( index > 0 ) {

      if ( modulus == 0 ) {
        if ( yCurrent - yPrior - errThreshold > mImagePieceHeight ) {
          return false;
        } else {
          if ( xCurrent - xPrior > errThreshold ) {
            return false;
          }
        }
      } else {
        if ( xCurrent - xPrior - errThreshold > mImagePieceWidth ) {
          return false;
        } else {
          if ( yCurrent - yPrior > errThreshold ) {
            return false;
          }
        }
      }
    }

//  qDebug() <<  " xCurrent=" << xCurrent << "  yCurrent=" << yCurrent  << " xPrior=" << xPrior  <<"  yPrior=" << yPrior  << "  index" << index << "  modulus" << modulus ;

    xPrior = xCurrent;
    yPrior = yCurrent;

  }

  return true;
}

QImage ImageProvider::requestImage( const QString& id, QSize* size, const QSize& requestedSize )
{
  Q_UNUSED( size );
  Q_UNUSED( requestedSize );

//  qDebug() << Q_FUNC_INFO << "+++++++++++++"   ;
  qDebug() << Q_FUNC_INFO << "id=" << id;
//  qDebug() << Q_FUNC_INFO << "id=" <<  id.toInt();


  if ( id.isEmpty() ) {
    mPieceXPosition = 0;
    mPieceYPosition = 0;
    return mImage.copy( mPieceXPosition, mPieceYPosition, mImageWidth,
                        mImageHeight  );
  } else {
    int x = mPieceXPosition ;
    int y = mPieceYPosition ;

    mPieceXPosition += ( mImagePieceWidth  );

    if ( mPieceXPosition * mDevicePixelRatio >= mImageWidth ) {
      mPieceXPosition = 0;
      mPieceYPosition += ( mImagePieceHeight  );
    }

    //qDebug() << Q_FUNC_INFO  << " x=" << x  << "  y=" << y << "  " << mImagePieceWidth << " x " << mImagePieceHeight;

    return mImage.copy( x * mDevicePixelRatio, y * mDevicePixelRatio, mImagePieceWidth * mDevicePixelRatio,
                        mImagePieceHeight * mDevicePixelRatio );

  }
}

void ImageProvider::setWindowDimenions( int windowWidth, int windowHeight )
{
  if ( windowWidth == 0 || windowHeight == 0 ) {
    return;
  }

  mWindowWidth = windowWidth;
  mWindowHeight = windowHeight;
}

