/*************
**
** Project:      Imaginable
** File info:    $Id$
** Author:       Copyright (C) 2009,2010 Kuzma Shapran <Kuzma.Shapran@gmail.com>
** License:      GPLv3
**
**  This file is part of Imaginable.
**
**  Imaginable is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  Imaginable is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with Imaginable.  If not, see <http://www.gnu.org/licenses/>.
**
*************/

#ifndef IMAGINABLE__IMAGE__INCLUDED
#define IMAGINABLE__IMAGE__INCLUDED


#include <types.hpp>

#include <boost/shared_array.hpp>

#include <QtCore/QSet>
#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QPoint>
#include <QtCore/QSize>
//#include <QtCore/QThread>



//class ThreadSleep: public QThread
//{
//public:
//	static void  sleep(unsigned long v) { QThread:: sleep(v); }
//	static void msleep(unsigned long v) { QThread::msleep(v); }
//	static void usleep(unsigned long v) { QThread::usleep(v); }
//};


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

	static Pixel scaleUp  (const uchar& value) { return static_cast<Pixel>(value)*0x0101 ; }
	static uchar scaleDown(const Pixel& value) { return static_cast<uchar>(value>>8); }

protected:
	typedef boost::shared_array<Image::Pixel> Plane;
	typedef QHash<Image::ColourPlane,Plane> ColourPlanes;
	typedef QHash<Image::ColourPlane,QString> ColourPlaneNames;
	typedef QHash<QString,QString> Text;

public:
	Q_PROPERTY(QSize  size    READ size    WRITE setSize   )
	Q_PROPERTY(int    width   READ width   WRITE setWidth  )
	Q_PROPERTY(int    height  READ height  WRITE setHeight )

	Q_PROPERTY(QPoint offset  READ offset  WRITE setOffset )
	Q_PROPERTY(int    x       READ x       WRITE setX      )
	Q_PROPERTY(int    y       READ y       WRITE setY      )

	Q_PROPERTY(bool   busy    READ busy    WRITE setBusy   )
	Q_PROPERTY(double percent READ percent WRITE setPercent)

public slots:
	virtual QSize   size(void) const     { return m_size; }
	virtual void setSize(QSize value)    { setWidth(value.width()); setHeight(value.height()); }

	virtual int     width (void) const   { return m_size.width();   }
	virtual void setWidth (int);

	virtual int     height(void) const   { return m_size.height();  }
	virtual void setHeight(int);

	virtual int     area(void) const     { return width()*height(); }


	virtual QPoint  offset(void) const   { return m_offset; }
	virtual void setOffset(QPoint value) { m_offset=value; }

	virtual int     x     (void) const   { return m_offset.x();  }
	virtual void setX     (int value)    { m_offset.setX(value); }

	virtual int     y     (void) const   { return m_offset.y();  }
	virtual void setY     (int value)    { m_offset.setY(value); }


	virtual void clear(void);


	/// returns true when image has non-zero dimension even if it does not have any colour plane
	virtual bool isEmpty(void) const { return ( (!m_size.width()) && (!m_size.height()) ); }
	/// returns true when image has at least one pixel of data
	virtual bool hasData(void) const { return ( (!isEmpty()) && (!m_plane.isEmpty()) ); }


	virtual ColourSpace colourSpace(void) const;

	virtual ColourPlaneSet planes    (void)      const { return m_plane.keys().toSet(); }
	virtual QintList       planesList(void)      const { return m_plane.keys(); }
	virtual bool        hasPlane     (int plane) const { return m_plane.find(plane) != m_plane.end(); }
	virtual bool        hasAlpha     (void)      const { return hasPlane(Image::PLANE_ALPHA); }
	virtual int         planesCount  (void)      const { return m_plane.size(); }

	virtual bool    addPlane(int);
	virtual bool removePlane(int);
	virtual bool   movePlane(int from,int to);

	virtual bool      planeHasName(int) const;
	virtual QString   planeName   (int) const;
	virtual bool   setPlaneName   (int,QString);
	virtual bool erasePlaneName   (int);


	virtual const Pixel* plane(int) const;
	virtual       Pixel* plane(int);


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
	QPoint m_offset;
	QSize m_size;
	ColourPlanes m_plane;
	ColourPlaneNames m_planeName;
	Text m_text;

	volatile bool m_busy;
	volatile double m_percent;

	virtual void onSetBusy(void) {}
	virtual void onSetPercent(void) {}

};

#endif // IMAGINABLE__IMAGE__INCLUDED
