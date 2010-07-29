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


#include <boost/scoped_array.hpp>

#include "resize.hpp"

#include "dbus_plugin_resize_adaptor.h"


Q_EXPORT_PLUGIN2(resize,PluginResize)


PluginResize::PluginResize(void)
	: QObject(NULL)
	, PluginInterface()
{
	qDBusRegisterMetaType<QfullPixel>();

	new ResizeAdaptor(this);
}

qulonglong PluginResize::resizeCommon(const char *function,qulonglong Id,newSizePolicy_t newSizePolicy,newColourPolicy_t newColourPolicy,rect newSize,QfullPixel newColour,quint16 value)
{
	if (newSizePolicy != NEWSIZE_AUTOCROP)
	{
		if (newSize.width() <= 0)
		{
			complain(LOG_ERR,__FUNCTION__,"Incorrect width",Id);
			m_lastErrorCodes[Id]=CODE_INCORrect_WIDTH;
			return 0ULL;
		}

		if (newSize.height() <= 0)
		{
			complain(LOG_ERR,__FUNCTION__,"Incorrect height",Id);
			m_lastErrorCodes[Id]=CODE_INCORrect_HEIGHT;
			return 0ULL;
		}
	}


	bool busy;
	Image *src=getOrComplain(function,"source image",Id,busy);
	if (!src)
	{
		complain(LOG_ERR,__FUNCTION__,"Cannot get source image",Id);
		m_lastErrorCodes[Id]=busy?(Core::CODE_SRC_IMAGE_BUSY):(Core::CODE_NO_SRC_IMAGE);
		return 0ULL;
	}


	switch (newColourPolicy)
	{
	case NEWCOLOUR_FULL:
		if ( (newSize.left()   <= 0)
		||   (newSize.top()    <= 0)
		||   (newSize.right()  >  src->width ())
		||   (newSize.bottom() >  src->height()) )
		{
			if (newColour.keys() != src->planesList())
			{
				complain(LOG_ERR,__FUNCTION__,"Incorrect new colour",Id);
				m_lastErrorCodes[Id]=CODE_INCORrect_NEWCOLOUR;
				return 0ULL;
			}
		}
		else
		{
			if (!newColour.isEmpty())
				complain(LOG_WARNING,function,"New colour is not empty",Id);
		}
		break;
	case NEWCOLOUR_PLAIN:
		foreach (int colourPlane,src->planes())
			newColour[colourPlane]=value;
		break;
	case NEWCOLOUR_IGNORE:
		break;
	}

	switch (newSizePolicy)
	{
	case NEWSIZE_RESIZE:
		break;
	case NEWSIZE_CROP:
		if (newSize.left() <= 0)
		{
			complain(LOG_WARNING,__FUNCTION__,"Shifting left side",Id);
			newSize.setLeft(0);
		}
		if (newSize.top()  <= 0)
		{
			complain(LOG_WARNING,__FUNCTION__,"Shifting top side",Id);
			newSize.setTop(0);
		}
		if (newSize.right()  > src->width ())
		{
			complain(LOG_WARNING,__FUNCTION__,"Shifting right side",Id);
			newSize.setRight(src->width());
		}
		if (newSize.bottom() > src->height())
		{
			complain(LOG_WARNING,__FUNCTION__,"Shifting bottom side",Id);
			newSize.setBottom(src->height());
		}
		break;
	case NEWSIZE_AUTOCROP:
	{
		if (!src->hasAlpha())
		{
			complain(LOG_ERR,__FUNCTION__,"Source image does not have alpha channel",Id);
			m_lastErrorCodes[Id]=CODE_NO_ALPHA;
			return 0ULL;
		}
		const Image::Pixel *src_alpha=src->plane(Image::PLANE_ALPHA);
		const int &src_area=src->area();
		bool ok=false;
		for (	int p=0; p<src_area; ++p)
			if (src_alpha[p])
			{
				ok=true;
				break;
			}
		if (!ok)
		{
			complain(LOG_ERR,__FUNCTION__,"Source image completely transparent",Id);
			m_lastErrorCodes[Id]=CODE_TRANSPARENT_SRC_IMAGE;
			return 0ULL;
		}

		newSize=rect(0,0,src->width(),src->height());
	}
		break;
	}


	qulonglong ret=m_core->createImage();
	if (!ret)
	{
		complain(LOG_CRIT,function,"Cannot create destination image",ret);
		m_lastErrorCodes[Id]=Core::CODE_NO_DST_IMAGE;
		return 0ULL;
	}

	Image *dst=getOrComplain(function,"destination image",ret,busy);
	if (!dst)
	{
		complain(LOG_ERR,__FUNCTION__,"Cannot get destination image",Id);
		m_lastErrorCodes[Id]=busy?(Core::CODE_DST_IMAGE_BUSY):(Core::CODE_NO_DST_IMAGE);
		return 0ULL;
	}


	message(LOG_INFO,function,QString("Resizing to [%1] to [%2,%3 %4x%5]").arg(ret).arg(newSize.left()).arg(newSize.top()).arg(newSize.width()).arg(newSize.height()),Id);

	doLongProcessing(QList<Image*>() << src << dst,QtConcurrent::run(boost::bind(&PluginResize::do_resize,this,function,Id,src,ret,dst,newSize,newColour,newSizePolicy==NEWSIZE_AUTOCROP)));

	m_lastErrorCodes.remove(Id);
	return ret;
}

