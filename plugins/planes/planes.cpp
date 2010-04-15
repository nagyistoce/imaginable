/*************
**
** Project:      Imaginable
** File info:    $Id: clone.cpp 16 2010-04-13 10:59:29Z Kuzma.Shapran@gmail.com $
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


#include "planes.hpp"
#include "dbus_plugin_planes_adaptor.h"



Q_EXPORT_PLUGIN2(clone,PluginPlanes)


PluginPlanes::PluginPlanes(void)
	: QObject(NULL)
	, PluginInterface()
{
}

bool PluginPlanes::init(Root* root)
{
	m_root=root;

	if(!QDBusConnection::sessionBus().registerObject(name(),new PlanesAdaptor(this),QDBusConnection::ExportNonScriptableContents))
	{
		complain(LOG_ALERT,__FUNCTION__,"Cannot register D-Bus object interface");
		return false;
	}

	return true;
}

namespace {

void rgb2luma_Pixel(
	Image::Pixel r,
	Image::Pixel g,
	Image::Pixel b,
	Image::Pixel& l )
{
	l=static_cast<Image::Pixel>( (
		0x4c8b*static_cast<unsigned>(r) +
		0x9646*static_cast<unsigned>(g) +
		0x1d2f*static_cast<unsigned>(b) )/0x10000 );
}

void rgb2hsv_Pixel(
	Image::Pixel r,
	Image::Pixel g,
	Image::Pixel b,
	Image::Pixel& H,
	Image::Pixel& h,
	Image::Pixel& s,
	Image::Pixel& v )
{
	H=0;
	h=0;
	s=0;
	Image::Pixel rgbmin=std::min(r,std::min(g,b));
	/**/         v     =std::max(r,std::max(g,b));
	Image::Pixel rgbdelta=v-rgbmin;

	if(v)
		s=static_cast<Image::Pixel>(0xffffu*static_cast<unsigned>(rgbdelta)/v);

	if(s)
	{
		if(r==v)
		{
			if(g>=b)
			{
				H=0;
				h=static_cast<Image::Pixel>(0xffffu*(        static_cast<unsigned>(g)-static_cast<unsigned>(b))/rgbdelta);
			}
			else
			{
				H=5;
				h=static_cast<Image::Pixel>(0xffffu*(0xffffu+static_cast<unsigned>(g)-static_cast<unsigned>(b))/rgbdelta);
			}
		}
		else
		if(g==v)
		{
			if(b>=r)
			{
				H=2;
				h=static_cast<Image::Pixel>(0xffffu*(        static_cast<unsigned>(b)-static_cast<unsigned>(r))/rgbdelta);
			}
			else
			{
				H=1;
				h=static_cast<Image::Pixel>(0xffffu*(0xffffu+static_cast<unsigned>(b)-static_cast<unsigned>(r))/rgbdelta);
			}
		}
		else
		if(b==v)
		{
			if(r>=g)
			{
				H=4;
				h=static_cast<Image::Pixel>(0xffffu*(        static_cast<unsigned>(r)-static_cast<unsigned>(g))/rgbdelta);
			}
			else
			{
				H=3;
				h=static_cast<Image::Pixel>(0xffffu*(0xffffu+static_cast<unsigned>(r)-static_cast<unsigned>(g))/rgbdelta);
			}
		}
	}
}

void hsv2rgb_Pixel(
	Image::Pixel H,
	Image::Pixel h,
	Image::Pixel s,
	Image::Pixel v,
	Image::Pixel& r,
	Image::Pixel& g,
	Image::Pixel& b )
{
	if(!s)
		r=g=b=v;
	else
	{
		Image::Pixel p=static_cast<Image::Pixel>((static_cast<unsigned>(v)*(0xffffu- static_cast<unsigned>(s)                                            ))/0xffffu);
		Image::Pixel q=static_cast<Image::Pixel>((static_cast<unsigned>(v)*(0xffffu-(static_cast<unsigned>(s)*         static_cast<unsigned>(h) )/0xffffu))/0xffffu);
		Image::Pixel t=static_cast<Image::Pixel>((static_cast<unsigned>(v)*(0xffffu-(static_cast<unsigned>(s)*(0xffffu-static_cast<unsigned>(h)))/0xffffu))/0xffffu);

		switch(H%6)
		{
			case 0: r=v; g=t; b=p; break;
			case 1: r=q; g=v; b=p; break;
			case 2: r=p; g=v; b=t; break;
			case 3: r=p; g=q; b=v; break;
			case 4: r=t; g=p; b=v; break;
			case 5: r=v; g=p; b=q; break;
		}
	}
}

