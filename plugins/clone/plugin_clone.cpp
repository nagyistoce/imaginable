/* * * * * *
 *
 * Title:     Imaginable
 * Created:   2010-03-21
 * Author:    Kuzma Shapran <Kuzma.Shapran@gmail.com>
 * License:   GPLv3
 *
 * * * * * */
// $Id$


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