qulonglong PluginResize::resize(qulonglong Id,int x,int y,uint w,uint h,QfullPixel newColour)
{
	return resizeCommon(__FUNCTION__,Id,NEWSIZE_RESIZE,NEWCOLOUR_FULL,rect(x,y,w,h),newColour);
}

qulonglong PluginResize::resizeBlack(qulonglong Id,int x,int y,uint w,uint h)
{
	return resizeCommon(__FUNCTION__,Id,NEWSIZE_RESIZE,NEWCOLOUR_PLAIN,rect(x,y,w,h),0);
}

qulonglong PluginResize::resizeWhite(qulonglong Id,int x,int y,uint w,uint h)
{
	return resizeCommon(__FUNCTION__,Id,NEWSIZE_RESIZE,NEWCOLOUR_PLAIN,rect(x,y,w,h),0xffff);
}

qulonglong PluginResize::crop(qulonglong Id,int x,int y,uint w,uint h)
{
	return resizeCommon(__FUNCTION__,Id,NEWSIZE_CROP,NEWCOLOUR_IGNORE,rect(x,y,w,h));
}

qulonglong PluginResize::autoCrop(qulonglong Id)
{
	return resizeCommon(__FUNCTION__,Id,NEWSIZE_AUTOCROP,NEWCOLOUR_IGNORE);
}


namespace {
	typedef struct planePair
	{
		int id;
		Image::Pixel *src;
		Image::Pixel *dst;
	} planePair;
}

