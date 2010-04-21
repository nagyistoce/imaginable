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

#ifndef IMAGINABLE__PLUGINS__RESIZE__RESIZE__INCLUDED
#define IMAGINABLE__PLUGINS__RESIZE__RESIZE__INCLUDED


#include "version.hpp"

#include <plugin_iface.hpp>

#include "rect.hpp"


class PluginResize : public QObject, PluginInterface
{
Q_OBJECT
Q_INTERFACES(PluginInterface)
public:
	PluginResize(void);
	~PluginResize() {}

	QString name   (void) const { return "resize"; }
	QString version(void) const { return QString::fromAscii(version::full_string()); }

signals:
	void setPercent(double);

public slots:
	QString errorCodeToString(uint errorCode) const;

	qulonglong resize     (qulonglong,int,int,uint,uint,QfullPixel);
	qulonglong resizeBlack(qulonglong,int,int,uint,uint);
	qulonglong resizeWhite(qulonglong,int,int,uint,uint);
	qulonglong crop       (qulonglong,int,int,uint,uint);
	qulonglong autoCrop   (qulonglong);

	uint lastErrorCode(qulonglong);

private:
	typedef QMap<qulonglong,uint> lastErrorCodes_t;
	lastErrorCodes_t m_lastErrorCodes;

	typedef enum
	{
		NEWCOLOUR_FULL=0,
		NEWCOLOUR_PLAIN,
		NEWCOLOUR_IGNORE
	} newColourPolicy_t;

	typedef enum
	{
		NEWSIZE_RESIZE=0,
		NEWSIZE_CROP,
		NEWSIZE_AUTOCROP
	} newSizePolicy_t;

	qulonglong resizeCommon(const char*,qulonglong,newSizePolicy_t,newColourPolicy_t,rect =rect(),QfullPixel =QfullPixel(),quint16 =0);
	inline qulonglong resizeCommon(const char* function,qulonglong Id,newSizePolicy_t newSizePolicy,newColourPolicy_t newColourPolicy,rect newSize,quint16 value)
	{ return resizeCommon(function,Id,newSizePolicy,newColourPolicy,newSize,QfullPixel(),value); }
	inline qulonglong resizeCommon(const char* function,qulonglong Id,newSizePolicy_t newSizePolicy,newColourPolicy_t newColourPolicy,QfullPixel fullPixel,quint16 value=0)
	{ return resizeCommon(function,Id,newSizePolicy,newColourPolicy,rect(),fullPixel,value); }
	inline qulonglong resizeCommon(const char* function,qulonglong Id,newSizePolicy_t newSizePolicy,newColourPolicy_t newColourPolicy,quint16 value)
	{ return resizeCommon(function,Id,newSizePolicy,newColourPolicy,rect(),QfullPixel(),value); }

	void do_resize(const char*,qulonglong,Image*,qulonglong,Image*,rect,QfullPixel,bool);

	enum
	{
		/**/CODE_INCORrect_WIDTH     = Core::CODE__CUSTOM
		,   CODE_INCORrect_HEIGHT
		,   CODE_INCORrect_NEWCOLOUR
		,   CODE_NO_ALPHA
		,   CODE_TRANSPARENT_SRC_IMAGE
	};
};

#endif // IMAGINABLE__PLUGINS__RESIZE__RESIZE__INCLUDED
