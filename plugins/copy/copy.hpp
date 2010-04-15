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

#ifndef IMAGINABLE__PLUGINS__COPY_COPY__INCLUDED
#define IMAGINABLE__PLUGINS__COPY_COPY__INCLUDED


#include "version.hpp"

#include <plugin_iface.hpp>

/*
class PublicThreadSleep: public QThread
{
public:
	static void  sleep(unsigned long v) { QThread:: sleep(v); }
	static void msleep(unsigned long v) { QThread::msleep(v); }
	static void usleep(unsigned long v) { QThread::usleep(v); }
};
*/
class PluginCopy : public QObject, PluginInterface
{
Q_OBJECT
Q_INTERFACES(PluginInterface)
public:
	PluginCopy(void);
	~PluginCopy() {}

	bool init(Root*);
	QString name   (void) const { return "/copy"; }
	QString version(void) const { return QString::fromAscii(version::full_string()); }

signals:
	void lock_percent(double);

public slots:
	QString errorCodeToString(uint errorCode) const;

	uint copyTo(qulonglong from,qulonglong to);
	qulonglong copyNew(qulonglong);

//	void lock(qulonglong,int);

private:
//	void long_lock(Image*,int);

	enum
	{
		/**/CODE_IMAGES_DONT_DIFFER = Root::CODE__CUSTOM
	};
};

#endif // IMAGINABLE__PLUGINS__COPY_COPY__INCLUDED