void PluginResize::do_resize(const char *function,qulonglong srcId,Image *src,qulonglong dstId,Image *dst,rect newSize,QfullPixel newColour,bool autoCrop)
{
	connect(this,SIGNAL(setPercent(double)),src,SLOT(setPercent(double)));
	connect(this,SIGNAL(setPercent(double)),dst,SLOT(setPercent(double)));


	int src_width =src->width();
	int src_height=src->height();


	if (autoCrop)
	{
		const Image::Pixel *src_alpha=src->plane(Image::PLANE_ALPHA);
		for (bool ok=true; ok; )
		{
			int yo=newSize.top()*src_width;
			for (int x=0; x<src_width; ++x)
				if (src_alpha[yo+x])
				{
					ok=false;
					break;
				}
			if (ok)
				newSize.moveTop(1);
		}
		for (bool ok=true; ok; )
		{
			int yo=(newSize.bottom()-1)*src_width;
			for (int x=0; x<src_width; ++x)
				if (src_alpha[yo+x])
				{
					ok=false;
					break;
				}
			if (ok)
				newSize.moveBottom(-1);
		}
		for (bool ok=true; ok; )
		{
			int xo=newSize.left();
			for (int y=newSize.top(); y<newSize.bottom(); ++y)
				if (src_alpha[xo+y*src_width])
				{
					ok=false;
					break;
				}
			if (ok)
				newSize.moveLeft(1);
		}
		for (bool ok=true; ok; )
		{
			int xo=newSize.right()-1;
			for (int y=newSize.top(); y<newSize.bottom(); ++y)
				if (src_alpha[xo+y*src_width])
				{
					ok=false;
					break;
				}
			if (ok)
				newSize.moveRight(-1);
		}
	}

	dst->setOffset(src->offset()+QPoint(newSize.left(),newSize.top()));
	dst->setSize(QPoint(newSize.width(),newSize.height()));

	int planesCount=src->planesCount();
	boost::scoped_array<planePair> planePairs_s(new planePair[planesCount]);
	planePair *planePairs=planePairs_s.get();

	{
		int i=0;
		foreach (int colourPlane,src->planesList())
		{
			dst->addPlane(colourPlane);
			planePairs[i].id=colourPlane;
			planePairs[i].src=src->plane(colourPlane);
			planePairs[i].dst=dst->plane(colourPlane);
			++i;
		}
	}


	QPoint overlap_s(
		std::max(0,newSize.left()),
		std::max(0,newSize.top ()) );
	QPoint overlap_d(
		std::max(-newSize.left(),0),
		std::max(-newSize.top (),0) );
	QPoint delta_overlap(
		std::min(src_width ,newSize.right ()),
		std::min(src_height,newSize.bottom()) );
	delta_overlap-=overlap_s;

	int dst_width =newSize.width ();
	int dst_height=newSize.height();

	if (overlap_d.y() > 0)
	{
		for (int p=0; p<planesCount; ++p)
		{
			Image::Pixel v=newColour[planePairs[p].id];
			int to_y=overlap_d.y();
			for (int y=0; y<to_y; ++y)
			{
				int yo=y*dst_width;
				for (int x=0; x<dst_width; ++x)
					planePairs[p].dst[yo+x]=v;
			}
		}
	}
	if (dst_height>overlap_d.y()+delta_overlap.y())
	{
		for (int p=0; p<planesCount; ++p)
		{
			Image::Pixel v=newColour[planePairs[p].id];
			int from_y=overlap_d.y()+delta_overlap.y();
			for (int y=from_y; y<dst_height; ++y)
			{
				int yo=y*dst_width;
				for (int x=0; x<dst_width; ++x)
					planePairs[p].dst[yo+x]=v;
			}
		}
	}

	if ( (!newSize.left())
	&&   (dst_width == src_width) )
		for (int p=0; p<planesCount; ++p)
			memcpy(
				planePairs[p].dst+overlap_d.y()*dst_width,
				planePairs[p].src+overlap_s.y()*src_width,
				delta_overlap.y()*delta_overlap.x()*sizeof(Image::Pixel) );
	else
	{
		int ls_x=overlap_d.x();
		int rs_x=overlap_d.x()+delta_overlap.x();
		int to_y=delta_overlap.y();

		if ( (overlap_d.x() > 0)
		||   (dst_width>overlap_d.x()+delta_overlap.x()) )
		{
			for (int p=0; p<planesCount; ++p)
			{
				Image::Pixel v=newColour[planePairs[p].id];
				for (int y=0; y<to_y; ++y)
				{
					int yo=(y+overlap_d.y())*dst_width;
					for (int x=0; x<ls_x; ++x)
						planePairs[p].dst[yo+x]=v;
					for (int x=rs_x; x<dst_width; ++x)
						planePairs[p].dst[yo+x]=v;
				}
			}
		}
		for (int y=0; y<to_y; ++y)
		{
			for (int p=0; p<planesCount; ++p)
				memcpy(
					planePairs[p].dst+(overlap_d.y()+y)*dst_width+overlap_d.x(),
					planePairs[p].src+(overlap_s.y()+y)*src_width+overlap_s.x(),
					delta_overlap.x()*sizeof(Image::Pixel) );
		}
	}

	planePairs_s.reset();


	foreach (QString key,src->textKeysList())
		dst->setText(key,src->text(key));


	disconnect(src,SLOT(setPercent(double)));
	disconnect(dst,SLOT(setPercent(double)));

	message(LOG_INFO,function,QString("Resized to [%1] to [%2,%3 %4x%5]").arg(dstId).arg(newSize.left()).arg(newSize.top()).arg(newSize.width()).arg(newSize.height()),srcId);
}

uint PluginResize::lastErrorCode(qulonglong image)
{
	lastErrorCodes_t::ConstIterator I=m_lastErrorCodes.constFind(image);
	if (I == m_lastErrorCodes.constEnd())
		return Core::CODE_OK;
	return I.value();
}

QString PluginResize::errorCodeToString(uint errorCode) const
{
	switch (errorCode)
	{
		#define CASE(ERR,STR) case CODE_##ERR: return STR ;
		CASE(INCORrect_WIDTH      ,"New width should be greater than zero")
		CASE(INCORrect_HEIGHT     ,"New height should be greater than zero")
		CASE(INCORrect_NEWCOLOUR  ,"Mismatching colour planes in new colour")
		CASE(NO_ALPHA             ,"No alpha channel")
		CASE(TRANSPARENT_SRC_IMAGE,"Transparent source image")
		#undef CASE
	}
	return m_core->errorCodeToString(errorCode);
}
