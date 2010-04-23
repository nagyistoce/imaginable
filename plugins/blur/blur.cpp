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


#include "blur.hpp"
#include "dbus_plugin_blur_adaptor.h"

#include <boost/scoped_array.hpp>


Q_EXPORT_PLUGIN2(blur,PluginBlur)


PluginBlur::PluginBlur(void)
	: QObject(NULL)
	, PluginInterface()
{
	new BlurAdaptor(this);
}




uint PluginBlur::boxAll(qulonglong Id,uint size)
{
	bool busy;
	Image* img=getOrComplain(__FUNCTION__,"image",Id,busy);
	if(!img)
		return busy?(Core::CODE_IMAGE_BUSY):(Core::CODE_NO_IMAGE);

	if(img->isEmpty())
		return CODE_IMAGE_IS_EMPTY;


	if( (size>static_cast<uint>(img->width ()))
	&&  (size>static_cast<uint>(img->height())) )
	{
		uint new_size=std::max(img->width(),img->height());
		message(LOG_WARNING,__FUNCTION__,QString("Box blur value [%1] is too high, reducing to [%2]").arg(size).arg(new_size),Id);
		size=new_size;
	}

	message(LOG_INFO,__FUNCTION__,QString("Applying box blur [%1]").arg(size),Id);

	doLongProcessing(img,QtConcurrent::run(boost::bind(&PluginBlur::do_boxAll,this,Id,img,size)));

	return Core::CODE_OK;
}

uint PluginBlur::boxAllPercent(qulonglong Id,uint size)
{
	bool busy;
	Image* img=getOrComplain(__FUNCTION__,"image",Id,busy);
	if(!img)
		return busy?(Core::CODE_IMAGE_BUSY):(Core::CODE_NO_IMAGE);

	if(img->isEmpty())
		return CODE_IMAGE_IS_EMPTY;

	if(size>100)
	{
		message(LOG_WARNING,__FUNCTION__,QString("Box blur value [%1\%] is too high, reducing to [100%]").arg(size),Id);
		size=100;
	}

	message(LOG_INFO,__FUNCTION__,QString("Applying box blur [%1\%]").arg(size),Id);

	size=static_cast<uint>(static_cast<double>(size)*static_cast<double>(std::max(img->width(),img->height()))/100.);

	message(LOG_INFO,__FUNCTION__,QString("Applying box blur [%1]").arg(size),Id);

	doLongProcessing(img,QtConcurrent::run(boost::bind(&PluginBlur::do_boxAll,this,Id,img,size)));

	return Core::CODE_OK;
}

void PluginBlur::do_boxAll(qulonglong Id,Image* img,int size)
{
	connect(this,SIGNAL(setPercent(double)),img,SLOT(setPercent(double)));

	double from=0.;
	double step=100./static_cast<double>(img->planesList().size());
	foreach(int colourPlane,img->planesList())
	{
		do_box(Id,img,colourPlane,size,from,step);
		from+=step;
	}

	disconnect(img,SLOT(setPercent(double)));

	message(LOG_INFO,"box",QString("Box blur [%1] applied").arg(size),Id);
}

uint PluginBlur::box(qulonglong Id,int colourPlane,uint size)
{
	bool busy;
	Image* img=getOrComplain(__FUNCTION__,"image",Id,busy);
	if(!img)
		return busy?(Core::CODE_IMAGE_BUSY):(Core::CODE_NO_IMAGE);

	if(img->isEmpty())
		return CODE_IMAGE_IS_EMPTY;

	if(!img->hasPlane(colourPlane))
		return CODE_NO_COLOUR_PLANE;


	if( (size>static_cast<uint>(img->width ()))
	&&  (size>static_cast<uint>(img->height())) )
	{
		uint new_size=std::max(img->width(),img->height());
		message(LOG_WARNING,__FUNCTION__,QString("Box blur value [%1] is too high, reducing [%2]").arg(size).arg(new_size),Id);
		size=new_size;
	}

	message(LOG_INFO,__FUNCTION__,QString("Applying box blur [%1] to colour plane [%2]").arg(size).arg(colourPlane),Id);


	doLongProcessing(img,QtConcurrent::run(boost::bind(&PluginBlur::do_boxPlain,this,Id,img,colourPlane,size)));

	return Core::CODE_OK;
}

uint PluginBlur::boxPercent(qulonglong Id,int colourPlane,uint size)
{
	bool busy;
	Image* img=getOrComplain(__FUNCTION__,"image",Id,busy);
	if(!img)
		return busy?(Core::CODE_IMAGE_BUSY):(Core::CODE_NO_IMAGE);

	if(img->isEmpty())
		return CODE_IMAGE_IS_EMPTY;

	if(!img->hasPlane(colourPlane))
		return CODE_NO_COLOUR_PLANE;

	if(size>100)
	{
		message(LOG_WARNING,__FUNCTION__,QString("Box blur value [%1\%] is too high, reducing to [100%]").arg(size),Id);
		size=100;
	}

	message(LOG_INFO,__FUNCTION__,QString("Applying box blur [%1\%] to colour plane [%2]").arg(size).arg(colourPlane),Id);

	size=static_cast<uint>(static_cast<double>(size)*static_cast<double>(std::max(img->width(),img->height()))/100.);

	message(LOG_INFO,__FUNCTION__,QString("Applying box blur [%1] to colour plane [%2]").arg(size).arg(colourPlane),Id);

	doLongProcessing(img,QtConcurrent::run(boost::bind(&PluginBlur::do_boxPlain,this,Id,img,colourPlane,size)));

	return Core::CODE_OK;
}

