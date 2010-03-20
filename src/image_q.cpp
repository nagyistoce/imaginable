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
#include "dbus_a_image_q.h"


Image_Q::Image_Q(QObject* parent)
	: QObject(parent)
	, Image()
{
}

Image_Q::Image_Q(const Image_Q& src,QObject* parent)
	: QObject(parent)
	, Image()
{
	copyFrom(src);
}

bool Image_Q::init_DBus(QString nodeName)
{
	new DBus_Adaptor_Image_Q(this);
	if(!QDBusConnection::sessionBus().registerObject(nodeName,this))
	{
		QTextStream(stderr)<<qPrintable(QString("Cannot register Image D-Bus object '%1'\n").arg(nodeName));
		return false;
	}
	return true;
}

QString Image_Q::colourSpaceToString(unsigned colourSpace)
{
	switch(colourSpace)
	{
		#define CASE(SPACE,NAME) case SPACE_ ## SPACE:  return #NAME ;
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
		#define CASE(PLANE,NAME) case PLANE_ ## PLANE:  return #NAME ;
		CASE(ALPHA,Alpha)
		CASE(RED,Red)
		CASE(GREEN,Green)
		CASE(BLUE,Blue)
		CASE(HUE_SECTOR,Hue_sector)
		CASE(HUE,Hue)
		CASE(HSV_SATURATION,HSV_Saturation)
		CASE(HSV_VALUE,HSV_Value)
		CASE(HSL_SATURATION,HSL_Saturation)
		CASE(LIGHTNESS,Lightness)
		#undef CASE
	}
	return QString();
}

QintList Image_Q::planes(void) const
{
	ColourPlaneSet colourPlaneSet=Image::planes();
	QintList ret;
	for(ColourPlaneSet::const_iterator I=colourPlaneSet.begin();I!=colourPlaneSet.end();++I)
		ret<<*I;
	return ret;
}

QStringList Image_Q::textKeys(void) const
{
	TextKeys textKeys=Image::textKeys();
	QStringList ret;
	for(TextKeys::const_iterator I=textKeys.begin();I!=textKeys.end();++I)
		ret<<QString::fromLocal8Bit(I->c_str());
	return ret;
}
