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


#include "plugin_clone.hpp"
#include "dbus_plugin_clone_adaptor.h"



Q_EXPORT_PLUGIN2(clone,PluginClone)


PluginClone::PluginClone(void)
	: QObject(NULL)
	, PluginInterface()
{
}

bool PluginClone::init(Root* root)
{
	m_root=root;

	if(!QDBusConnection::sessionBus().registerObject(name(),new CloneAdaptor(this),QDBusConnection::ExportNonScriptableContents))
	{
		QTextStream(stderr)<<qPrintable(QString("Cannot register Clone plugin interface in D-Bus object '%1'\n").arg(name()));
		return false;
	}
	return true;
}

bool PluginClone::clone(qulonglong from,qulonglong to)
{
	if(!from)
	{
		COMPLAIN(LOG_CRIT,from,"Non-existent source image");
		return false;
	}
	if(!to)
	{
		COMPLAIN(LOG_CRIT,from,"Non-existent destination image");
		return false;
	}
	if(from==to)
	{
		COMPLAIN(LOG_WARNING,from,"Source and destination images must differ");
		return false;
	}

	Image* src=GET_OR_COMPLAIN(from);
	if(!src)
		return false;

	Image* dst=GET_OR_COMPLAIN(to);
	if(!dst)
		return false;

	dst->copyFrom(*src);

	return true;
}

void PluginClone::lock(qulonglong Id,int msec)
{
	Image* img=GET_OR_COMPLAIN(Id);
	if(!img)
		return;

	doLongProcessing(img,QtConcurrent::run(this,&PluginClone::long_lock,img,msec));
}

void PluginClone::long_lock(Image* img,int msec)
{
	connect(this,SIGNAL(lock_percent(double)),img,SLOT(setPercent(double)));
	for(unsigned percent=0;percent<100;percent+=1)
	{
		PublicThreadSleep::msleep(msec/100);
		emit lock_percent(static_cast<double>(percent));
	}
	disconnect(img,SLOT(setPercent(double)));
}
