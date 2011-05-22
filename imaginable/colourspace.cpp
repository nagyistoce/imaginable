/*************
**
** Project:      Imaginable
** File info:    $Id$
** Author:       Copyright (C) 2009 - 2011 Kuzma Shapran <Kuzma.Shapran@gmail.com>
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


#include "colourspace.hpp"


namespace imaginable {

void rgb_to_hsv(double r,double g,double b,double &h,double &s,double &v)
{
	h=0.;
	s=0.;
	double rgbmin=std::min(r,std::min(g,b));
	v=std::max(r,std::max(g,b));
	double rgbdelta=v-rgbmin;

	if (v>0.)
		s=rgbdelta/v;

	if (s>0.)
	{
		if (r==v)
			h=   (g-b)/rgbdelta;
		else
		if (g==v)
			h=2.+(b-r)/rgbdelta;
		else
		if (b==v)
			h=4.+(r-g)/rgbdelta;
		h/=6.;
		if (h<0.)
			h+=1.;
	}
}

void hsv_to_rgb(double h,double s,double v,double &r,double &g,double &b)
{
	if (s<=0.)
	{
		r=g=b=v;
	}
	else
	{
		int i;
		double f,p,q,t;

		if (h>=1.)
			h=0.;
		h*=6.;
		i=static_cast<int>(h);
		f=h-static_cast<double>(i);
		p=v*(1- s       );
		q=v*(1-(s*   f ));
		t=v*(1-(s*(1-f)));

		switch (i)
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

void rgb_to_hsl(double r,double g,double b,double &h,double &s,double &l)
{
	double rgbmin=std::min(r,std::min(g,b));
	double rgbmax=std::max(r,std::max(g,b));
	double rgbdelta=rgbmax-rgbmin;

	h=0.;
	s=0.;
	l=(rgbmax+rgbmin)/2.;

	if (rgbdelta>0.)
	{
		s=rgbdelta/(2.*((l>.5)? (1.-l) : l));

		if (r==rgbmax)
			h=   (g-b)/rgbdelta;
		else
		if (g==rgbmax)
			h=2.+(b-r)/rgbdelta;
		else
		if (b==rgbmax)
			h=4.+(r-g)/rgbdelta;
		h/=6.;
		if (h<0.)
			h+=1.;
	}
}

void rgb_to_lightness(double r,double g,double b,double &l)
{
	double rgbmin=std::min(r,std::min(g,b));
	double rgbmax=std::max(r,std::max(g,b));

	l=(rgbmax+rgbmin)/2.;
}

void hsl_to_rgb(double h,double s,double l,double &r,double &g,double &b)
{
	double q=(l>.5)? (l+s-(l*s)) : (l*(1.+s));
	double p=2*l-q;
	double t[3]={h+1./3.,h,h-1./3.};
	if (t[0]>1.)
		t[0]-=1.;
	if (t[2]<0.)
		t[2]+=1.;
	double* c[3]={&r,&g,&b};
	for (size_t i=0; i<3; ++i)
	{
		*c[i]=
			((t[i]<1./6.)? (p+((q-p)*6.*t[i])) :
			((t[i]<1./2.)? q :
			((t[i]<2./3.)? (p+((q-p)*6.*(2./3.-t[i]))) :
			/*         */  p )));
	}
}

void pixel_rgb_to_hsv(Image::pixel red,Image::pixel green,Image::pixel blue,Image::pixel& hue,Image::pixel& saturation,Image::pixel& value,double maximum)
{
	double dh,ds,dv;
	rgb_to_hsv(
		static_cast<double>(red  )/maximum,
		static_cast<double>(green)/maximum,
		static_cast<double>(blue )/maximum,
		dh,
		ds,
		dv);
	hue       =static_cast<Image::pixel>(dh*maximum);
	saturation=static_cast<Image::pixel>(ds*maximum);
	value     =static_cast<Image::pixel>(dv*maximum);
}

void pixel_hsv_to_rgb(Image::pixel hue,Image::pixel saturation,Image::pixel value,Image::pixel& red,Image::pixel& green,Image::pixel& blue,double maximum)
{
	double dr,dg,db;
	hsv_to_rgb(
		static_cast<double>(hue       )/maximum,
		static_cast<double>(saturation)/maximum,
		static_cast<double>(value     )/maximum,
		dr,
		dg,
		db);
	red  =static_cast<Image::pixel>(dr*maximum);
	green=static_cast<Image::pixel>(dg*maximum);
	blue =static_cast<Image::pixel>(db*maximum);
}

void pixel_rgb_to_hsl(Image::pixel red,Image::pixel green,Image::pixel blue,Image::pixel& hue,Image::pixel& saturation,Image::pixel& lightness,double maximum)
{
	double dh,ds,dl;
	rgb_to_hsl(
		static_cast<double>(red  )/maximum,
		static_cast<double>(green)/maximum,
		static_cast<double>(blue )/maximum,
		dh,
		ds,
		dl);
	hue       =static_cast<Image::pixel>(dh*maximum);
	saturation=static_cast<Image::pixel>(ds*maximum);
	lightness =static_cast<Image::pixel>(dl*maximum);
}

