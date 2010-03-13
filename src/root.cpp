/* * * * * *
 *
 * Title:     Imaginable
 * Created:   2010-02-16
 * Author:    Kuzma Shapran
 * Copyright: Kuzma Shapran <Kuzma.Shapran@gmail.com>
 * License:   GPLv3
 *
 * * * * * */


#include "root.hpp"
#include "dbus_a_root.h"

#include <QtCore/QCoreApplication>


namespace {

	const char dbus_object_name[]="/";

}

Root::Root(QObject* parent)
	: QObject(parent)
{
	connect(&m_autoCloseTimer,SIGNAL(timeout()),this,SLOT(quit()));

	m_autoCloseTimer.setInterval(24*60 * 60*1000);
	m_autoCloseTimer.setSingleShot(true);
	m_autoCloseTimer.start();
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
	if(m_autoCloseTimer.isActive())
		m_autoCloseTimer.stop();
	if(m_autoCloseTimer.interval() /* && m_images.isEmpty() */)
		m_autoCloseTimer.start();
}
//add image -> touch()
//del image -> touch()
