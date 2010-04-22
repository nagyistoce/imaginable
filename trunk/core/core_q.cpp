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


#include "core_q.hpp"
#include "dbus_core_q_adaptor.h"
#include "main.hpp"
#include "image_q.hpp"
#include "plugin_iface.hpp"

#include <QtCore/QCoreApplication>



namespace {
	const char dbus_object_name[]="/";
}

Core_Q::Core_Q(QObject* parent)
	: QObject(parent)
	, Core()
{
	message(LOG_INFO,"Starting");

	connect(&m_autoCloseTimer,SIGNAL(timeout()),this,SLOT(autoCloseTimeout()));

	m_autoCloseTimer.setSingleShot(true);
	setAutoCloseTime(1*60);
}

bool Core_Q::init(void)
{
	new CoreAdaptor(this);
	if(!QDBusConnection::sessionBus().registerObject(dbus_object_name,this))
	{
		message(LOG_ALERT,"Cannot register D-Bus object interface");
		return false;
	}
	return true;
}

QString Core_Q::version(void) const
{
	return QCoreApplication::applicationVersion();
}

void Core_Q::autoCloseTimeout(void)
{
	message(LOG_NOTICE,"Autoclosing now");
	quit();
}

void Core_Q::quit(void)
{
	message(LOG_INFO,"Quitting");
	QCoreApplication::quit();
}

void Core_Q::setAutoCloseTime(unsigned value)
{
	m_autoCloseTimer.setInterval(value * 60*1000);
	restartAutoCloser();
}

void Core_Q::restartAutoCloser(void)
{
	if( m_autoCloseTimer.isActive() )
	{
		m_autoCloseTimer.stop();
		message(LOG_INFO,"Autoclosing cancelled");
	}
	if( m_autoCloseTimer.interval() && m_images.isEmpty() )
	{
		{
			unsigned m=(m_autoCloseTimer.interval()/1000/60) %60;
			unsigned h=(m_autoCloseTimer.interval()/1000/60/60);

			message(LOG_INFO,QString("Autoclosing in %1h %2m").arg(h).arg(m,2,10,QChar('0')));
		}
		m_autoCloseTimer.start();
	}
}

qulonglong Core_Q::createImage(void)
{
	qulonglong Id=nextIndex();

	Image_Q* newImage=new Image_Q(this);
	if(newImage->init(QString("/%1").arg(Id)))
	{
		m_images[Id]=newImage;

		message(LOG_NOTICE,"Image created",Id);

		emit imageCreated(Id);
	}

	restartAutoCloser();

	return Id;
}

uint Core_Q::deleteImage(qulonglong Id)
{
	Images::Iterator I=m_images.find(Id);
	if(I==m_images.end())
	{
		message(LOG_ERR,"Image is not found for deletion",Id);
		return CODE_NO_IMAGE;
	}

	if(I.value()->busy())
	{
		message(LOG_WARNING,"Image is busy",Id);
		return CODE_IMAGE_BUSY;
	}

	delete I.value();
	m_images.erase(I);

	message(LOG_NOTICE,"Image deleted",Id);

	emit imageDeleted(Id);

	restartAutoCloser();

	return CODE_OK;
}

qulonglong Core_Q::nextIndex(void)
{
	if(m_images.isEmpty())
		return 1;
	Images::const_iterator I=m_images.constEnd();
	--I;
	return I.key()+1;
}

bool Core_Q::hasImage(qulonglong Id) const
{
	return m_images.constFind(Id)!=m_images.constEnd();
}

Image* Core_Q::image(qulonglong Id)
{
	Images::iterator I=m_images.find(Id);
	if(I!=m_images.end())
		return I.value();
	return NULL;
}

Image* Core_Q::image(qulonglong Id,bool& busy)
{
	Image* ret=image(Id);
	busy=false;
	if(ret)
	{
		busy=ret->busy();
		if(busy)
			ret=NULL;
	}
	return ret;
}