void pixel_rgb_to_lightness(Image::pixel red,Image::pixel green,Image::pixel blue,Image::pixel& lightness,double maximum)
{
	double dl;
	rgb_to_lightness(
		static_cast<double>(red  )/maximum,
		static_cast<double>(green)/maximum,
		static_cast<double>(blue )/maximum,
		dl);
	lightness =static_cast<Image::pixel>(dl*maximum);
}

void pixel_hsl_to_rgb(Image::pixel hue,Image::pixel saturation,Image::pixel lightness,Image::pixel& red,Image::pixel& green,Image::pixel& blue,double maximum)
{
	double dr,dg,db;
	hsl_to_rgb(
		static_cast<double>(hue       )/maximum,
		static_cast<double>(saturation)/maximum,
		static_cast<double>(lightness )/maximum,
		dr,
		dg,
		db);
	red  =static_cast<Image::pixel>(dr*maximum);
	green=static_cast<Image::pixel>(dg*maximum);
	blue =static_cast<Image::pixel>(db*maximum);
}

void rgb_to_hsv(Image& img,bool keep_rgb,progress_notifier notifier)
{
	if (!img.hasData())
		throw exception(exception::NO_IMAGE);

	if (img.colourSpace() != Image::IMAGE_RGB)
		throw exception(exception::INVALID_COLOUR_SPACE);

	size_t width =img.width();
	size_t height=img.height();

	img.addPlane(Image::PLANE_HUE);
	img.addPlane(Image::PLANE_HSV_SATURATION);
	img.addPlane(Image::PLANE_HSV_VALUE);

	const Image::pixel* red       =img.plane(Image::PLANE_RED);
	const Image::pixel* green     =img.plane(Image::PLANE_GREEN);
	const Image::pixel* blue      =img.plane(Image::PLANE_BLUE);
	/* */ Image::pixel* hue       =img.plane(Image::PLANE_HUE);
	/* */ Image::pixel* saturation=img.plane(Image::PLANE_HSV_SATURATION);
	/* */ Image::pixel* value     =img.plane(Image::PLANE_HSV_VALUE);

	double dmax=static_cast<double>(img.maximum());
	for (size_t y=0; y<height; ++y)
	{
		notifier(static_cast<float>(y)/static_cast<float>(height));

		size_t yo=y*width;
		for (size_t x=0; x<width; ++x)
		{
			size_t xyo=yo+x;
			pixel_rgb_to_hsv(red[xyo],green[xyo],blue[xyo],hue[xyo],saturation[xyo],value[xyo],dmax);
		}
	}

	if (!keep_rgb)
	{
		img.removePlane(Image::PLANE_RED);
		img.removePlane(Image::PLANE_GREEN);
		img.removePlane(Image::PLANE_BLUE);
	}
}

void hsv_to_rgb(Image& img,bool keep_hsv,progress_notifier notifier)
{
	if (!img.hasData())
		throw exception(exception::NO_IMAGE);

	if (img.colourSpace() != Image::IMAGE_HSV)
		throw exception(exception::INVALID_COLOUR_SPACE);

	size_t width =img.width();
	size_t height=img.height();

	img.addPlane(Image::PLANE_RED);
	img.addPlane(Image::PLANE_GREEN);
	img.addPlane(Image::PLANE_BLUE);

	const Image::pixel* hue       =img.plane(Image::PLANE_HUE);
	const Image::pixel* saturation=img.plane(Image::PLANE_HSV_SATURATION);
	const Image::pixel* value     =img.plane(Image::PLANE_HSV_VALUE);
	/* */ Image::pixel* red       =img.plane(Image::PLANE_RED);
	/* */ Image::pixel* green     =img.plane(Image::PLANE_GREEN);
	/* */ Image::pixel* blue      =img.plane(Image::PLANE_BLUE);

	double dmax=static_cast<double>(img.maximum());
	for (size_t y=0; y<height; ++y)
	{
		notifier(static_cast<float>(y)/static_cast<float>(height));

		size_t yo=y*width;
		for (size_t x=0; x<width; ++x)
		{
			size_t xyo=yo+x;
			pixel_hsv_to_rgb(hue[xyo],saturation[xyo],value[xyo],red[xyo],green[xyo],blue[xyo],dmax);
		}
	}

	if (!keep_hsv)
	{
		img.removePlane(Image::PLANE_HUE);
		img.removePlane(Image::PLANE_HSV_SATURATION);
		img.removePlane(Image::PLANE_HSV_VALUE);
	}
}

