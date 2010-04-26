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


#include "colour_adjust.hpp"
#include "dbus_plugin_colour_adjust_adaptor.h"


Q_EXPORT_PLUGIN2(colour_adjust,PluginColourAdjust)


PluginColourAdjust::PluginColourAdjust(void)
	: QObject(NULL)
	, PluginInterface()
{
	qDBusRegisterMetaType<QAdjustPoint>();

	new Colour_adjustAdaptor(this);
}




uint PluginColourAdjust::invertAll(qulonglong Id)
{
	bool busy;
	Image* img=getOrComplain(__FUNCTION__,"image",Id,busy);
	if(!img)
		return busy?(Core::CODE_IMAGE_BUSY):(Core::CODE_NO_IMAGE);


	message(LOG_INFO,__FUNCTION__,QString("Inverting"),Id);

	doLongProcessing(img,QtConcurrent::run(boost::bind(&PluginColourAdjust::do_invertAll,this,Id,img)));

	return Core::CODE_OK;
}

void PluginColourAdjust::do_invertAll(qulonglong Id,Image* img)
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

uint PluginColourAdjust::invert(qulonglong Id,int colourPlane)
{
	bool busy;
	Image* img=getOrComplain(__FUNCTION__,"image",Id,busy);
	if(!img)
		return busy?(Core::CODE_IMAGE_BUSY):(Core::CODE_NO_IMAGE);

	if(!img->hasPlane(colourPlane))
		return CODE_NO_COLOUR_PLANE;


	message(LOG_INFO,__FUNCTION__,QString("Inverting colour plane [%1]").arg(colourPlane),Id);


	doLongProcessing(img,QtConcurrent::run(boost::bind(&PluginColourAdjust::do_invertPlain,this,Id,img,colourPlane)));

	return Core::CODE_OK;
}

void PluginColourAdjust::do_invertPlain(qulonglong Id,Image* img,int colourPlane)
{
	connect(this,SIGNAL(setPercent(double)),img,SLOT(setPercent(double)));

	do_invert(Id,img,colourPlane,0.,100.);

	disconnect(img,SLOT(setPercent(double)));
}

void PluginColourAdjust::do_invert(qulonglong Id,Image* img,int colourPlane,double fromPercent,double stepPercent)
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




namespace {
	void gamma_Pixel(Image::Pixel& p,double value)
	{
		if(value<0.)
			p=static_cast<Image::Pixel>(0xffffu*( (exp(-value*(static_cast<double>(p)/65535.))-1.)/(exp(-value)-1.)           ));
		if(value>0.)
			p=static_cast<Image::Pixel>(0xffffu*(  log(       (static_cast<double>(p)/65535.)     *(exp( value)-1.)+1.)/value ));
	}
}

uint PluginColourAdjust::gammaAll(qulonglong Id,double value)
{
	bool busy;
	Image* img=getOrComplain(__FUNCTION__,"image",Id,busy);
	if(!img)
		return busy?(Core::CODE_IMAGE_BUSY):(Core::CODE_NO_IMAGE);


	message(LOG_INFO,__FUNCTION__,QString("Applying gamma [%1]").arg(value),Id);

	doLongProcessing(img,QtConcurrent::run(boost::bind(&PluginColourAdjust::do_gammaAll,this,Id,img,value)));

	return Core::CODE_OK;
}

void PluginColourAdjust::do_gammaAll(qulonglong Id,Image* img,double value)
{
	connect(this,SIGNAL(setPercent(double)),img,SLOT(setPercent(double)));

	double from=0.;
	double step=100./static_cast<double>(img->planesList().size());
	foreach(int colourPlane,img->planesList())
	{
		do_gamma(Id,img,colourPlane,value,from,step);
		from+=step;
	}

	disconnect(img,SLOT(setPercent(double)));

	message(LOG_INFO,"gammaAll",QString("Gamma [%1] applied").arg(value),Id);
}

uint PluginColourAdjust::gamma(qulonglong Id,int colourPlane,double value)
{
	bool busy;
	Image* img=getOrComplain(__FUNCTION__,"image",Id,busy);
	if(!img)
		return busy?(Core::CODE_IMAGE_BUSY):(Core::CODE_NO_IMAGE);

	if(!img->hasPlane(colourPlane))
		return CODE_NO_COLOUR_PLANE;


	message(LOG_INFO,__FUNCTION__,QString("Applying gamma [%1] to colour plane [%2]").arg(value).arg(colourPlane),Id);


	doLongProcessing(img,QtConcurrent::run(boost::bind(&PluginColourAdjust::do_gammaPlain,this,Id,img,colourPlane,value)));

	return Core::CODE_OK;
}