QulonglongList Core_Q::imagesList(void) const
{
	return m_images.keys();
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

void Core_Q::message(int level,QString text,QString source,qulonglong Id) const
{
	QDateTime now(QDateTime::currentDateTime());
	QTextStream(stdout)<<(Id?
		(QString("%1 [%2] %3: Image[%4]: %5\n").arg(now.toString("yyyy-MM-dd hh:mm:ss.zzz")).arg(messageLevelToString(level),-9).arg(source).arg(Id).arg(text)):
		(QString("%1 [%2] %3: %4\n")           .arg(now.toString("yyyy-MM-dd hh:mm:ss.zzz")).arg(messageLevelToString(level),-9).arg(source)        .arg(text)));
}

uint Core_Q::loadPlugin(QString fileName)
{
	uint ret=CODE_OK;
	QString msg;

	fileName=QFileInfo(fileName).absoluteFilePath();

	do{
		if(isPluginLoaded(fileName))
		{
			msg=QString("Plugin[%1] cannot be loaded: already loaded").arg(fileName);
			ret=CODE_DUPLICATE_PLUGIN;
			break;
		}

		if(!QFileInfo(fileName).isFile())
		{
			msg=QString("Plugin[%1] cannot be loaded: file does not exist").arg(fileName);
			ret=CODE_NO_PLUGIN_FILE;
			break;
		}
	}while(false);
	if(ret)
	{
		message(LOG_ERR,msg);
		return ret;
	}

	QPluginLoader* pluginLoader=new QPluginLoader(fileName,this);
	PluginInterface* plugin;
	do{
		if(!pluginLoader->load())
		{
			msg=QString("Plugin[%1] cannot be loaded: load() failed: %2").arg(fileName).arg(pluginLoader->errorString());
			ret=CODE_PLUGINLOADER_FAILURE;
			break;
		}

		QObject* instance=pluginLoader->instance();
		if(!instance)
		{
			msg=QString("Plugin[%1] cannot be loaded: instance() failed: %2").arg(fileName).arg(pluginLoader->errorString());
			ret=CODE_PLUGINLOADER_FAILURE;
			break;
		}

		plugin=qobject_cast<PluginInterface*>(instance);
		if(!plugin)
		{
			msg=QString("Plugin[%1] cannot be loaded: not an Imaginable plugin").arg(fileName);
			ret=CODE_INVALID_PLUGIN;
			break;
		}

		if( (plugin->name().isEmpty())
		||  (plugin->name().contains('/')) )
		{
			msg=QString("Plugin[%1] cannot be loaded: invalid plugin name").arg(fileName);
			ret=CODE_INVALID_PLUGIN_NAME;
			break;
		}

		plugin->init(this);
		if(!QDBusConnection::sessionBus().registerObject(QString("/")+plugin->name(),instance))
		{
			msg=QString("Plugin[%1] cannot be loaded: init() failed").arg(fileName);
			ret=CODE_PLUGIN_DBUS_REGISTRATION;
			break;
		}
	}
	while(false);
	if(ret)
	{
		delete pluginLoader;
		message(LOG_CRIT,msg);
		return ret;
	}

	m_plugins[fileName]=pluginLoader;
	message(LOG_NOTICE,QString("Plugin[%1] loaded: \"%2\" Version: %3").arg(fileName).arg(plugin->name()).arg(plugin->version()));
	return ret;
}

QStringList Core_Q::loadAllPlugins(QString dirName)
{
	QStringList ret;
	QDir pluginsDir(dirName);
	if(!pluginsDir.exists())
	{
		message(LOG_ERR,QString("Plugins[%1] cannot be loaded: directory does not exist").arg(dirName));
		return ret;
	}
	foreach(QString fileName,pluginsDir.entryList(QDir::Files))
	{
		QString fullPath(QFileInfo(pluginsDir.filePath(fileName)).absoluteFilePath());
		if(!loadPlugin(fullPath))
			ret<<fullPath;
	}
	return ret;
}

QStringList Core_Q::autoLoadPlugins(QStringList names)
{
	QStringList ret;
	foreach(QString name,names)
	{
		if(QFileInfo(name).isFile())
		{
			if(!loadPlugin(name))
				ret<<name;
		}
		else if(QFileInfo(name).isDir())
		{
			ret<<loadAllPlugins(name);
		}
	}
	return ret;
}

bool Core_Q::isPluginLoaded(QString fileName) const
{
	return m_plugins.constFind(fileName)!=m_plugins.constEnd();
}

QString Core_Q::pluginName(QString fileName) const
{
	Plugins::ConstIterator I=m_plugins.constFind(fileName);
	if(I==m_plugins.constEnd())
		return QString();

	return qobject_cast<PluginInterface*>(I.value()->instance())->name();
}

QString Core_Q::pluginVersion(QString fileName) const
{
	Plugins::ConstIterator I=m_plugins.constFind(fileName);
	if(I==m_plugins.constEnd())
		return QString();

	return qobject_cast<PluginInterface*>(I.value()->instance())->version();
}

QStringList Core_Q::pluginsList(void) const
{
	return m_plugins.keys();
}

uint Core_Q::unloadPlugin(QString fileName)
{
	Plugins::iterator I=m_plugins.find(fileName);
	if(I==m_plugins.end())
	{
		message(LOG_ERR,QString("Plugin[%1] cannot be unloaded: not loaded").arg(fileName));
		return CODE_NO_PLUGIN_LOADED;
	}

	bool ok=I.value()->unload();
	I.value()->deleteLater();
	m_plugins.erase(I);

	if(ok)
		message(LOG_NOTICE,QString("Plugin[%1] unloaded").arg(fileName));
	else
		message(LOG_CRIT,QString("Plugin[%1] cannot be unloaded: unload() failed").arg(fileName));

	return ok ?
		static_cast<uint>(CODE_OK) :
		static_cast<uint>(CODE_PLUGINLOADER_FAILURE);
}

QString Core_Q::errorCodeToString(uint errorCode) const
{
	switch(errorCode)
	{
		#define CASE(ERR,STR) case CODE_##ERR: return STR ;
		CASE(OK,"OK")

		CASE(NO_IMAGE      ,"No image")
		CASE(IMAGE_BUSY    ,"Image is busy")
		CASE(NO_SRC_IMAGE  ,"No source image")
		CASE(SRC_IMAGE_BUSY,"Source image is busy")
		CASE(NO_DST_IMAGE  ,"No destination image")
		CASE(DST_IMAGE_BUSY,"Destination image is busy")

		CASE(DUPLICATE_PLUGIN        ,"Duplicate plugin")
		CASE(NO_PLUGIN_FILE          ,"No plugin file")
		CASE(PLUGINLOADER_FAILURE    ,"PluginLoader failure")
		CASE(INVALID_PLUGIN          ,"Invalid plugin")
		CASE(INVALID_PLUGIN_NAME     ,"Invalid plugin name")
		CASE(PLUGIN_DBUS_REGISTRATION,"Cannot register plugin interface on D-Bus")
		CASE(NO_PLUGIN_LOADED        ,"No plugin loaded")
		#undef CASE
	}
	return QString();
}