void PluginBlur::do_boxPlain(qulonglong Id,Image* img,int colourPlane,int size)
{
	connect(this,SIGNAL(setPercent(double)),img,SLOT(setPercent(double)));

	do_box(Id,img,colourPlane,size,0.,100.);

	disconnect(img,SLOT(setPercent(double)));
}

void PluginBlur::do_box(qulonglong Id,Image* img,int colourPlane,int size,double fromPercent,double stepPercent)
{
	int width =img->width();
	int height=img->height();

	Image::Pixel* data=img->plane(colourPlane);

	{
		int sz=std::min(size,width);
		int fullSz=sz*2+1;
		boost::scoped_array<Image::Pixel> mem_s(new Image::Pixel[fullSz]);
		Image::Pixel* mem=mem_s.get();

		for(int y=0;y<height;++y)
		{
			emit setPercent(fromPercent+static_cast<double>(y)*stepPercent/static_cast<double>(2*height));

			unsigned acc=0;
			int c=0;
			int yo=y*width;
			for(int x=-sz;x<width;++x)
			{
				int x_d=x-sz-1;
				int x_a=x+sz;
				bool d=(x>=sz+1);
				bool p=(x>=0);
				bool a=(x<width-sz);
				if(d!=a)
					c+=a?1:-1;
				if(a)
					acc+=static_cast<unsigned>(data[yo+x_a]);
				if(d)
					acc-=static_cast<unsigned>(mem[x_d%fullSz]);
				if(p)
				{
					mem[x%fullSz]=data[yo+x];
					data[yo+x]=static_cast<Image::Pixel>(acc/c);
				}
			}
		}
	}

	{
		int sz=std::min(size,height);
		int fullSz=sz*2+1;
		int sz_yo=sz*width;
		boost::scoped_array<Image::Pixel> mem_s(new Image::Pixel[fullSz]);
		Image::Pixel* mem=mem_s.get();

		for(int x=0;x<width;++x)
		{
			emit setPercent(fromPercent+static_cast<double>(width+x)*stepPercent/static_cast<double>(2*width));

			unsigned acc=0;
			int c=0;
			for(int y=-sz,yw=-sz_yo;y<height;yw+=width,++y)
			{
				int y_d=y-sz-1;
				int y_a=yw+sz_yo;
				bool d=(y>=sz+1);
				bool p=(y>=0);
				bool a=(y<height-sz);
				if(d!=a)
					c+=a?1:-1;
				if(a)
					acc+=static_cast<unsigned>(data[y_a+x]);
				if(d)
					acc-=static_cast<unsigned>(mem[y_d%fullSz]);
				if(p)
				{
					mem[y%fullSz]=data[yw+x];
					data[yw+x]=static_cast<Image::Pixel>(acc/c);
				}
			}
		}
	}

	message(LOG_INFO,"box",QString("Box blur [%1] applied to colour plane [%2]").arg(size).arg(colourPlane),Id);
}




uint PluginBlur::frameAll(qulonglong Id,uint size,uint hole)
{
	bool busy;
	Image* img=getOrComplain(__FUNCTION__,"image",Id,busy);
	if(!img)
		return busy?(Core::CODE_IMAGE_BUSY):(Core::CODE_NO_IMAGE);

	if(img->isEmpty())
		return CODE_IMAGE_IS_EMPTY;


	if( (size>static_cast<uint>(img->width ()))
	&&  (size>static_cast<uint>(img->height())) )
	{
		uint new_size=std::max(img->width(),img->height());
		message(LOG_WARNING,__FUNCTION__,QString("Frame blur size value [%1] is too high, reducing to [%2]").arg(size).arg(new_size),Id);
		size=new_size;
	}

	if(hole>=size)
	{
		uint new_hole=size-1;
		message(LOG_WARNING,__FUNCTION__,QString("Frame blur hole value [%1] is too high, reducing to [%2]").arg(hole).arg(new_hole),Id);
		hole=new_hole;
	}

	message(LOG_INFO,__FUNCTION__,QString("Applying frame blur [%1] [%2]").arg(size).arg(hole),Id);

	doLongProcessing(img,QtConcurrent::run(boost::bind(&PluginBlur::do_frameAll,this,Id,img,size,hole)));

	return Core::CODE_OK;
}

void PluginBlur::do_frameAll(qulonglong Id,Image* img,int size,int hole)
{
	connect(this,SIGNAL(setPercent(double)),img,SLOT(setPercent(double)));

	double from=0.;
	double step=100./static_cast<double>(img->planesList().size());
	foreach(int colourPlane,img->planesList())
	{
		do_frame(Id,img,colourPlane,size,hole,from,step);
		from+=step;
	}

	disconnect(img,SLOT(setPercent(double)));

	message(LOG_INFO,"frame",QString("Frame blur [%1] applied").arg(size),Id);
}