void hsv2hsl_Pixel(
	Image::Pixel& s,
	Image::Pixel  v,
	Image::Pixel& l )
{
	l=static_cast<Image::Pixel>((static_cast<unsigned>(v)*(0xffffu-static_cast<unsigned>(s)/2))/0xffffu);
	if( (l>=0x8000u) && (l< 0xffffu) )
		s=static_cast<Image::Pixel>(((static_cast<unsigned>(s)*0xffffu)/(2*(0xffffu-static_cast<unsigned>(l)))));
}

void hsl2hsv_Pixel(
	Image::Pixel& s,
	Image::Pixel l,
	Image::Pixel& v )
{
	if(l>=0x8000u)
		s=static_cast<Image::Pixel>((static_cast<unsigned>(s)*2*(0xffffu-static_cast<unsigned>(l)))/0xffffu);
	v=static_cast<Image::Pixel>(static_cast<unsigned>(l)*0xffffu/(0xffffu-static_cast<unsigned>(s)/2));
}

///TODO: Make direct conversion
void rgb2hsl_Pixel(
	Image::Pixel r,
	Image::Pixel g,
	Image::Pixel b,
	Image::Pixel& H,
	Image::Pixel& h,
	Image::Pixel& s,
	Image::Pixel& l )
{
	Image::Pixel v;
	rgb2hsv_Pixel(r,g,b,H,h,s,v);
	hsv2hsl_Pixel(s,v,l);

}

///TODO: Make direct conversion
void hsl2rgb_Pixel(
	Image::Pixel H,
	Image::Pixel h,
	Image::Pixel s,
	Image::Pixel l,
	Image::Pixel& r,
	Image::Pixel& g,
	Image::Pixel& b )
{
	Image::Pixel v;
	hsl2hsv_Pixel(s,l,v);
	hsv2rgb_Pixel(H,h,s,v,r,g,b);
}

void uncompressHue_Pixel(
	Image::Pixel& H,
	Image::Pixel& h )
{
	unsigned h6=static_cast<unsigned>(h)*6;
	H=static_cast<Image::Pixel>(h6/0x10000u);
	h=static_cast<Image::Pixel>(h6%0x10000u);
}

void compressHue_Pixel(
	Image::Pixel H,
	Image::Pixel& h )
{
	h=static_cast<Image::Pixel>(((static_cast<unsigned>(H)%6)*0x10000u+static_cast<unsigned>(h))/6);
}

}

uint PluginPlanes::copy(qulonglong fromId,qulonglong toId,int fromPlane,int toPlane)
{
	if( (fromId   ==toId   )
	&&  (fromPlane==toPlane) )
	{
		complain(LOG_WARNING,__FUNCTION__,QString("Source and destination planes [%1] or images must differ").arg(fromPlane),fromId);
		return CODE_PLANES_DONT_DIFFER;
	}

	bool busy;
	Image* src=getOrComplain(__FUNCTION__,"source image",fromId,busy);
	if(!src)
		return busy?(Root::CODE_SRC_IMAGE_BUSY):(Root::CODE_NO_SRC_IMAGE);

	Image* dst;
	if(fromId==toId)
		dst=src;
	else
	{
		dst=getOrComplain(__FUNCTION__,"destination image",toId,busy);
		if(!dst)
			return busy?(Root::CODE_DST_IMAGE_BUSY):(Root::CODE_NO_DST_IMAGE);
	}

	if( (fromId!=toId)
	&&  (src->size()!=dst->size()) )
		return CODE_SIZES_DIFFER;

	if( (!src->hasPlane(fromPlane)) )
		return CODE_NO_SOURCE_PLANE;

	if( (dst->hasPlane(toPlane)) )
		return CODE_DESTINATION_PLANE_EXISTS;


	dst->addPlane(toPlane);
	Image::Pixel* src_plane=src->plane(fromPlane);
	Image::Pixel* dst_plane=dst->plane(toPlane);
	memcpy(dst_plane,src_plane,src->area()*sizeof(Image::Pixel));
	message(LOG_INFO,__FUNCTION__,QString("Plane [%1] from image [%2] copied to plane [%3]").arg(fromPlane).arg(fromId).arg(toPlane),toId);


	return Root::CODE_OK;
}

