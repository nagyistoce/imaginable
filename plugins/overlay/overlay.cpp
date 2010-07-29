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


#include "overlay.hpp"

#include "dbus_plugin_overlay_adaptor.h"


Q_EXPORT_PLUGIN2(overlay,PluginOverlay)


PluginOverlay::PluginOverlay(void)
	: QObject(NULL)
	, PluginInterface()
{
	new OverlayAdaptor(this);
}

uint PluginOverlay::difference(qulonglong Id,int srcColourPlane1,int srcColourPlane2,int dstColourPlane)
{
	bool busy;
	Image *img=getOrComplain(__FUNCTION__,"image",Id,busy);
	if (!img)
		return busy ? (Core::CODE_IMAGE_BUSY) : (Core::CODE_NO_IMAGE);

	if (!img->hasPlane(srcColourPlane1))
		return CODE_NO_SRC_COLOUR_PLANE_1;

	if (!img->hasPlane(srcColourPlane2))
		return CODE_NO_SRC_COLOUR_PLANE_2;

	if (img->hasPlane(dstColourPlane))
		return CODE_DST_COLOUR_PLANE_EXISTS;


	message(LOG_INFO,__FUNCTION__,QString("Overlaying colour planes [%1] and [%2] to [%3] with 'difference' function").arg(srcColourPlane1).arg(srcColourPlane2).arg(dstColourPlane),Id);


	doLongProcessing(img,QtConcurrent::run(boost::bind(&PluginOverlay::do_difference,this,Id,img,srcColourPlane1,srcColourPlane2,dstColourPlane)));

	return Core::CODE_OK;
}

void PluginOverlay::do_difference(qulonglong Id,Image *img,int srcColourPlane1,int srcColourPlane2,int dstColourPlane)
{
	connect(this,SIGNAL(setPercent(double)),img,SLOT(setPercent(double)));

	const int &width =img->width();
	const int &height=img->height();

	img->addPlane(dstColourPlane);

	const Image::Pixel *src1=img->plane(srcColourPlane1);
	const Image::Pixel *src2=img->plane(srcColourPlane2);
	/* */ Image::Pixel *dst =img->plane(dstColourPlane);

	for (int y=0; y<height; ++y)
	{
		emit setPercent(static_cast<double>(y)*100./static_cast<double>(height));

		int yo=y*width;
		for (int x=0; x<width; ++x)
		{
			int yxo=yo+x;
			dst[yxo]=static_cast<Image::Pixel>(abs(static_cast<int>(src1[yxo])-static_cast<int>(src2[yxo])));
		}
	}

	message(LOG_INFO,"difference",QString("Colour planes [%1] and [%2] overlayed to [%3] with 'difference' function").arg(srcColourPlane1).arg(srcColourPlane2).arg(dstColourPlane),Id);

	disconnect(img,SLOT(setPercent(double)));
}

uint PluginOverlay::alphaBlend(qulonglong Id,int srcColourPlane1,int srcColourPlane2,int srcColourPlane3,int dstColourPlane)
{
	bool busy;
	Image *img=getOrComplain(__FUNCTION__,"image",Id,busy);
	if (!img)
		return busy ? (Core::CODE_IMAGE_BUSY) : (Core::CODE_NO_IMAGE);

	if (!img->hasPlane(srcColourPlane1))
		return CODE_NO_SRC_COLOUR_PLANE_1;

	if (!img->hasPlane(srcColourPlane2))
		return CODE_NO_SRC_COLOUR_PLANE_2;

	if (!img->hasPlane(srcColourPlane3))
		return CODE_NO_SRC_COLOUR_PLANE_3;

	if (img->hasPlane(dstColourPlane))
		return CODE_DST_COLOUR_PLANE_EXISTS;


	message(LOG_INFO,__FUNCTION__,QString("Overlaying colour planes [%1] and [%2] to [%4] usign [%3] as alpha with 'alphaBlend' function").arg(srcColourPlane1).arg(srcColourPlane2).arg(srcColourPlane3).arg(dstColourPlane),Id);


	doLongProcessing(img,QtConcurrent::run(boost::bind(&PluginOverlay::do_alphaBlend,this,Id,img,srcColourPlane1,srcColourPlane2,srcColourPlane3,dstColourPlane)));

	return Core::CODE_OK;
}

void PluginOverlay::do_alphaBlend(qulonglong Id,Image *img,int srcColourPlane1,int srcColourPlane2,int srcColourPlane3,int dstColourPlane)
{
	connect(this,SIGNAL(setPercent(double)),img,SLOT(setPercent(double)));

	const int &width =img->width();
	const int &height=img->height();

	img->addPlane(dstColourPlane);

	const Image::Pixel *src1=img->plane(srcColourPlane1);
	const Image::Pixel *src2=img->plane(srcColourPlane2);
	const Image::Pixel *src3=img->plane(srcColourPlane3);
	/* */ Image::Pixel *dst =img->plane(dstColourPlane);

	for (int y=0; y<height; ++y)
	{
		emit setPercent(static_cast<double>(y)*100./static_cast<double>(height));

		int yo=y*width;
		for (int x=0; x<width; ++x)
		{
			int yxo=yo+x;
			unsigned alpha=static_cast<unsigned>(src3[yxo]);
			dst[yxo]=static_cast<Image::Pixel>( (
				static_cast<unsigned>(src1[yxo])*        alpha  +
				static_cast<unsigned>(src2[yxo])*(0xffff-alpha) )/0xffff );
		}
	}

	message(LOG_INFO,"difference",QString("Colour planes [%1] and [%2] overlayed to [%4] usign [%3] as alpha with 'alphaBlend' function").arg(srcColourPlane1).arg(srcColourPlane2).arg(srcColourPlane3).arg(dstColourPlane),Id);

	disconnect(img,SLOT(setPercent(double)));
}

QString PluginOverlay::errorCodeToString(uint errorCode) const
{
	switch (errorCode)
	{
		#define CASE(ERR,STR) case CODE_##ERR: return STR ;
		CASE(NO_SRC_COLOUR_PLANE_1  ,"No source colour plane #1")
		CASE(NO_SRC_COLOUR_PLANE_2  ,"No source colour plane #2")
		CASE(NO_SRC_COLOUR_PLANE_3  ,"No source colour plane #3")
		CASE(DST_COLOUR_PLANE_EXISTS,"Destination colour plane already exists")
		#undef CASE
	}
	return m_core->errorCodeToString(errorCode);
}
