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

#include "linear.hpp"
#include "cubic_hermite_spline.hpp"

#include <boost/shared_ptr.hpp>


Q_EXPORT_PLUGIN2(colour_adjust,PluginColourAdjust)


QFunctionList PluginColourAdjust::functionList;

namespace {
	enum {
		CURVE_FUNCTION_LINEAR=0,
		CURVE_FUNCTION_CUBIC_HERMITE_SPLINE
	};
}


PluginColourAdjust::PluginColourAdjust(void)
	: QObject(NULL)
	, PluginInterface()
{
	qDBusRegisterMetaType<QintList>();
	qDBusRegisterMetaType<QAdjustPoint>();
	qDBusRegisterMetaType<QFunctionList>();

	if(functionList.isEmpty())
	{
		functionList[CURVE_FUNCTION_LINEAR]="Linear";
		functionList[CURVE_FUNCTION_CUBIC_HERMITE_SPLINE]="Cubic Hermite Spline";
	}

	new Colour_adjustAdaptor(this);
}




uint PluginColourAdjust::gammaAll(qulonglong Id,double value)
{
	bool busy;
	Image* img=getOrComplain(__FUNCTION__,"image",Id,busy);
	if(!img)
		return busy?(Core::CODE_IMAGE_BUSY):(Core::CODE_NO_IMAGE);

	message(LOG_INFO,__FUNCTION__,QString("Applying gamma [%1] to all colour planes").arg(value),Id);


	doLongProcessing(img,QtConcurrent::run(boost::bind(&PluginColourAdjust::do_gammaAll,this,Id,img,value)));

	return Core::CODE_OK;
}

void PluginColourAdjust::do_gammaAll(qulonglong Id,Image* img,double value)
{
	do_gamma(Id,img,img->planesList(),value);
}

uint PluginColourAdjust::gamma(qulonglong Id,QintList colourPlanes,double value)
{
	bool busy;
	Image* img=getOrComplain(__FUNCTION__,"image",Id,busy);
	if(!img)
		return busy?(Core::CODE_IMAGE_BUSY):(Core::CODE_NO_IMAGE);

	if(colourPlanes.isEmpty())
		return CODE_NO_COLOUR_PLANE;

	foreach(int colourPlane,colourPlanes)
		if(!img->hasPlane(colourPlane))
			return CODE_NO_COLOUR_PLANE;


	QString colourPlanes_str;
	foreach(int colourPlane,colourPlanes)
		colourPlanes_str+=QString((colourPlanes_str.isEmpty())?"%1":", %1").arg(colourPlane);

	message(LOG_INFO,__FUNCTION__,QString("Applying gamma [%1] to colour planes [%2]").arg(value).arg(colourPlanes_str),Id);


	doLongProcessing(img,QtConcurrent::run(boost::bind(&PluginColourAdjust::do_gamma,this,Id,img,colourPlanes,value)));

	return Core::CODE_OK;
}

