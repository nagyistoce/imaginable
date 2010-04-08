/* * * * * *
 *
 * Project:   Imaginable
 * Created:   2010-03-18
 * Author:    Kuzma Shapran <Kuzma.Shapran@gmail.com>
 * License:   GPLv3
 *
 * * * * * */
// $Id$


#include "image_q.hpp"
#include "dbus_image_q_main_adaptor.h"
#include "dbus_image_q_busy_adaptor.h"


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
		QTextStream(stderr)<<qPrintable(QString("Cannot register `busy` interface in D-Bus object '%1'\n").arg(m_DBusIFaceNodeName));
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
		QTextStream(stderr)<<qPrintable(QString("Cannot register `image` interface in D-Bus object '%1'\n").arg(m_DBusIFaceImageNodeName));
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
