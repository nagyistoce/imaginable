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

#ifndef IMAGINABLE__CORE__INCLUDED
#define IMAGINABLE__CORE__INCLUDED


#include <QtCore/QtGlobal>

#include <sys/syslog.h> // for log levels


class Image;

class Core
{
public:
	Core(void) {}
	virtual ~Core() {}

	static Core* instance(void) { return s_instance; }

	virtual qulonglong createImage(void)                   =0;
	virtual Image*           image(qulonglong)             =0;
	virtual Image*           image(qulonglong,bool&)       =0;
	virtual bool          hasImage(qulonglong)       const =0;
	virtual uint       deleteImage(qulonglong)             =0;

	virtual QString errorCodeToString(uint) const =0;
	
	virtual void message(int level,QString message,QString source,qulonglong Id=0ULL) const =0;

	enum
	{
		/**/CODE_OK         =   0

		,   CODE_NO_IMAGE
		,   CODE_IMAGE_BUSY
		,   CODE_NO_SRC_IMAGE
		,   CODE_SRC_IMAGE_BUSY
		,   CODE_NO_DST_IMAGE
		,   CODE_DST_IMAGE_BUSY

		,   CODE__CORE

		,   CODE__CUSTOM    = 100
	};
protected:
	static Core* s_instance;
};

#endif // IMAGINABLE__CORE__INCLUDED