void PluginColourAdjust::do_gamma(qulonglong Id,Image* img,QintList colourPlanes,double value)
{
	connect(this,SIGNAL(setPercent(double)),img,SLOT(setPercent(double)));

	double from=0.;
	double step=100./static_cast<double>(colourPlanes.size());
	foreach(int colourPlane,colourPlanes)
	{
		do_gammaPlain(Id,img,colourPlane,value,from,step);
		from+=step;
	}

	disconnect(img,SLOT(setPercent(double)));
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

void PluginColourAdjust::do_gammaPlain(qulonglong Id,Image* img,int colourPlane,double value,double fromPercent,double stepPercent)
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




uint PluginColourAdjust::invertAll(qulonglong Id)
{
	bool busy;
	Image* img=getOrComplain(__FUNCTION__,"image",Id,busy);
	if(!img)
		return busy?(Core::CODE_IMAGE_BUSY):(Core::CODE_NO_IMAGE);


	message(LOG_INFO,__FUNCTION__,QString("Inverting all colour planes"),Id);


	doLongProcessing(img,QtConcurrent::run(boost::bind(&PluginColourAdjust::do_invertAll,this,Id,img)));

	return Core::CODE_OK;
}

void PluginColourAdjust::do_invertAll(qulonglong Id,Image* img)
{
	do_invert(Id,img,img->planesList());
}

uint PluginColourAdjust::invert(qulonglong Id,QintList colourPlanes)
{
	bool busy;
	Image* img=getOrComplain(__FUNCTION__,"image",Id,busy);
	if(!img)
		return busy?(Core::CODE_IMAGE_BUSY):(Core::CODE_NO_IMAGE);

	if(colourPlanes.isEmpty())
		return CODE_NO_COLOUR_PLANE;

	foreach(int colourPlane,colourPlanes)
		if(!img->hasPlane(colourPlane))
			return CODE_NO_COLOUR_PLANE;


	QString colourPlanes_str;
	foreach(int colourPlane,colourPlanes)
		colourPlanes_str+=QString((colourPlanes_str.isEmpty())?"%1":", %1").arg(colourPlane);

	message(LOG_INFO,__FUNCTION__,QString("Inverting colour planes [%1]").arg(colourPlanes_str),Id);


	doLongProcessing(img,QtConcurrent::run(boost::bind(&PluginColourAdjust::do_invert,this,Id,img,colourPlanes)));

	return Core::CODE_OK;
}

void PluginColourAdjust::do_invert(qulonglong Id,Image* img,QintList colourPlanes)
{
	connect(this,SIGNAL(setPercent(double)),img,SLOT(setPercent(double)));

	double from=0.;
	double step=100./static_cast<double>(colourPlanes.size());
	foreach(int colourPlane,colourPlanes)
	{
		do_invertPlain(Id,img,colourPlane,from,step);
		from+=step;
	}

	disconnect(img,SLOT(setPercent(double)));
}

void PluginColourAdjust::do_invertPlain(qulonglong Id,Image* img,int colourPlane,double fromPercent,double stepPercent)
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




uint PluginColourAdjust::linearAll(qulonglong Id,ushort p0,ushort p1)
{
	bool busy;
	Image* img=getOrComplain(__FUNCTION__,"image",Id,busy);
	if(!img)
		return busy?(Core::CODE_IMAGE_BUSY):(Core::CODE_NO_IMAGE);

	message(LOG_INFO,__FUNCTION__,QString("Applying linear adjustment [%1,%2] to all colour planes").arg(p0).arg(p1),Id);


	doLongProcessing(img,QtConcurrent::run(boost::bind(&PluginColourAdjust::do_curveAll,this,Id,img,static_cast<uint>(CURVE_FUNCTION_LINEAR),p0,p1,QAdjustPoint())));

	return Core::CODE_OK;
}

uint PluginColourAdjust::linear(qulonglong Id,QintList colourPlanes,ushort p0,ushort p1)
{
	bool busy;
	Image* img=getOrComplain(__FUNCTION__,"image",Id,busy);
	if(!img)
		return busy?(Core::CODE_IMAGE_BUSY):(Core::CODE_NO_IMAGE);

	if(colourPlanes.isEmpty())
		return CODE_NO_COLOUR_PLANE;

	foreach(int colourPlane,colourPlanes)
		if(!img->hasPlane(colourPlane))
			return CODE_NO_COLOUR_PLANE;


	QString colourPlanes_str;
	foreach(int colourPlane,colourPlanes)
		colourPlanes_str+=QString((colourPlanes_str.isEmpty())?"%1":", %1").arg(colourPlane);

	message(LOG_INFO,__FUNCTION__,QString("Applying linear adjustment [%1,%2] to colour planes [%3]").arg(p0).arg(p1).arg(colourPlanes_str),Id);


	doLongProcessing(img,QtConcurrent::run(boost::bind(&PluginColourAdjust::do_curve,this,Id,img,colourPlanes,static_cast<uint>(CURVE_FUNCTION_LINEAR),p0,p1,QAdjustPoint())));

	return Core::CODE_OK;
}



namespace {

boost::shared_ptr<CurveFunction> initCurveFunction(uint function,ushort p0,ushort p1,QAdjustPoint points)
{
	boost::shared_ptr<CurveFunction> curveFunction;
	switch(function)
	{
		case CURVE_FUNCTION_LINEAR:
			curveFunction.reset(new Linear(p0,p1,points));
		break;
		case CURVE_FUNCTION_CUBIC_HERMITE_SPLINE:
			curveFunction.reset(new CubicHermiteSpline(p0,p1,points));
		break;
	}

	return curveFunction;
}

}

uint PluginColourAdjust::curveAll(qulonglong Id,uint function,ushort p0,ushort p1,QAdjustPoint points)
{
	if(functionList.constFind(function)==functionList.constEnd())
		return CODE_NO_SUCH_FUNCTION;

	bool busy;
	Image* img=getOrComplain(__FUNCTION__,"image",Id,busy);
	if(!img)
		return busy?(Core::CODE_IMAGE_BUSY):(Core::CODE_NO_IMAGE);

	if(!points.size())
		return CODE_NO_POINTS;

	message(LOG_INFO,__FUNCTION__,QString("Applying %1 curve").arg(functionList[function]),Id);


	doLongProcessing(img,QtConcurrent::run(boost::bind(&PluginColourAdjust::do_curveAll,this,Id,img,function,p0,p1,points)));

	return Core::CODE_OK;
}

void PluginColourAdjust::do_curveAll(qulonglong Id,Image* img,uint function,ushort p0,ushort p1,QAdjustPoint points)
{
	do_curve(Id,img,img->planesList(),function,p0,p1,points);
}

uint PluginColourAdjust::curve(qulonglong Id,QintList colourPlanes,uint function,ushort p0,ushort p1,QAdjustPoint points)
{
	if(functionList.constFind(function)==functionList.constEnd())
		return CODE_NO_SUCH_FUNCTION;

	bool busy;
	Image* img=getOrComplain(__FUNCTION__,"image",Id,busy);
	if(!img)
		return busy?(Core::CODE_IMAGE_BUSY):(Core::CODE_NO_IMAGE);

	if(colourPlanes.isEmpty())
		return CODE_NO_COLOUR_PLANE;

	foreach(int colourPlane,colourPlanes)
		if(!img->hasPlane(colourPlane))
			return CODE_NO_COLOUR_PLANE;


	if(!points.size())
		return CODE_NO_POINTS;


	QString colourPlanes_str;
	foreach(int colourPlane,colourPlanes)
		colourPlanes_str+=QString((colourPlanes_str.isEmpty())?"%1":", %1").arg(colourPlane);

	message(LOG_INFO,__FUNCTION__,QString("Applying %1 curve to colour planes [%2]").arg(functionList[function]).arg(colourPlanes_str),Id);


	doLongProcessing(img,QtConcurrent::run(boost::bind(&PluginColourAdjust::do_curve,this,Id,img,colourPlanes,function,p0,p1,points)));

	return Core::CODE_OK;
}

void PluginColourAdjust::do_curve(qulonglong Id,Image* img,QintList colourPlanes,uint function,ushort p0,ushort p1,QAdjustPoint points)
{
	connect(this,SIGNAL(setPercent(double)),img,SLOT(setPercent(double)));

	boost::shared_ptr<CurveFunction> curveFunction=initCurveFunction(function,p0,p1,points);

	double from=0.;
	double step=100./static_cast<double>(colourPlanes.size());
	foreach(int colourPlane,colourPlanes)
	{
		do_curvePlain(Id,img,colourPlane,function,curveFunction.get(),from,step);
		from+=step;
	}

	disconnect(img,SLOT(setPercent(double)));
}

void PluginColourAdjust::do_curvePlain(qulonglong Id,Image* img,int colourPlane,uint function,CurveFunction* curveFunction,double fromPercent,double stepPercent)
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
			data[xyo]=curveFunction->get(data[xyo]);
		}
	}

	message(LOG_INFO,"curve",QString("%1 curve applied to colour plane [%2]").arg(functionList[function]).arg(colourPlane),Id);
}




QString PluginColourAdjust::errorCodeToString(uint errorCode) const
{
	switch(errorCode)
	{
		#define CASE(ERR,STR) case CODE_##ERR: return STR ;
		CASE(NO_COLOUR_PLANE, "No colour plane")
		CASE(NO_SUCH_FUNCTION,"No such function")
		CASE(NO_POINTS,       "No adjust points")
		#undef CASE
	}
	return m_core->errorCodeToString(errorCode);
}