uint PluginBlur::frame(qulonglong Id,int colourPlane,uint size,uint hole)
{
	bool busy;
	Image* img=getOrComplain(__FUNCTION__,"image",Id,busy);
	if(!img)
		return busy?(Core::CODE_IMAGE_BUSY):(Core::CODE_NO_IMAGE);

	if(img->isEmpty())
		return CODE_IMAGE_IS_EMPTY;

	if(!img->hasPlane(colourPlane))
		return CODE_NO_COLOUR_PLANE;


	if( (size>static_cast<uint>(img->width ()))
	&&  (size>static_cast<uint>(img->height())) )
	{
		uint new_size=std::max(img->width(),img->height());
		message(LOG_WARNING,__FUNCTION__,QString("Frame blur size value [%1] is too high, reducing [%2]").arg(size).arg(new_size),Id);
		size=new_size;
	}

	if(hole>=size)
	{
		uint new_hole=size-1;
		message(LOG_WARNING,__FUNCTION__,QString("Frame blur hole value [%1] is too high, reducing to [%2]").arg(hole).arg(new_hole),Id);
		hole=new_hole;
	}

	message(LOG_INFO,__FUNCTION__,QString("Applying frame blur [%1] [%2] to colour plane [%3]").arg(size).arg(hole).arg(colourPlane),Id);


	doLongProcessing(img,QtConcurrent::run(boost::bind(&PluginBlur::do_framePlain,this,Id,img,colourPlane,size,hole)));

	return Core::CODE_OK;
}

void PluginBlur::do_framePlain(qulonglong Id,Image* img,int colourPlane,int size,int hole)
{
	connect(this,SIGNAL(setPercent(double)),img,SLOT(setPercent(double)));

	do_frame(Id,img,colourPlane,size,hole,0.,100.);

	disconnect(img,SLOT(setPercent(double)));
}

void PluginBlur::do_frame(qulonglong Id,Image* img,int colourPlane,int size,int hole,double fromPercent,double stepPercent)
{
	int width =img->width();
	int height=img->height();

	Image::Pixel* data=img->plane(colourPlane);

	if(!img->hasPlane(Image::PLANE__INTERNAL))
		img->addPlane(Image::PLANE__INTERNAL);
	Image::Pixel* dst=img->plane(Image::PLANE__INTERNAL);

	for(int x=-size;x<width;++x)
	{
		emit setPercent(fromPercent+static_cast<double>(x)*stepPercent/static_cast<double>(width));

		int xs_l=std::max(0,x-size);
		int xs_r=std::min(width,x+size+1);

		int xh_l=std::max(0,x-hole);
		int xh_r=std::min(width,x+hole+1);

		unsigned acc=0;
		int c=0;
		for(int y=-size;y<height;++y)
		{
			int ys_d=y-size-1;
			int ys_a=y+size;
			bool ys_df=(y>=size+1);
			bool ys_af=(y<height-size);

			int yh_d=y+hole;
			int yh_a=y-hole-1;
			bool yh_df=(yh_d>=0) && (yh_d<height);
			bool yh_af=(yh_a>=0) && (yh_a<height);

			bool p=(y>=0)&&(x>=0);

			if(ys_af)
			{
				c+=xs_r-xs_l;

				int ysoa=ys_a*width;
				for(int a=xs_l;a<xs_r;++a)
					acc+=static_cast<unsigned>(data[ysoa+a]);
			}
			if(yh_af)
			{
				c+=xh_r-xh_l;

				int yhoa=yh_a*width;
				for(int a=xh_l;a<xh_r;++a)
					acc+=static_cast<unsigned>(data[yhoa+a]);
			}
			if(ys_df)
			{
				c-=xs_r-xs_l;

				int ysod=ys_d*width;
				for(int a=xs_l;a<xs_r;++a)
					acc-=static_cast<unsigned>(data[ysod+a]);
			}
			if(yh_df)
			{
				c-=xh_r-xh_l;

				int yhod=yh_d*width;
				for(int a=xh_l;a<xh_r;++a)
					acc-=static_cast<unsigned>(data[yhod+a]);
			}
			if(p)
				dst[y*width+x]=static_cast<Image::Pixel>(acc/c);
		}
	}
	img->removePlane(colourPlane);
	img->movePlane(Image::PLANE__INTERNAL,colourPlane);


	message(LOG_INFO,"frame",QString("Frame blur [%1] [%2] applied to colour plane [%3]").arg(size).arg(hole).arg(colourPlane),Id);
}



QString PluginBlur::errorCodeToString(uint errorCode) const
{
	switch(errorCode)
	{
		#define CASE(ERR,STR) case CODE_##ERR: return STR ;
		CASE(NO_COLOUR_PLANE,"No colour plane")
		CASE(IMAGE_IS_EMPTY ,"Image is empty")
		#undef CASE
	}
	return m_core->errorCodeToString(errorCode);
}