uint PluginPlanes::rgb2luma(qulonglong Id)
{
	bool busy;
	Image* img=getOrComplain(__FUNCTION__,"image",Id,busy);
	if(!img)
		return busy?(Root::CODE_IMAGE_BUSY):(Root::CODE_NO_IMAGE);

	if( (!img->hasPlane(Image::PLANE_RED))
	||  (!img->hasPlane(Image::PLANE_GREEN))
	||  (!img->hasPlane(Image::PLANE_BLUE)) )
		return CODE_NO_SOURCE_PLANE;

	if(img->hasPlane(Image::PLANE_LIGHTNESS))
		return CODE_DESTINATION_PLANE_EXISTS;


	const int& width =img->width();
	const int& height=img->height();

	img->addPlane(Image::PLANE_LIGHTNESS);

	const Image::Pixel* red       =img->plane(Image::PLANE_RED);
	const Image::Pixel* green     =img->plane(Image::PLANE_GREEN);
	const Image::Pixel* blue      =img->plane(Image::PLANE_BLUE);
	/* */ Image::Pixel* lightness =img->plane(Image::PLANE_LIGHTNESS);

	for(int y=0;y<height;++y)
	{
		int yo=y*width;
		for(int x=0;x<width;++x)
		{
			int xyo=yo+x;
			rgb2luma_Pixel(red[xyo],green[xyo],blue[xyo],lightness[xyo]);
		}
	}

	img->removePlane(Image::PLANE_RED);
	img->removePlane(Image::PLANE_GREEN);
	img->removePlane(Image::PLANE_BLUE);


	message(LOG_INFO,__FUNCTION__,"Colour planes converted: RGB -> luma",Id);

	return Root::CODE_OK;
}

uint PluginPlanes::rgb2hsv(qulonglong Id)
{
	bool busy;
	Image* img=getOrComplain(__FUNCTION__,"image",Id,busy);
	if(!img)
		return busy?(Root::CODE_IMAGE_BUSY):(Root::CODE_NO_IMAGE);

	if( (!img->hasPlane(Image::PLANE_RED))
	||  (!img->hasPlane(Image::PLANE_GREEN))
	||  (!img->hasPlane(Image::PLANE_BLUE)) )
		return CODE_NO_SOURCE_PLANE;

	if( (img->hasPlane(Image::PLANE_HUE_SECTOR))
	||  (img->hasPlane(Image::PLANE_HUE))
	||  (img->hasPlane(Image::PLANE_SATURATION))
	||  (img->hasPlane(Image::PLANE_HSV_VALUE)) )
		return CODE_DESTINATION_PLANE_EXISTS;


	const int& width =img->width();
	const int& height=img->height();

	img->addPlane(Image::PLANE_HUE_SECTOR);
	img->addPlane(Image::PLANE_HUE);
	img->addPlane(Image::PLANE_SATURATION);
	img->addPlane(Image::PLANE_HSV_VALUE);

	const Image::Pixel* red       =img->plane(Image::PLANE_RED);
	const Image::Pixel* green     =img->plane(Image::PLANE_GREEN);
	const Image::Pixel* blue      =img->plane(Image::PLANE_BLUE);
	/* */ Image::Pixel* hue_sector=img->plane(Image::PLANE_HUE_SECTOR);
	/* */ Image::Pixel* hue       =img->plane(Image::PLANE_HUE);
	/* */ Image::Pixel* saturation=img->plane(Image::PLANE_SATURATION);
	/* */ Image::Pixel* value     =img->plane(Image::PLANE_HSV_VALUE);

	for(int y=0;y<height;++y)
	{
		int yo=y*width;
		for(int x=0;x<width;++x)
		{
			int xyo=yo+x;
			rgb2hsv_Pixel(red[xyo],green[xyo],blue[xyo],hue_sector[xyo],hue[xyo],saturation[xyo],value[xyo]);
		}
	}

	img->removePlane(Image::PLANE_RED);
	img->removePlane(Image::PLANE_GREEN);
	img->removePlane(Image::PLANE_BLUE);


	message(LOG_INFO,__FUNCTION__,"Colour planes converted: RGB -> HSV",Id);

	return Root::CODE_OK;
}

