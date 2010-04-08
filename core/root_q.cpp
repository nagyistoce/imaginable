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


#include "root_q.hpp"
#include "dbus_root_q_adaptor.h"
#include "main.hpp"
#include "image_q.hpp"
#include "plugin_iface.hpp"

#include <QtCore/QCoreApplication>


namespace {
	const char dbus_object_name[]="/";
}

Root_Q::Root_Q(QObject* parent)
	: QObject(parent)
	, Root()
{
	connect(&m_autoCloseTimer,SIGNAL(timeout()),this,SLOT(autoCloseTimeout()));

	m_autoCloseTimer.setSingleShot(true);
	setAutoCloseTime(1*60);

//	m_plugins<<(new PluginCloneImage(this));
}

bool Root_Q::init(void)
{
	new RootAdaptor(this);
	if(!QDBusConnection::sessionBus().registerObject(dbus_object_name,this))
	{
		QTextStream(stderr)<<qPrintable(QString("Cannot register `root` D-Bus object '%1'\n").arg(dbus_object_name));
		return false;
	}

//	foreach(PluginInterface* plugin,m_plugins)
//		plugin->init();
	return true;
}

QString Root_Q::version(void) const
{
	return QCoreApplication::applicationVersion();
}

void Root_Q::autoCloseTimeout(void)
{
	if(program_options().flag("--verbose"))
		QTextStream(stdout)<<"Root: Autoclosing now\n";
	quit();
}

void Root_Q::quit(void)
{
	QCoreApplication::quit();
}

void Root_Q::setAutoCloseTime(unsigned value)
{
	m_autoCloseTimer.setInterval(value * 60*1000);
	restartAutoCloser();
}

void Root_Q::restartAutoCloser(void)
{
	if( m_autoCloseTimer.isActive() )
		m_autoCloseTimer.stop();
	if( m_autoCloseTimer.interval() && m_images.isEmpty() )
	{
		if(program_options().flag("--verbose"))
		{
			unsigned m=(m_autoCloseTimer.interval()/1000/60) %60;
			unsigned h=(m_autoCloseTimer.interval()/1000/60/60);

			QTextStream(stdout)<<QString("Root: Autoclosing in %1h %2m\n").arg(h).arg(m,2,10,QChar('0'));
		}
		m_autoCloseTimer.start();
	}
}

qulonglong Root_Q::createImage(void)
{
	qulonglong Id=nextIndex();

	Image_Q* newImage=new Image_Q(this);
	if(newImage->init(QString("/%1").arg(Id)))
	{
		m_images[Id]=newImage;

		if(program_options().flag("--verbose"))
			QTextStream(stdout)<<QString("Root: Image[%1] created\n").arg(Id);

		emit imageCreated(Id);
	}

	restartAutoCloser();

	return Id;
}

bool Root_Q::deleteImage(qulonglong Id)
{
	Images::Iterator I=m_images.find(Id);
	if(I==m_images.end())
	{
		if(program_options().flag("--verbose"))
			QTextStream(stdout)<<QString("Root: Image[%1] is not found for deletion\n").arg(Id);
		return false;
	}

	if(I.value()->busy())
	{
		if(program_options().flag("--verbose"))
			QTextStream(stdout)<<QString("Root: Image[%1] is busy\n").arg(Id);
		return false;
	}

	delete I.value();
	m_images.erase(I);

	if(program_options().flag("--verbose"))
		QTextStream(stdout)<<QString("Root: Image[%1] deleted\n").arg(Id);

	emit imageDeleted(Id);

	restartAutoCloser();

	return true;
}

qulonglong Root_Q::nextIndex(void)
{
	if(m_images.isEmpty())
		return 1;
	Images::const_iterator I=m_images.constEnd();
	--I;
	return I.key()+1;
}

bool Root_Q::hasImage(qulonglong Id) const
{
	return m_images.constFind(Id)!=m_images.constEnd();
}

Image* Root_Q::image(qulonglong Id)
{
	Images::iterator I=m_images.find(Id);
	if(I!=m_images.end())
		return I.value();
	return NULL;
}

