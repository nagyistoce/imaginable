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


#include "invert.hpp"
#include "dbus_plugin_invert_adaptor.h"


Q_EXPORT_PLUGIN2(invert,PluginInvert)


PluginInvert::PluginInvert(void)
	: QObject(NULL)
	, PluginInterface()
{
	new InvertAdaptor(this);
}

uint PluginInvert::invertAll(qulonglong Id)
{
	bool busy;
	Image* img=getOrComplain(__FUNCTION__,"image",Id,busy);
	if(!img)
		return busy?(Core::CODE_IMAGE_BUSY):(Core::CODE_NO_IMAGE);


	message(LOG_INFO,__FUNCTION__,QString("Inverting"),Id);

	doLongProcessing(img,QtConcurrent::run(boost::bind(&PluginInvert::do_invertAll,this,Id,img)));

	return Core::CODE_OK;
}

void PluginInvert::do_invertAll(qulonglong Id,Image* img)
{
	connect(this,SIGNAL(setPercent(double)),img,SLOT(setPercent(double)));

	double from=0.;
	double step=100./static_cast<double>(img->planesList().size());
	foreach(int colourPlane,img->planesList())
	{
		do_invert(Id,img,colourPlane,from,step);
		from+=step;
	}

	disconnect(img,SLOT(setPercent(double)));

	message(LOG_INFO,"invertAll",QString("Inverted"),Id);
}

uint PluginInvert::invert(qulonglong Id,int colourPlane)
{
	bool busy;
	Image* img=getOrComplain(__FUNCTION__,"image",Id,busy);
	if(!img)
		return busy?(Core::CODE_IMAGE_BUSY):(Core::CODE_NO_IMAGE);

	if(!img->hasPlane(colourPlane))
		return CODE_NO_COLOUR_PLANE;


	message(LOG_INFO,__FUNCTION__,QString("Inverting colour plane [%1]").arg(colourPlane),Id);


	doLongProcessing(img,QtConcurrent::run(boost::bind(&PluginInvert::do_invertPlain,this,Id,img,colourPlane)));

	return Core::CODE_OK;
}

void PluginInvert::do_invertPlain(qulonglong Id,Image* img,int colourPlane)
{
	connect(this,SIGNAL(setPercent(double)),img,SLOT(setPercent(double)));

	do_invert(Id,img,colourPlane,0.,100.);

	disconnect(img,SLOT(setPercent(double)));
}

void PluginInvert::do_invert(qulonglong Id,Image* img,int colourPlane,double fromPercent,double stepPercent)
{
	const int& width =img->width();
	const int& height=img->height();

	Image::Pixel* data=img->plane(colourPlane);

	for(int y=0;y<height;++y)
	{
		emit setPercent(fromPercent+static_cast<double>(y)*stepPercent/static_cast<double>(height));

		int yo=y*width;
		for(int x=0;x<width;++x)
			data[yo+x]^=0xffff;
	}

	message(LOG_INFO,"invert",QString("Colour plane [%1] inverted").arg(colourPlane),Id);
}

QString PluginInvert::errorCodeToString(uint errorCode) const
{
	switch(errorCode)
	{
		#define CASE(ERR,STR) case CODE_##ERR: return STR ;
		CASE(NO_COLOUR_PLANE,"No colour plane")
		#undef CASE
	}
	return m_core->errorCodeToString(errorCode);
}