void rgb_to_hsl(Image& img,bool keep_rgb,progress_notifier notifier)
{
	if (!img.hasData())
		throw exception(exception::NO_IMAGE);

	if (img.colourSpace() != Image::IMAGE_RGB)
		throw exception(exception::INVALID_COLOUR_SPACE);

	size_t width =img.width();
	size_t height=img.height();

	img.addPlane(Image::PLANE_HUE);
	img.addPlane(Image::PLANE_HSL_SATURATION);
	img.addPlane(Image::PLANE_HSL_LIGHTNESS);

	const Image::pixel* red       =img.plane(Image::PLANE_RED);
	const Image::pixel* green     =img.plane(Image::PLANE_GREEN);
	const Image::pixel* blue      =img.plane(Image::PLANE_BLUE);
	/* */ Image::pixel* hue       =img.plane(Image::PLANE_HUE);
	/* */ Image::pixel* saturation=img.plane(Image::PLANE_HSL_SATURATION);
	/* */ Image::pixel* lightness =img.plane(Image::PLANE_HSL_LIGHTNESS);

	double dmax=static_cast<double>(img.maximum());
	for (size_t y=0; y<height; ++y)
	{

		notifier(static_cast<float>(y)/static_cast<float>(height));

		size_t yo=y*width;
		for (size_t x=0; x<width; ++x)
		{
			size_t xyo=yo+x;
			pixel_rgb_to_hsl(red[xyo],green[xyo],blue[xyo],hue[xyo],saturation[xyo],lightness[xyo],dmax);
		}
	}

	if (!keep_rgb)
	{
		img.removePlane(Image::PLANE_RED);
		img.removePlane(Image::PLANE_GREEN);
		img.removePlane(Image::PLANE_BLUE);
	}
}

void hsl_to_rgb(Image& img,bool keep_hsl,progress_notifier notifier)
{
	if (!img.hasData())
		throw exception(exception::NO_IMAGE);

	if (img.colourSpace() != Image::IMAGE_HSL)
		throw exception(exception::INVALID_COLOUR_SPACE);

	size_t width =img.width();
	size_t height=img.height();

	img.addPlane(Image::PLANE_RED);
	img.addPlane(Image::PLANE_GREEN);
	img.addPlane(Image::PLANE_BLUE);

	const Image::pixel* hue       =img.plane(Image::PLANE_HUE);
	const Image::pixel* saturation=img.plane(Image::PLANE_HSL_SATURATION);
	const Image::pixel* lightness =img.plane(Image::PLANE_HSL_LIGHTNESS);
	/* */ Image::pixel* red       =img.plane(Image::PLANE_RED);
	/* */ Image::pixel* green     =img.plane(Image::PLANE_GREEN);
	/* */ Image::pixel* blue      =img.plane(Image::PLANE_BLUE);

	double dmax=static_cast<double>(img.maximum());
	for (size_t y=0; y<height; ++y)
	{
		notifier(static_cast<float>(y)/static_cast<float>(height));

		size_t yo=y*width;
		for (size_t x=0; x<width; ++x)
		{
			size_t xyo=yo+x;
			pixel_hsl_to_rgb(hue[xyo],saturation[xyo],lightness[xyo],red[xyo],green[xyo],blue[xyo],dmax);
		}
	}

	if (!keep_hsl)
	{
		img.removePlane(Image::PLANE_HUE);
		img.removePlane(Image::PLANE_HSL_SATURATION);
		img.removePlane(Image::PLANE_HSL_LIGHTNESS);
	}
}

void rgb_to_lightness(Image& img,bool keep_rgb,progress_notifier notifier)
{
	if (!img.hasData())
		throw exception(exception::NO_IMAGE);

	if (img.colourSpace() != Image::IMAGE_RGB)
		throw exception(exception::INVALID_COLOUR_SPACE);

	size_t width =img.width();
	size_t height=img.height();

	img.addPlane(Image::PLANE_HSL_LIGHTNESS);

	const Image::pixel* red       =img.plane(Image::PLANE_RED);
	const Image::pixel* green     =img.plane(Image::PLANE_GREEN);
	const Image::pixel* blue      =img.plane(Image::PLANE_BLUE);
	/* */ Image::pixel* lightness =img.plane(Image::PLANE_HSL_LIGHTNESS);

	double dmax=static_cast<double>(img.maximum());
	for (size_t y=0; y<height; ++y)
	{
		notifier(static_cast<float>(y)/static_cast<float>(height));

		size_t yo=y*width;
		for (size_t x=0; x<width; ++x)
		{
			size_t xyo=yo+x;
			pixel_rgb_to_lightness(red[xyo],green[xyo],blue[xyo],lightness[xyo],dmax);
		}
	}

	if (!keep_rgb)
	{
		img.removePlane(Image::PLANE_RED);
		img.removePlane(Image::PLANE_GREEN);
		img.removePlane(Image::PLANE_BLUE);
	}
}

}
