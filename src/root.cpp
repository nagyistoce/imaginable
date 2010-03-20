/* * * * * *
 *
 * Title:     Imaginable
 * Created:   2010-02-16
 * Author:    Kuzma Shapran <Kuzma.Shapran@gmail.com>
 * License:   GPLv3
 *
 * * * * * */
// $Id$


#include "types.hpp"
#include "root.hpp"
#include "dbus_a_root.h"
#include "main.hpp"

#include <QtCore/QCoreApplication>


namespace {
	const char dbus_object_name[]="/";
}

Root::Root(QObject* parent)
	: QObject(parent)
{
	qDBusRegisterMetaType<QintList>();

	connect(&m_autoCloseTimer,SIGNAL(timeout()),this,SLOT(timeout()));

	m_autoCloseTimer.setSingleShot(true);
	setAutoCloseTime(1*60);
}

bool Root::init_DBus(void)
{
	new DBus_Adaptor_Root(this);
	if(!QDBusConnection::sessionBus().registerObject(dbus_object_name,this))
	{
		QTextStream(stderr)<<qPrintable(QString("Cannot register Root D-Bus object '%1'\n").arg(dbus_object_name));
		return false;
	}
	return true;
}

QString Root::version(void) const
{
	return QCoreApplication::applicationVersion();
}

void Root::timeout(void)
{
	if(program_options().flag("--verbose"))
		QTextStream(stdout)<<"Root: Autoclosing\n";
	quit();
}

void Root::quit(void)
{
	QCoreApplication::quit();
}

void Root::setAutoCloseTime(unsigned value)
{
	m_autoCloseTimer.setInterval(value * 60*1000);
	touch();
}

void Root::touch(void)
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

qulonglong Root::newImage(void)
{
	qulonglong Id=nextIndex();

	boost::shared_ptr<Image_Q> newImage(new Image_Q);
	if(newImage->init_DBus(QString("/%1").arg(Id)))
	{
		m_images[Id]=newImage;
		if(program_options().flag("--verbose"))
			QTextStream(stdout)<<QString("Root: New image created. Id=%1\n").arg(Id);
	}

	touch();

	return Id;
}

qulonglong Root::cloneImage(qulonglong oldId)
{
	ImagesMap::const_iterator I=m_images.constFind(oldId);
	if(I==m_images.constEnd())
		return 0;

	qulonglong newId=nextIndex();
	boost::shared_ptr<Image_Q> newImage(new Image_Q(*I.value().get()));
	if(newImage->init_DBus(QString("/%1").arg(newId)))
	{
		m_images[newId]=newImage;
		if(program_options().flag("--verbose"))
			QTextStream(stdout)<<QString("Root: Image with id=%1 cloned. New id=%2\n").arg(oldId).arg(newId);
	}

	touch();

	return newId;
}

void Root::deleteImage(qulonglong Id)
{
	ImagesMap::iterator I=m_images.find(Id);
	if(I!=m_images.end())
	{
		m_images.erase(I);
		if(program_options().flag("--verbose"))
			QTextStream(stdout)<<QString("Root: Image with id=%1 deleted\n").arg(Id);
		touch();
	}
}

qulonglong Root::nextIndex(void)
{
	if(m_images.isEmpty())
		return 1;
	ImagesMap::const_iterator I=m_images.constEnd();
	--I;
	return I.key()+1;
}
