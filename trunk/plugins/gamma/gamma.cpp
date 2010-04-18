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


#include "gamma.hpp"
#include "dbus_plugin_gamma_adaptor.h"

#include <boost/bind.hpp>


Q_EXPORT_PLUGIN2(gamma,PluginGamma)


PluginGamma::PluginGamma(void)
	: QObject(NULL)
	, PluginInterface()
{
}

bool PluginGamma::init(Root* root)
{
	m_root=root;

	if(!QDBusConnection::sessionBus().registerObject(name(),new GammaAdaptor(this),QDBusConnection::ExportNonScriptableContents))
	{
		complain(LOG_ALERT,__FUNCTION__,"Cannot register D-Bus object interface");
		return false;
	}

	return true;
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

uint PluginGamma::gammaAll(qulonglong Id,double value)
{
	bool busy;
	Image* img=getOrComplain(__FUNCTION__,"image",Id,busy);
	if(!img)
		return busy?(Root::CODE_IMAGE_BUSY):(Root::CODE_NO_IMAGE);


	message(LOG_INFO,__FUNCTION__,QString("Applying gamma [%1]").arg(value),Id);

	doLongProcessing(img,QtConcurrent::run(boost::bind(&PluginGamma::do_gammaAll,this,Id,img,value)));

	return Root::CODE_OK;
}

void PluginGamma::do_gammaAll(qulonglong Id,Image* img,double value)
{
	connect(this,SIGNAL(setPercent(double)),img,SLOT(setPercent(double)));

	double from=0.;
	double step=100./static_cast<double>(img->planes().size());
	foreach(int plane,img->planes())
	{
		do_gamma(Id,img,plane,value,from,from+step);
		from+=step;
	}

	disconnect(img,SLOT(setPercent(double)));

	message(LOG_INFO,"gammaAll",QString("Gamma [%1] applied").arg(value),Id);
}

uint PluginGamma::gamma(qulonglong Id,int colourPlane,double value)
{
	bool busy;
	Image* img=getOrComplain(__FUNCTION__,"image",Id,busy);
	if(!img)
		return busy?(Root::CODE_IMAGE_BUSY):(Root::CODE_NO_IMAGE);

	if(!img->hasPlane(colourPlane))
		return CODE_NO_SOURCE_PLANE;


	message(LOG_INFO,__FUNCTION__,QString("Applying gamma [%1] to colour plane [%2]").arg(value).arg(colourPlane),Id);


	doLongProcessing(img,QtConcurrent::run(boost::bind(&PluginGamma::do_gamma,this,Id,img,colourPlane,value,0.,100.)));

	return Root::CODE_OK;
}

void PluginGamma::do_gamma(qulonglong Id,Image* img,int colourPlane,double value,double fromPercent,double stepPercent)
{
	connect(this,SIGNAL(setPercent(double)),img,SLOT(setPercent(double)));

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


	disconnect(img,SLOT(setPercent(double)));

	message(LOG_INFO,"gamma",QString("Gamma [%1] applied to colour plane [%2]").arg(value).arg(colourPlane),Id);
}

QString PluginGamma::errorCodeToString(uint errorCode) const
{
	switch(errorCode)
	{
		#define CASE(ERR,STR) case CODE_##ERR: return STR ;
		CASE(NO_SOURCE_PLANE ,"No source colour plane")
		#undef CASE
	}
	return m_root->errorCodeToString(errorCode);
}