namespace {
	QString messageLevelToString(int level)
	{
		switch(level)
		{
			case LOG_EMERG:   return "Emergency";
			case LOG_ALERT:   return "Alert";
			case LOG_CRIT:    return "Critical";
			case LOG_ERR:     return "Error";
			case LOG_WARNING: return "Warning";
			case LOG_NOTICE:  return "Notice";
			case LOG_INFO:    return "Info";
			case LOG_DEBUG:   return "Debug";
			default: return QString("#%1").arg(level);
		}
	}
}

void Root_Q::pluginMessage(int level,QString plugin,qulonglong Id,QString message) const
{
	QTextStream(stdout)<<QString("[%1] %2: Image[%3]: %4\n").arg(messageLevelToString(level)).arg(plugin).arg(Id).arg(message);
}

bool Root_Q::loadPlugin(QString fileName)
{
	try
	{
		if(isPluginLoaded(fileName))
			throw QString("Root: Plugin[%1] cannot be loaded: already loaded\n").arg(fileName);

		if(!QFileInfo(fileName).isFile())
			throw QString("Root: Plugin[%1] cannot be loaded: file does not exist\n").arg(fileName);
	}
	catch(QString& msg)
	{
		QTextStream(stdout)<<msg;
		return false;
	}

	QPluginLoader* pluginLoader=new QPluginLoader(fileName,this);
	PluginInterface* plugin;
	try
	{
		if(!pluginLoader->load())
			throw QString("Root: Plugin[%1] cannot be loaded: load() failed: %2\n").arg(fileName).arg(pluginLoader->errorString());

		QObject* instance=pluginLoader->instance();
		if(!instance)
			throw QString("Root: Plugin[%1] cannot be loaded: instance() failed: %2\n").arg(fileName).arg(pluginLoader->errorString());

		plugin=qobject_cast<PluginInterface*>(instance);
		if(!plugin)
			throw QString("Root: Plugin[%1] cannot be loaded: not an Imaginable plugin\n").arg(fileName);

		if(!plugin->init(this))
			throw QString("Root: Plugin[%1] cannot be loaded: init() failed\n").arg(fileName);
	}
	catch(QString& msg)
	{
		delete pluginLoader;
		QTextStream(stdout)<<msg;
		return false;
	}

	m_plugins[fileName]=pluginLoader;
	QTextStream(stdout)<<QString("Root: Plugin[%1] loaded: \"%2\" Version: %3\n").arg(fileName).arg(plugin->name()).arg(plugin->version());
	return true;
}

QStringList Root_Q::loadAllPlugins(QString dirName)
{
	QStringList ret;
	QDir pluginsDir(dirName);
	if(!pluginsDir.exists())
	{
		QTextStream(stdout)<<QString("Root: Plugins[%1] cannot be loaded: directory does not exist\n").arg(dirName);
		return ret;
	}
	foreach(QString fileName,pluginsDir.entryList(QDir::Files))
	{
		QString fullPath(pluginsDir.absoluteFilePath(fileName));
		if(loadPlugin(fullPath))
			ret<<fullPath;
	}
	return ret;
}

bool Root_Q::isPluginLoaded(QString fileName) const
{
	return m_plugins.constFind(fileName)!=m_plugins.constEnd();
}

QString Root_Q::pluginName(QString fileName) const
{
	Plugins::ConstIterator I=m_plugins.constFind(fileName);
	if(I==m_plugins.constEnd())
		return QString();

	return qobject_cast<PluginInterface*>(I.value()->instance())->name();
}

QString Root_Q::pluginVersion(QString fileName) const
{
	Plugins::ConstIterator I=m_plugins.constFind(fileName);
	if(I==m_plugins.constEnd())
		return QString();

	return qobject_cast<PluginInterface*>(I.value()->instance())->version();
}

QStringList Root_Q::pluginsList(void) const
{
	return m_plugins.keys();
}

bool Root_Q::unloadPlugin(QString fileName)
{
	Plugins::iterator I=m_plugins.find(fileName);
	if(I==m_plugins.end())
	{
		QTextStream(stdout)<<QString("Root: Plugin[%1] cannot be unloaded: not loaded\n").arg(fileName);
		return false;
	}

	bool ret=I.value()->unload();
	I.value()->deleteLater();
	m_plugins.erase(I);

	if(ret)
		QTextStream(stdout)<<QString("Root: Plugin[%1] unloaded\n").arg(fileName);
	else
		QTextStream(stdout)<<QString("Root: Plugin[%1] cannot be unloaded: unload() failed\n").arg(fileName);

	return ret;
}