uint PluginPlanes::hsv2rgb(qulonglong Id)
{
	bool busy;
	Image* img=getOrComplain(__FUNCTION__,"image",Id,busy);
	if(!img)
		return busy?(Root::CODE_IMAGE_BUSY):(Root::CODE_NO_IMAGE);

	if( (!img->hasPlane(Image::PLANE_HUE_SECTOR))
	||  (!img->hasPlane(Image::PLANE_HUE))
	||  (!img->hasPlane(Image::PLANE_SATURATION))
	||  (!img->hasPlane(Image::PLANE_HSV_VALUE)) )
		return CODE_NO_SOURCE_PLANE;

	if( (img->hasPlane(Image::PLANE_RED))
	||  (img->hasPlane(Image::PLANE_GREEN))
	||  (img->hasPlane(Image::PLANE_BLUE)) )
		return CODE_DESTINATION_PLANE_EXISTS;


	const int& width =img->width();
	const int& height=img->height();

	img->addPlane(Image::PLANE_RED);
	img->addPlane(Image::PLANE_GREEN);
	img->addPlane(Image::PLANE_BLUE);

	const Image::Pixel* hue_sector=img->plane(Image::PLANE_HUE_SECTOR);
	const Image::Pixel* hue       =img->plane(Image::PLANE_HUE);
	const Image::Pixel* saturation=img->plane(Image::PLANE_SATURATION);
	const Image::Pixel* value     =img->plane(Image::PLANE_HSV_VALUE);
	/* */ Image::Pixel* red       =img->plane(Image::PLANE_RED);
	/* */ Image::Pixel* green     =img->plane(Image::PLANE_GREEN);
	/* */ Image::Pixel* blue      =img->plane(Image::PLANE_BLUE);

	for(int y=0;y<height;++y)
	{
		int yo=y*width;
		for(int x=0;x<width;++x)
		{
			int xyo=yo+x;
			hsv2rgb_Pixel(hue_sector[xyo],hue[xyo],saturation[xyo],value[xyo],red[xyo],green[xyo],blue[xyo]);
		}
	}

	img->removePlane(Image::PLANE_HUE_SECTOR);
	img->removePlane(Image::PLANE_HUE);
	img->removePlane(Image::PLANE_SATURATION);
	img->removePlane(Image::PLANE_HSV_VALUE);


	message(LOG_INFO,__FUNCTION__,"Colour planes converted: HSV -> RGB",Id);

	return Root::CODE_OK;
}

uint PluginPlanes::rgb2hsl(qulonglong Id)
{
	bool busy;
	Image* img=getOrComplain(__FUNCTION__,"image",Id,busy);
	if(!img)
		return busy?(Root::CODE_IMAGE_BUSY):(Root::CODE_NO_IMAGE);

	if( (!img->hasPlane(Image::PLANE_RED))
	||  (!img->hasPlane(Image::PLANE_GREEN))
	||  (!img->hasPlane(Image::PLANE_BLUE)) )
		return CODE_NO_SOURCE_PLANE;

	if( (img->hasPlane(Image::PLANE_HUE_SECTOR))
	||  (img->hasPlane(Image::PLANE_HUE))
	||  (img->hasPlane(Image::PLANE_SATURATION))
	||  (img->hasPlane(Image::PLANE_LIGHTNESS)) )
		return CODE_DESTINATION_PLANE_EXISTS;


	const int& width =img->width();
	const int& height=img->height();

	img->addPlane(Image::PLANE_HUE_SECTOR);
	img->addPlane(Image::PLANE_HUE);
	img->addPlane(Image::PLANE_SATURATION);
	img->addPlane(Image::PLANE_LIGHTNESS);

	const Image::Pixel* red       =img->plane(Image::PLANE_RED);
	const Image::Pixel* green     =img->plane(Image::PLANE_GREEN);
	const Image::Pixel* blue      =img->plane(Image::PLANE_BLUE);
	/* */ Image::Pixel* hue_sector=img->plane(Image::PLANE_HUE_SECTOR);
	/* */ Image::Pixel* hue       =img->plane(Image::PLANE_HUE);
	/* */ Image::Pixel* saturation=img->plane(Image::PLANE_SATURATION);
	/* */ Image::Pixel* lightness =img->plane(Image::PLANE_LIGHTNESS);

	for(int y=0;y<height;++y)
	{
		int yo=y*width;
		for(int x=0;x<width;++x)
		{
			int xyo=yo+x;
			rgb2hsl_Pixel(red[xyo],green[xyo],blue[xyo],hue_sector[xyo],hue[xyo],saturation[xyo],lightness[xyo]);
		}
	}

	img->removePlane(Image::PLANE_RED);
	img->removePlane(Image::PLANE_GREEN);
	img->removePlane(Image::PLANE_BLUE);


	message(LOG_INFO,__FUNCTION__,"Colour planes converted: RGB -> HSL",Id);

	return Root::CODE_OK;
}

