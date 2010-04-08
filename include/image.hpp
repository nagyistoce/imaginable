/* * * * * *
 *
 * Project:   Imaginable
 * Created:   2009-09-03
 * Author:    Kuzma Shapran <Kuzma.Shapran@gmail.com>
 * License:   GPLv3
 *
 * * * * * */
// $Id: image.hpp 9 2010-03-20 09:53:22Z Kuzma.Shapran $

#ifndef IMAGINABLE__IMAGE__INCLUDED
#define IMAGINABLE__IMAGE__INCLUDED


#include "types.hpp"

#include <boost/shared_array.hpp>

#include <QtCore/QSet>
#include <QtCore/QMap>
#include <QtCore/QList>
#include <QtCore/QStringList>
#include <QtCore/QString>
#include <QtCore/QSize>


class QObject;

class Image : public QObject
{
Q_OBJECT
public:
	Image(QObject* = NULL);
	virtual ~Image();

public:
	typedef enum
	{
		SPACE__CUSTOM=-1,
		SPACE__EMPTY=0,
		SPACE_LIGHTNESS,
		SPACE_MONO       = SPACE_LIGHTNESS,
		SPACE_MONOCHROME = SPACE_MONO,
		SPACE_GREY       = SPACE_LIGHTNESS,
		SPACE_GRAY       = SPACE_GREY,
		SPACE_RGB,
		SPACE_RGBA       = SPACE_RGB,
		SPACE_HSV,
		SPACE_HSVA       = SPACE_HSV,
		SPACE_HSL,
		SPACE_HSLA       = SPACE_HSL,
		SPACE_ALPHA,
		SPACE_MASK       = SPACE_ALPHA
	} ColourSpace;

	enum
	{
		PLANE_ALPHA=0,
		PLANE_RED,
		PLANE_GREEN,
		PLANE_BLUE,
		PLANE_HUE_SECTOR,
		PLANE_HUE,
		PLANE_SATURATION,
		PLANE_HSV_VALUE,
		PLANE_LIGHTNESS,
		PLANE__USER    = 0x100,
		PLANE__INTERNAL=-1
	};
	typedef int ColourPlane;

	typedef QSet<ColourPlane> ColourPlaneSet;
	typedef quint16 Pixel;
	typedef QSet<QString> TextKeys;

protected:
	typedef boost::shared_array<Image::Pixel> Plane;
	typedef QMap<Image::ColourPlane,Plane> ColourPlanes;
	typedef QMap<Image::ColourPlane,QString> ColourPlaneNames;
	typedef QMap<QString,QString> Text;

public:
	Q_PROPERTY(int   width   READ width   WRITE setWidth  )
	Q_PROPERTY(int   height  READ height  WRITE setHeight )
	Q_PROPERTY(QSize size    READ size    WRITE setSize   )
	Q_PROPERTY(Pixel maximum READ maximum WRITE setMaximum)

	Q_PROPERTY(bool   busy    READ busy    WRITE setBusy   )
	Q_PROPERTY(double percent READ percent WRITE setPercent)

public slots:
	virtual bool copyFrom(const Image&);


	virtual int     width (void) const { return m_size.width();  }
	virtual void setWidth (int);

	virtual int     height(void) const { return m_size.height(); }
	virtual void setHeight(int);

	virtual QSize   size(void) const   { return m_size; }
	virtual void setSize(QSize value)  { setWidth(value.width()); setHeight(value.height()); }
	virtual int     area(void) const   { return width()*height(); }

	virtual Pixel   maximum(void) const  { return m_maximum; }
	virtual void setMaximum(Pixel value) { m_maximum=value; }


	virtual void clear(void);


	/// returns true when image has non-zero dimension even if it does not have any colour plane
	virtual bool isEmpty(void) const { return ( (!m_size.width()) && (!m_size.height()) ); }
	/// returns true when image has at least one pixel of data
	virtual bool hasData(void) const { return ( (!isEmpty()) && (!m_plane.isEmpty()) ); }


	virtual ColourSpace colourSpace(void) const;

	virtual ColourPlaneSet planes    (void) const { return m_plane.keys().toSet(); }
	virtual QintList       planesList(void) const { return m_plane.keys(); }
	virtual bool hasPlane   (ColourPlane plane) const { return m_plane.find(plane) != m_plane.end(); }
	virtual bool hasAlpha   (void)              const { return hasPlane(Image::PLANE_ALPHA); }
	virtual int  planesCount(void)              const { return m_plane.size(); }

	virtual bool    addPlane(ColourPlane);
	virtual bool removePlane(ColourPlane);
	virtual bool   movePlane(ColourPlane from,ColourPlane to);

	virtual bool      planeHasName(ColourPlane) const;
	virtual QString   planeName   (ColourPlane) const;
	virtual bool   setPlaneName   (ColourPlane,QString);
	virtual bool erasePlaneName   (ColourPlane);


	virtual const Pixel* plane(ColourPlane) const;
	virtual       Pixel* plane(ColourPlane);


	virtual bool     hasText        (QString key) const { return m_text.find(key)!=m_text.end(); }
	virtual QString     text        (QString key) const;
	virtual TextKeys    textKeys    (void)        const { return m_text.keys().toSet(); }
	virtual QStringList textKeysList(void)        const { return m_text.keys(); }
	virtual bool     setText        (QString key,QString value);
	virtual bool   eraseText        (QString key);
	virtual void   clearTexts       (void)              { m_text.clear(); }



	virtual bool    busy(void) const { return m_busy; }
	virtual void setBusy(bool);

	virtual void  startLongProcessing(void) { setBusy(true); }
	virtual void finishLongProcessing(void) { setBusy(false); }

	virtual double  percent(void) const { return m_percent; }
	virtual void setPercent(double);

protected:
	QSize m_size;
	Image::Pixel m_maximum;
	ColourPlanes m_plane;
	ColourPlaneNames m_planeName;
	Text m_text;

	bool m_busy;
	double m_percent;

	virtual void onSetBusy(void) {}
	virtual void onSetPercent(void) {}
};

#endif // IMAGINABLE__IMAGE__INCLUDED
