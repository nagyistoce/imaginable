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


#include "image_q.hpp"
#include "dbus_image_q_main_adaptor.h"
#include "dbus_image_q_busy_adaptor.h"

#include <sys/syslog.h> // for log levels


Image_Q::Image_Q(QObject* parent)
	: Image(parent)
{
}

Image_Q::~Image_Q()
{
	QDBusConnection::sessionBus().unregisterObject(m_DBusIFaceNodeName,QDBusConnection::UnregisterTree);
}

bool Image_Q::init(QString nodeName)
{
	m_DBusIFaceNodeName=nodeName;

	if(!QDBusConnection::sessionBus().registerObject(m_DBusIFaceNodeName,new Image_busyAdaptor(this),QDBusConnection::ExportNonScriptableContents))
	{
		message(LOG_ALERT,"Cannot register D-Bus object interface `busy`");
		return false;
	}

	m_DBusIFaceMain=new Image_mainAdaptor(this);
	m_DBusIFaceImageNodeName=m_DBusIFaceNodeName+"/image";
	return set_DBus_main();
}

bool Image_Q::set_DBus_main(void)
{
	if(!QDBusConnection::sessionBus().registerObject(m_DBusIFaceImageNodeName,m_DBusIFaceMain,QDBusConnection::ExportNonScriptableContents))
	{
		message(LOG_ALERT,"Cannot register D-Bus object interface");
		return false;
	}
	return true;
}

void Image_Q::hide_DBus_main(void)
{
	QDBusConnection::sessionBus().unregisterObject(m_DBusIFaceImageNodeName);
}

QString Image_Q::colourSpaceToString(int colourSpace)
{
	switch(colourSpace)
	{
		#define CASE(SPACE,NAME) case Image::SPACE_ ## SPACE:  return #NAME ;
		CASE(_CUSTOM,Custom)
		CASE(_EMPTY,Empty)
		CASE(LIGHTNESS,Lightness)
		CASE(RGB,RGB)
		CASE(HSL,HSL)
		CASE(HSV,HSV)
		CASE(ALPHA,Alpha)
		#undef CASE
	}
	return QString();
}

QString Image_Q::colourPlaneToString(int plane)
{
	switch(plane)
	{
		#define CASE(PLANE,NAME) case Image::PLANE_ ## PLANE:  return #NAME ;
		CASE(ALPHA,Alpha)
		CASE(RED,Red)
		CASE(GREEN,Green)
		CASE(BLUE,Blue)
		CASE(HUE_SECTOR,Hue_sector)
		CASE(HUE,Hue)
		CASE(SATURATION,Saturation)
		CASE(HSV_VALUE,HSV_Value)
		CASE(LIGHTNESS,Lightness)
		#undef CASE
	}
	return QString();
}

void Image_Q::onSetBusy(void)
{
	if(m_busy)
	{
		emit longProcessingStarted();
		hide_DBus_main();
	}
	else
	{
		set_DBus_main();
		emit longProcessingFinished();
	}
}

void Image_Q::onSetPercent(void)
{
	emit percentChanged(percent());
}
