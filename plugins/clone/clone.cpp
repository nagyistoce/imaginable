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


#include "clone.hpp"
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
		COMPLAIN2(LOG_ALERT,"Cannot register D-Bus object interface");
		return false;
	}
	return true;
}

uint PluginClone::cloneTo(qulonglong from,qulonglong to)
{
	if( (from==to)
	&& from
	&& to )
	{
		COMPLAIN(LOG_WARNING,"Source and destination images must differ",from);
		return Root::SAME_IMAGE;
	}

	uint ret;
	Image* src=GET_OR_COMPLAIN("source image",from,ret);
	if(!src)
		return ret;

	Image* dst=GET_OR_COMPLAIN("destination image",to,ret);
	if(!dst)
		return ret+100;

	dst->copyFrom(*src);
	MESSAGE(LOG_INFO,QString("Cloned from image[%1]").arg(from),to);

	return Root::OK;
}

qulonglong PluginClone::clone(qulonglong from)
{
	qulonglong to=m_root->createImage();
	if(!to)
	{
		COMPLAIN(LOG_CRIT,"Cannot create destination image",to);
		return 0ULL;
	}

	if(cloneTo(from,to))
	{
		m_root->deleteImage(to);
		return 0ULL;
	}

	return to;
}
/*
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
*/