uint PluginPlanes::hsl2rgb(qulonglong Id)
{
	bool busy;
	Image* img=getOrComplain(__FUNCTION__,"image",Id,busy);
	if(!img)
		return busy?(Root::CODE_IMAGE_BUSY):(Root::CODE_NO_IMAGE);

	if( (!img->hasPlane(Image::PLANE_HUE_SECTOR))
	||  (!img->hasPlane(Image::PLANE_HUE))
	||  (!img->hasPlane(Image::PLANE_SATURATION))
	||  (!img->hasPlane(Image::PLANE_LIGHTNESS)) )
		return CODE_NO_SOURCE_PLANE;

	if( (img->hasPlane(Image::PLANE_RED))
	||  (img->hasPlane(Image::PLANE_GREEN))
	||  (img->hasPlane(Image::PLANE_BLUE)) )
		return CODE_DESTINATION_PLANE_EXISTS;


	const int& width =img->width();
	const int& height=img->height();

	img->addPlane(Image::PLANE_RED);
	img->addPlane(Image::PLANE_GREEN);
	img->addPlane(Image::PLANE_BLUE);

	const Image::Pixel* hue_sector=img->plane(Image::PLANE_HUE_SECTOR);
	const Image::Pixel* hue       =img->plane(Image::PLANE_HUE);
	const Image::Pixel* saturation=img->plane(Image::PLANE_SATURATION);
	const Image::Pixel* lightness =img->plane(Image::PLANE_LIGHTNESS);
	/* */ Image::Pixel* red       =img->plane(Image::PLANE_RED);
	/* */ Image::Pixel* green     =img->plane(Image::PLANE_GREEN);
	/* */ Image::Pixel* blue      =img->plane(Image::PLANE_BLUE);

	for(int y=0;y<height;++y)
	{
		int yo=y*width;
		for(int x=0;x<width;++x)
		{
			int xyo=yo+x;
			hsl2rgb_Pixel(hue_sector[xyo],hue[xyo],saturation[xyo],lightness[xyo],red[xyo],green[xyo],blue[xyo]);
		}
	}

	img->removePlane(Image::PLANE_HUE_SECTOR);
	img->removePlane(Image::PLANE_HUE);
	img->removePlane(Image::PLANE_SATURATION);
	img->removePlane(Image::PLANE_LIGHTNESS);


	message(LOG_INFO,__FUNCTION__,"Colour planes converted: HSL -> RGB",Id);

	return Root::CODE_OK;
}

uint PluginPlanes::hsv2hsl(qulonglong Id)
{
	bool busy;
	Image* img=getOrComplain(__FUNCTION__,"image",Id,busy);
	if(!img)
		return busy?(Root::CODE_IMAGE_BUSY):(Root::CODE_NO_IMAGE);

	if( (!img->hasPlane(Image::PLANE_HUE_SECTOR))
	||  (!img->hasPlane(Image::PLANE_HUE))
	||  (!img->hasPlane(Image::PLANE_SATURATION))
	||  (!img->hasPlane(Image::PLANE_HSV_VALUE)) )
		return CODE_NO_SOURCE_PLANE;

	if( (img->hasPlane(Image::PLANE_LIGHTNESS)) )
		return CODE_DESTINATION_PLANE_EXISTS;


	const int& width =img->width();
	const int& height=img->height();

	img->addPlane(Image::PLANE_LIGHTNESS);

	/* */ Image::Pixel* saturation=img->plane(Image::PLANE_SATURATION);
	const Image::Pixel* value     =img->plane(Image::PLANE_HSV_VALUE);
	/* */ Image::Pixel* lightness =img->plane(Image::PLANE_LIGHTNESS);

	for(int y=0;y<height;++y)
	{
		int yo=y*width;
		for(int x=0;x<width;++x)
		{
			int xyo=yo+x;
			hsv2hsl_Pixel(saturation[xyo],value[xyo],lightness[xyo]);
		}
	}

	img->removePlane(Image::PLANE_HSV_VALUE);


	message(LOG_INFO,__FUNCTION__,"Colour planes converted: HSV -> HSL",Id);

	return Root::CODE_OK;
}