void PluginColourAdjust::do_gammaPlain(qulonglong Id,Image* img,int colourPlane,double value)
{
	connect(this,SIGNAL(setPercent(double)),img,SLOT(setPercent(double)));

	do_gamma(Id,img,colourPlane,value,0.,100.);

	disconnect(img,SLOT(setPercent(double)));
}

void PluginColourAdjust::do_gamma(qulonglong Id,Image* img,int colourPlane,double value,double fromPercent,double stepPercent)
{
	const int& width =img->width();
	const int& height=img->height();

	Image::Pixel* data=img->plane(colourPlane);

	for(int y=0;y<height;++y)
	{
		emit setPercent(fromPercent+static_cast<double>(y)*stepPercent/static_cast<double>(height));

		int yo=y*width;
		for(int x=0;x<width;++x)
		{
			int xyo=yo+x;
			gamma_Pixel(data[xyo],value);
		}
	}

	message(LOG_INFO,"gamma",QString("Gamma [%1] applied to colour plane [%2]").arg(value).arg(colourPlane),Id);
}




uint PluginColourAdjust::curveAll(qulonglong Id,QAdjustPoint value)
{
	bool busy;
	Image* img=getOrComplain(__FUNCTION__,"image",Id,busy);
	if(!img)
		return busy?(Core::CODE_IMAGE_BUSY):(Core::CODE_NO_IMAGE);


	message(LOG_INFO,__FUNCTION__,"Applying custom curve",Id);

	doLongProcessing(img,QtConcurrent::run(boost::bind(&PluginColourAdjust::do_curveAll,this,Id,img,value)));

	return Core::CODE_OK;
}

void PluginColourAdjust::do_curveAll(qulonglong Id,Image* img,QAdjustPoint value)
{
	connect(this,SIGNAL(setPercent(double)),img,SLOT(setPercent(double)));

	double from=0.;
	double step=100./static_cast<double>(img->planesList().size());
	foreach(int colourPlane,img->planesList())
	{
		do_curve(Id,img,colourPlane,value,from,step);
		from+=step;
	}

	disconnect(img,SLOT(setPercent(double)));

	message(LOG_INFO,"curveAll","Custom curve applied",Id);
}

uint PluginColourAdjust::curve(qulonglong Id,int colourPlane,QAdjustPoint value)
{
	bool busy;
	Image* img=getOrComplain(__FUNCTION__,"image",Id,busy);
	if(!img)
		return busy?(Core::CODE_IMAGE_BUSY):(Core::CODE_NO_IMAGE);

	if(!img->hasPlane(colourPlane))
		return CODE_NO_COLOUR_PLANE;


	message(LOG_INFO,__FUNCTION__,QString("Applying custom curve to colour plane [%1]").arg(colourPlane),Id);


	doLongProcessing(img,QtConcurrent::run(boost::bind(&PluginColourAdjust::do_curvePlain,this,Id,img,colourPlane,value)));

	return Core::CODE_OK;
}

void PluginColourAdjust::do_curvePlain(qulonglong Id,Image* img,int colourPlane,QAdjustPoint value)
{
	connect(this,SIGNAL(setPercent(double)),img,SLOT(setPercent(double)));

	do_curve(Id,img,colourPlane,value,0.,100.);

	disconnect(img,SLOT(setPercent(double)));
}

void PluginColourAdjust::do_curve(qulonglong Id,Image* img,int colourPlane,QAdjustPoint value,double fromPercent,double stepPercent)
{
	const int& width =img->width();
	const int& height=img->height();

	Image::Pixel* data=img->plane(colourPlane);

	for(int y=0;y<height;++y)
	{
		emit setPercent(fromPercent+static_cast<double>(y)*stepPercent/static_cast<double>(height));

		int yo=y*width;
		for(int x=0;x<width;++x)
		{
			int xyo=yo+x;
//			curve_Pixel(data[xyo],value);
		}
	}

	message(LOG_INFO,"curve",QString("Custom curve applied to colour plane [%1]").arg(colourPlane),Id);
}




QString PluginColourAdjust::errorCodeToString(uint errorCode) const
{
	switch(errorCode)
	{
		#define CASE(ERR,STR) case CODE_##ERR: return STR ;
		CASE(NO_COLOUR_PLANE,"No colour plane")
		#undef CASE
	}
	return m_core->errorCodeToString(errorCode);
}
