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


#include <boost/shared_array.hpp>

#include <QtCore/QSet>
#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QPoint>

#include "types.hpp"


class QObject;

class Image : public QObject
{
Q_OBJECT
public:
	Image(QObject * =NULL);
	~Image();

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
		PLANE__USER    = 100,
		PLANE__INTERNAL=-1
	};
	typedef int ColourPlane;

	typedef QSet<ColourPlane> ColourPlaneSet;
	typedef quint16 Pixel;
	typedef QSet<QString> TextKeys;

	static Pixel scaleUp  (const uchar &value) { return static_cast<Pixel>(value)*0x0101 ; }
	static uchar scaleDown(const Pixel &value) { return static_cast<uchar>(value>>8); }

protected:
	typedef boost::shared_array<Image::Pixel> Plane;
	typedef QHash<Image::ColourPlane,Plane> ColourPlanes;
	typedef QHash<Image::ColourPlane,QString> ColourPlaneNames;
	typedef QHash<QString,QString> Text;

public:
	Q_PROPERTY(QPoint size    READ size    WRITE setSize   )
	Q_PROPERTY(int    width   READ width   WRITE setWidth  )
	Q_PROPERTY(int    height  READ height  WRITE setHeight )

	Q_PROPERTY(QPoint offset  READ offset  WRITE setOffset )
	Q_PROPERTY(int    x       READ x       WRITE setX      )
	Q_PROPERTY(int    y       READ y       WRITE setY      )

	Q_PROPERTY(bool   busy    READ busy    WRITE setBusy   )
	Q_PROPERTY(double percent READ percent WRITE setPercent)

public slots:
	virtual QPoint  size  (void) const   { return m_size; }
	virtual void setSize  (QPoint value) { setWidth(value.x()); setHeight(value.y()); }

	virtual int     width (void) const   { return m_size.x();   }
	virtual void setWidth (int);

	virtual int     height(void) const   { return m_size.y();  }
	virtual void setHeight(int);

	virtual int    area   (void) const   { return width()*height(); }


	virtual QPoint  offset(void) const   { return m_offset; }
	virtual void setOffset(QPoint value) { m_offset=value; }

	virtual int     x     (void) const   { return m_offset.x();  }
	virtual void setX     (int value)    { m_offset.setX(value); }

	virtual int     y     (void) const   { return m_offset.y();  }
	virtual void setY     (int value)    { m_offset.setY(value); }


	/// returns true when image has non-zero dimension even if it does not have any colour plane
	virtual bool isEmpty(void) const { return !area(); }
	/// returns true when image has at least one pixel of data
	virtual bool hasData(void) const { return ( (!isEmpty()) && (!m_plane.isEmpty()) ); }

	virtual void clear(void);


	virtual int/*ColourSpace*/ colourSpace(void) const;


	virtual ColourPlaneSet planes      (void)                     const { return m_plane.keys().toSet(); }
	virtual QintList       planesList  (void)                     const { return m_plane.keys(); }
	virtual bool        hasPlane       (int/*ColourPlane*/ plane) const { return m_plane.find(plane) != m_plane.end(); }
	virtual bool        hasAlpha       (void)                     const { return hasPlane(Image::PLANE_ALPHA); }
	virtual int            planesCount (void)                     const { return m_plane.size(); }
	virtual bool        addPlane       (int/*ColourPlane*/);
	virtual bool     removePlane       (int/*ColourPlane*/);
	virtual bool       movePlane       (int/*ColourPlane*/ from,int/*ColourPlane*/ to);

	virtual bool           planeHasName(int/*ColourPlane*/)       const;
	virtual QString        planeName   (int/*ColourPlane*/)       const;
	virtual bool        setPlaneName   (int/*ColourPlane*/,QString);
	virtual bool      erasePlaneName   (int/*ColourPlane*/);

	virtual const Pixel *  plane       (int/*ColourPlane*/) const;
	virtual       Pixel *  plane       (int/*ColourPlane*/);


	virtual bool     hasText        (QString key) const { return m_text.find(key)!=m_text.end(); }
	virtual QString     text        (QString key) const;
	virtual TextKeys    textKeys    (void)        const { return m_text.keys().toSet(); }
	virtual QStringList textKeysList(void)        const { return m_text.keys(); }
	virtual bool     setText        (QString key,QString value);
	virtual bool   eraseText        (QString key);
	virtual void   clearTexts       (void)              { m_text.clear(); }


	virtual bool    busy   (void) const { return m_busy; }
	virtual void setBusy   (bool);

	virtual double  percent(void) const { return m_percent; }
	virtual void setPercent(double);

	virtual void  startLongProcessing(void) { setBusy(true); }
	virtual void finishLongProcessing(void) { setBusy(false); }

protected:
	QPoint m_offset;
	QPoint m_size;
	ColourPlanes m_plane;
	ColourPlaneNames m_planeName;
	Text m_text;

	volatile bool   m_busy;
	volatile double m_percent;

	virtual void onSetBusy   (void) {}
	virtual void onSetPercent(void) {}
};

#endif // IMAGINABLE__IMAGE__INCLUDED