uint PluginPlanes::hsl2hsv(qulonglong Id)
{
	bool busy;
	Image* img=getOrComplain(__FUNCTION__,"image",Id,busy);
	if(!img)
		return busy?(Root::CODE_IMAGE_BUSY):(Root::CODE_NO_IMAGE);

	if( (!img->hasPlane(Image::PLANE_HUE_SECTOR))
	||  (!img->hasPlane(Image::PLANE_HUE))
	||  (!img->hasPlane(Image::PLANE_SATURATION))
	||  (!img->hasPlane(Image::PLANE_LIGHTNESS)) )
		return CODE_NO_SOURCE_PLANE;

	if( (img->hasPlane(Image::PLANE_HSV_VALUE)) )
		return CODE_DESTINATION_PLANE_EXISTS;


	const int& width =img->width();
	const int& height=img->height();

	img->addPlane(Image::PLANE_HSV_VALUE);

	/* */ Image::Pixel* saturation=img->plane(Image::PLANE_SATURATION);
	const Image::Pixel* lightness =img->plane(Image::PLANE_LIGHTNESS);
	/* */ Image::Pixel* value     =img->plane(Image::PLANE_HSV_VALUE);

	for(int y=0;y<height;++y)
	{
		int yo=y*width;
		for(int x=0;x<width;++x)
		{
			int xyo=yo+x;
			hsl2hsv_Pixel(saturation[xyo],lightness[xyo],value[xyo]);
		}
	}

	img->removePlane(Image::PLANE_LIGHTNESS);


	message(LOG_INFO,__FUNCTION__,"Colour planes converted: HSL -> HSV",Id);

	return Root::CODE_OK;
}

uint PluginPlanes::uncompressHue(qulonglong Id)
{
	bool busy;
	Image* img=getOrComplain(__FUNCTION__,"image",Id,busy);
	if(!img)
		return busy?(Root::CODE_IMAGE_BUSY):(Root::CODE_NO_IMAGE);

	if(!img->hasPlane(Image::PLANE_HUE))
		return CODE_NO_SOURCE_PLANE;

	if(img->hasPlane(Image::PLANE_HUE_SECTOR))
		return CODE_DESTINATION_PLANE_EXISTS;


	const int& width =img->width();
	const int& height=img->height();

	img->addPlane(Image::PLANE_HUE_SECTOR);

	/* */ Image::Pixel* hue_sector=img->plane(Image::PLANE_HUE_SECTOR);
	/* */ Image::Pixel* hue       =img->plane(Image::PLANE_HUE);

	for(int y=0;y<height;++y)
	{
		int yo=y*width;
		for(int x=0;x<width;++x)
		{
			int xyo=yo+x;
			uncompressHue_Pixel(hue_sector[xyo],hue[xyo]);
		}
	}


	message(LOG_INFO,__FUNCTION__,"Colour planes converted: HSV -> HSL",Id);

	return Root::CODE_OK;
}

uint PluginPlanes::compressHue(qulonglong Id)
{
	bool busy;
	Image* img=getOrComplain(__FUNCTION__,"image",Id,busy);
	if(!img)
		return busy?(Root::CODE_IMAGE_BUSY):(Root::CODE_NO_IMAGE);

	if( (!img->hasPlane(Image::PLANE_HUE_SECTOR))
	||  (!img->hasPlane(Image::PLANE_HUE)) )
		return CODE_NO_SOURCE_PLANE;


	const int& width =img->width();
	const int& height=img->height();

	const Image::Pixel* hue_sector=img->plane(Image::PLANE_HUE_SECTOR);
	/* */ Image::Pixel* hue       =img->plane(Image::PLANE_HUE);

	for(int y=0;y<height;++y)
	{
		int yo=y*width;
		for(int x=0;x<width;++x)
		{
			int xyo=yo+x;
			compressHue_Pixel(hue_sector[xyo],hue[xyo]);
		}
	}

	img->removePlane(Image::PLANE_HUE_SECTOR);


	message(LOG_INFO,__FUNCTION__,"Colour planes converted: HSL -> HSV",Id);

	return Root::CODE_OK;
}

QString PluginPlanes::errorCodeToString(uint errorCode) const
{
	switch(errorCode)
	{
		#define CASE(ERR,STR) case CODE_##ERR: return STR ;
		CASE(PLANES_DONT_DIFFER      ,"Planes do not differ")
		CASE(NO_SOURCE_PLANE         ,"No source colour plane")
		CASE(DESTINATION_PLANE_EXISTS,"Detination colour plane already exists")
		#undef CASE
	}
	return m_root->errorCodeToString(errorCode);
}
