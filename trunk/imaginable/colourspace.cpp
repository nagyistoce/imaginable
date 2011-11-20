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

#include <cmath>


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

void hsl_to_rgb(double h,double s,double l,double &r,double &g,double &b)
{
	double q = (l>.5) ? (l+s-(l*s)) : (l*(1.+s));
	double p = 2*l-q;
	double t[3] = {h+1./3., h, h-1./3.};
	if (t[0] >  1.)
		t[0] -= 1.;
	if (t[2] <  0.)
		t[2] += 1.;
	double* c[3] = {&r, &g, &b};
	for (size_t i=0; i<3; ++i)
	{
		*c[i]=
			((t[i]<1./6.) ? (p+((q-p)*6.*t[i])) :
			((t[i]<1./2.) ?  q :
			((t[i]<2./3.) ? (p+((q-p)*6.*(2./3.-t[i]))) :
			/*         */    p )));
	}
}

void rgb_to_lightness(double r,double g,double b,double &l)
{
	double rgbmin = std::min(r,std::min(g,b));
	double rgbmax = std::max(r,std::max(g,b));

	l = (rgbmax + rgbmin)/2.;
}

void rgb_to_hcy(double r,double g,double b,double &h, double &c, double &y)
{
	double r_weight = 0.299;
	double g_weight = 0.587;
	double b_weight = 0.114;

	double rgbmin = std::min(std::min(r,g),b);
	double rgbmax = std::max(std::max(r,g),b);

	y = (r_weight * r + g_weight * g + b_weight * b);

	int H_sec = 0;
	double H_insec = 0.;
	if (rgbmax != rgbmin)
	{
		double X;
		if (rgbmax == r)
		{
			if (rgbmin == g)
			{
				H_sec = 5;
				c = r - rgbmin;
				X  = b - rgbmin;
				H_insec = 1. - X/c;
			}
			else
			{
				H_sec = 0;
				c = r - rgbmin;
				X  = g - rgbmin;
				H_insec = X/c;
			}
		}
		else if (rgbmax == g)
		{
			if (rgbmin == b)
			{
				H_sec = 1;
				c = g - rgbmin;
				X  = r - rgbmin;
				H_insec = 1. - X/c;
			}
			else
			{
				H_sec = 2;
				c = g - rgbmin;
				X  = b - rgbmin;
				H_insec = X/c;
			}
		}
		else
		{
			if (rgbmin == r)
			{
				H_sec = 3;
				c = b - rgbmin;
				X  = g - rgbmin;
				H_insec = 1. - X/c;
			}
			else
			{
				H_sec = 4;
				c = b - rgbmin;
				X  = r - rgbmin;
				H_insec = X/c;
			}
		}
	}

	double Y_peak = 0.; // shut up the compiler
	switch (H_sec)
	{
	case 0:
		Y_peak =     r_weight  + H_insec * ((1.-b_weight) -     r_weight );
		break;
	case 1:
		Y_peak = (1.-b_weight) + H_insec * (    g_weight  - (1.-b_weight));
		break;
	case 2:
		Y_peak =     g_weight  + H_insec * ((1.-r_weight) -     g_weight );
		break;
	case 3:
		Y_peak = (1.-r_weight) + H_insec * (    b_weight  - (1.-r_weight));
		break;
	case 4:
		Y_peak =     b_weight  + H_insec * ((1.-g_weight) -     b_weight );
		break;
	case 5:
		Y_peak = (1.-g_weight) + H_insec * (    r_weight  - (1.-g_weight));
		break;
	}
	if (rgbmax != rgbmin)
	{
		if (y < Y_peak)
			c /= y/Y_peak;
		else
			c /=(1. - y)/(1. - Y_peak);
	}


	h = (H_sec + H_insec) / 6.;
}

void hcy_to_rgb(double h, double c, double y,double &r,double &g,double &b)
{
	double r_weight = 0.299;
	double g_weight = 0.587;
	double b_weight = 0.114;

	if (h >= 1.0)
		h -= static_cast<int>(h);

	h *= 6.;
	int H_sec = static_cast<int>(h);
	int H1 = H_sec/2;
	H1*=2;
	double H2 = h - static_cast<double>(H1);


	double Y_peak=0.; // shut up the compiler
	double H_insec = h - H_sec;
	switch (H_sec)
	{
	case 0:
		Y_peak =     r_weight  + H_insec * ((1.-b_weight) -     r_weight );
		break;
	case 1:
		Y_peak = (1.-b_weight) + H_insec * (    g_weight  - (1.-b_weight));
		break;
	case 2:
		Y_peak =     g_weight  + H_insec * ((1.-r_weight) -     g_weight );
		break;
	case 3:
		Y_peak = (1.-r_weight) + H_insec * (    b_weight  - (1.-r_weight));
		break;
	case 4:
		Y_peak =     b_weight  + H_insec * ((1.-g_weight) -     b_weight );
		break;
	case 5:
		Y_peak = (1.-g_weight) + H_insec * (    r_weight  - (1.-g_weight));
		break;
	}
	if (y < Y_peak)
		c *= y/Y_peak;
	else
		c *=(1. - y)/(1. - Y_peak);


	double X = c * (1. - fabs(H2 - 1.));

	r = 0.;
	g = 0.;
	b = 0.;
	switch (H_sec)
	{
	case 0:
		r = c; g = X; /*  */
		break;
	case 1:
		r = X; g = c; /*  */
		break;
	case 2:
		/*  */ g = c; b = X;
		break;
	case 3:
		/*  */ g = X; b = c;
		break;
	case 4:
		r = X; /*  */ b = c;
		break;
	case 5:
		r = c; /*  */ b = X;
		break;
	}

	double rgbmin = y - (r_weight * r + g_weight * g + b_weight * b);

	r += rgbmin;
	g += rgbmin;
	b += rgbmin;
}

void rgb_to_luma(double r,double g,double b,double &l)
{
	double r_weight = 0.299;
	double g_weight = 0.587;
	double b_weight = 0.114;

	l = (r_weight * r + g_weight * g + b_weight * b);
}

void pixel_rgb_to_hsv(Image::Pixel red,Image::Pixel green,Image::Pixel blue,Image::Pixel& hue,Image::Pixel& saturation,Image::Pixel& value,double maximum)
{
	double dh,ds,dv;
	rgb_to_hsv(
		static_cast<double>(red  )/maximum,
		static_cast<double>(green)/maximum,
		static_cast<double>(blue )/maximum,
		dh,
		ds,
		dv);
	hue        = static_cast<Image::Pixel>(dh*maximum);
	saturation = static_cast<Image::Pixel>(ds*maximum);
	value      = static_cast<Image::Pixel>(dv*maximum);
}

void pixel_hsv_to_rgb(Image::Pixel hue,Image::Pixel saturation,Image::Pixel value,Image::Pixel& red,Image::Pixel& green,Image::Pixel& blue,double maximum)
{
	double dr,dg,db;
	hsv_to_rgb(
		static_cast<double>(hue       )/maximum,
		static_cast<double>(saturation)/maximum,
		static_cast<double>(value     )/maximum,
		dr,
		dg,
		db);
	red   = static_cast<Image::Pixel>(dr*maximum);
	green = static_cast<Image::Pixel>(dg*maximum);
	blue  = static_cast<Image::Pixel>(db*maximum);
}

void pixel_rgb_to_hsl(Image::Pixel red,Image::Pixel green,Image::Pixel blue,Image::Pixel& hue,Image::Pixel& saturation,Image::Pixel& lightness,double maximum)
{
	double dh,ds,dl;
	rgb_to_hsl(
		static_cast<double>(red  )/maximum,
		static_cast<double>(green)/maximum,
		static_cast<double>(blue )/maximum,
		dh,
		ds,
		dl);
	hue        = static_cast<Image::Pixel>(dh*maximum);
	saturation = static_cast<Image::Pixel>(ds*maximum);
	lightness  = static_cast<Image::Pixel>(dl*maximum);
}

void pixel_hsl_to_rgb(Image::Pixel hue,Image::Pixel saturation,Image::Pixel lightness,Image::Pixel& red,Image::Pixel& green,Image::Pixel& blue,double maximum)
{
	double dr,dg,db;
	hsl_to_rgb(
		static_cast<double>(hue       )/maximum,
		static_cast<double>(saturation)/maximum,
		static_cast<double>(lightness )/maximum,
		dr,
		dg,
		db);
	red   = static_cast<Image::Pixel>(dr*maximum);
	green = static_cast<Image::Pixel>(dg*maximum);
	blue  = static_cast<Image::Pixel>(db*maximum);
}

void pixel_rgb_to_lightness(Image::Pixel red,Image::Pixel green,Image::Pixel blue,Image::Pixel& lightness,double maximum)
{
	double dl;
	rgb_to_lightness(
		static_cast<double>(red  )/maximum,
		static_cast<double>(green)/maximum,
		static_cast<double>(blue )/maximum,
		dl);
	lightness = static_cast<Image::Pixel>(dl*maximum);
}

void pixel_rgb_to_hcy(Image::Pixel red,Image::Pixel green,Image::Pixel blue,Image::Pixel& hue,Image::Pixel& chroma,Image::Pixel& luma,double maximum)
{
	double dh,dc,dy;
	rgb_to_hcy(
		static_cast<double>(red  )/maximum,
		static_cast<double>(green)/maximum,
		static_cast<double>(blue )/maximum,
		dh,
		dc,
		dy);
	hue    = static_cast<Image::Pixel>(dh*maximum);
	chroma = static_cast<Image::Pixel>(dc*maximum);
	luma   = static_cast<Image::Pixel>(dy*maximum);
}

void pixel_hcy_to_rgb(Image::Pixel hue,Image::Pixel chroma,Image::Pixel luma,Image::Pixel& red,Image::Pixel& green,Image::Pixel& blue,double maximum)
{
	double dr,dg,db;
	hcy_to_rgb(
		static_cast<double>(hue   )/maximum,
		static_cast<double>(chroma)/maximum,
		static_cast<double>(luma  )/maximum,
		dr,
		dg,
		db);
	red   = static_cast<Image::Pixel>(dr*maximum);
	green = static_cast<Image::Pixel>(dg*maximum);
	blue  = static_cast<Image::Pixel>(db*maximum);
}

void pixel_rgb_to_luma(Image::Pixel red,Image::Pixel green,Image::Pixel blue,Image::Pixel& luma,double maximum)
{
	double dl;
	rgb_to_lightness(
		static_cast<double>(red  )/maximum,
		static_cast<double>(green)/maximum,
		static_cast<double>(blue )/maximum,
		dl);
	luma = static_cast<Image::Pixel>(dl*maximum);
}

#define NOTIFY_STEP 10

void rgb_to_hsv(Image& img,bool keep_rgb,progress_notifier notifier)
{
	if (!img.hasData())
		throw exception(exception::NO_IMAGE);

	if (img.colourSpace() != Image::COLOURSPACE_RGB)
		throw exception(exception::INVALID_COLOUR_SPACE);

	size_t width =img.width();
	size_t height=img.height();

	img.addPlane(Image::PLANE_HUE);
	img.addPlane(Image::PLANE_SATURATION);
	img.addPlane(Image::PLANE_VALUE);

	const Image::Pixel* red       =img.plane(Image::PLANE_RED);
	const Image::Pixel* green     =img.plane(Image::PLANE_GREEN);
	const Image::Pixel* blue      =img.plane(Image::PLANE_BLUE);
	/* */ Image::Pixel* hue       =img.plane(Image::PLANE_HUE);
	/* */ Image::Pixel* saturation=img.plane(Image::PLANE_SATURATION);
	/* */ Image::Pixel* value     =img.plane(Image::PLANE_VALUE);

	double dmax=static_cast<double>(img.maximum());
	for (size_t y=0; y<height; ++y)
	{
		if (!(y%NOTIFY_STEP))
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

	if (img.colourSpace() != Image::COLOURSPACE_HSV)
		throw exception(exception::INVALID_COLOUR_SPACE);

	size_t width =img.width();
	size_t height=img.height();

	img.addPlane(Image::PLANE_RED);
	img.addPlane(Image::PLANE_GREEN);
	img.addPlane(Image::PLANE_BLUE);

	const Image::Pixel* hue       =img.plane(Image::PLANE_HUE);
	const Image::Pixel* saturation=img.plane(Image::PLANE_SATURATION);
	const Image::Pixel* value     =img.plane(Image::PLANE_VALUE);
	/* */ Image::Pixel* red       =img.plane(Image::PLANE_RED);
	/* */ Image::Pixel* green     =img.plane(Image::PLANE_GREEN);
	/* */ Image::Pixel* blue      =img.plane(Image::PLANE_BLUE);

	double dmax=static_cast<double>(img.maximum());
	for (size_t y=0; y<height; ++y)
	{
		if (!(y%NOTIFY_STEP))
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
		img.removePlane(Image::PLANE_SATURATION);
		img.removePlane(Image::PLANE_VALUE);
	}
}

void rgb_to_hsl(Image& img,bool keep_rgb,progress_notifier notifier)
{
	if (!img.hasData())
		throw exception(exception::NO_IMAGE);

	if (img.colourSpace() != Image::COLOURSPACE_RGB)
		throw exception(exception::INVALID_COLOUR_SPACE);

	size_t width =img.width();
	size_t height=img.height();

	img.addPlane(Image::PLANE_HUE);
	img.addPlane(Image::PLANE_SATURATION);
	img.addPlane(Image::PLANE_LIGHTNESS);

	const Image::Pixel* red       =img.plane(Image::PLANE_RED);
	const Image::Pixel* green     =img.plane(Image::PLANE_GREEN);
	const Image::Pixel* blue      =img.plane(Image::PLANE_BLUE);
	/* */ Image::Pixel* hue       =img.plane(Image::PLANE_HUE);
	/* */ Image::Pixel* saturation=img.plane(Image::PLANE_SATURATION);
	/* */ Image::Pixel* lightness =img.plane(Image::PLANE_LIGHTNESS);

	double dmax=static_cast<double>(img.maximum());
	for (size_t y=0; y<height; ++y)
	{
		if (!(y%NOTIFY_STEP))
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

	if (img.colourSpace() != Image::COLOURSPACE_HSL)
		throw exception(exception::INVALID_COLOUR_SPACE);

	size_t width =img.width();
	size_t height=img.height();

	img.addPlane(Image::PLANE_RED);
	img.addPlane(Image::PLANE_GREEN);
	img.addPlane(Image::PLANE_BLUE);

	const Image::Pixel* hue       =img.plane(Image::PLANE_HUE);
	const Image::Pixel* saturation=img.plane(Image::PLANE_SATURATION);
	const Image::Pixel* lightness =img.plane(Image::PLANE_LIGHTNESS);
	/* */ Image::Pixel* red       =img.plane(Image::PLANE_RED);
	/* */ Image::Pixel* green     =img.plane(Image::PLANE_GREEN);
	/* */ Image::Pixel* blue      =img.plane(Image::PLANE_BLUE);

	double dmax=static_cast<double>(img.maximum());
	for (size_t y=0; y<height; ++y)
	{
		if (!(y%NOTIFY_STEP))
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
		img.removePlane(Image::PLANE_SATURATION);
		img.removePlane(Image::PLANE_LIGHTNESS);
	}
}

void rgb_to_lightness(Image& img,bool keep_rgb,progress_notifier notifier)
{
	if (!img.hasData())
		throw exception(exception::NO_IMAGE);

	if (img.colourSpace() != Image::COLOURSPACE_RGB)
		throw exception(exception::INVALID_COLOUR_SPACE);

	size_t width =img.width();
	size_t height=img.height();

	img.addPlane(Image::PLANE_LIGHTNESS);

	const Image::Pixel* red       =img.plane(Image::PLANE_RED);
	const Image::Pixel* green     =img.plane(Image::PLANE_GREEN);
	const Image::Pixel* blue      =img.plane(Image::PLANE_BLUE);
	/* */ Image::Pixel* lightness =img.plane(Image::PLANE_LIGHTNESS);

	double dmax=static_cast<double>(img.maximum());
	for (size_t y=0; y<height; ++y)
	{
		if (!(y%NOTIFY_STEP))
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

void rgb_to_hcy(Image& img,bool keep_rgb,progress_notifier notifier)
{
	if (!img.hasData())
		throw exception(exception::NO_IMAGE);

	if (img.colourSpace() != Image::COLOURSPACE_RGB)
		throw exception(exception::INVALID_COLOUR_SPACE);

	size_t width =img.width();
	size_t height=img.height();

	img.addPlane(Image::PLANE_HUE);
	img.addPlane(Image::PLANE_CHROMA);
	img.addPlane(Image::PLANE_LUMA);

	const Image::Pixel* red    =img.plane(Image::PLANE_RED);
	const Image::Pixel* green  =img.plane(Image::PLANE_GREEN);
	const Image::Pixel* blue   =img.plane(Image::PLANE_BLUE);
	/* */ Image::Pixel* hue    =img.plane(Image::PLANE_HUE);
	/* */ Image::Pixel* chroma =img.plane(Image::PLANE_CHROMA);
	/* */ Image::Pixel* luma   =img.plane(Image::PLANE_LUMA);

	double dmax=static_cast<double>(img.maximum());
	for (size_t y=0; y<height; ++y)
	{
		if (!(y%NOTIFY_STEP))
			notifier(static_cast<float>(y)/static_cast<float>(height));

		size_t yo=y*width;
		for (size_t x=0; x<width; ++x)
		{
			size_t xyo=yo+x;
			pixel_rgb_to_hcy(red[xyo],green[xyo],blue[xyo],hue[xyo],chroma[xyo],luma[xyo],dmax);
		}
	}

	if (!keep_rgb)
	{
		img.removePlane(Image::PLANE_RED);
		img.removePlane(Image::PLANE_GREEN);
		img.removePlane(Image::PLANE_BLUE);
	}
}

void hcy_to_rgb(Image& img,bool keep_hcy,progress_notifier notifier)
{
	if (!img.hasData())
		throw exception(exception::NO_IMAGE);

	if (img.colourSpace() != Image::COLOURSPACE_HCY)
		throw exception(exception::INVALID_COLOUR_SPACE);

	size_t width =img.width();
	size_t height=img.height();

	img.addPlane(Image::PLANE_RED);
	img.addPlane(Image::PLANE_GREEN);
	img.addPlane(Image::PLANE_BLUE);

	const Image::Pixel* hue    =img.plane(Image::PLANE_HUE);
	const Image::Pixel* chroma =img.plane(Image::PLANE_CHROMA);
	const Image::Pixel* luma   =img.plane(Image::PLANE_LUMA);
	/* */ Image::Pixel* red    =img.plane(Image::PLANE_RED);
	/* */ Image::Pixel* green  =img.plane(Image::PLANE_GREEN);
	/* */ Image::Pixel* blue   =img.plane(Image::PLANE_BLUE);

	double dmax=static_cast<double>(img.maximum());
	for (size_t y=0; y<height; ++y)
	{
		if (!(y%NOTIFY_STEP))
			notifier(static_cast<float>(y)/static_cast<float>(height));

		size_t yo=y*width;
		for (size_t x=0; x<width; ++x)
		{
			size_t xyo=yo+x;
			pixel_hcy_to_rgb(hue[xyo],chroma[xyo],luma[xyo],red[xyo],green[xyo],blue[xyo],dmax);
		}
	}

	if (!keep_hcy)
	{
		img.removePlane(Image::PLANE_HUE);
		img.removePlane(Image::PLANE_CHROMA);
		img.removePlane(Image::PLANE_LUMA);
	}
}

void rgb_to_luma(Image& img,bool keep_rgb,progress_notifier notifier)
{
	if (!img.hasData())
		throw exception(exception::NO_IMAGE);

	if (img.colourSpace() != Image::COLOURSPACE_RGB)
		throw exception(exception::INVALID_COLOUR_SPACE);

	size_t width =img.width();
	size_t height=img.height();

	img.addPlane(Image::PLANE_LUMA);

	const Image::Pixel* red   = img.plane(Image::PLANE_RED);
	const Image::Pixel* green = img.plane(Image::PLANE_GREEN);
	const Image::Pixel* blue  = img.plane(Image::PLANE_BLUE);
	/* */ Image::Pixel* luma  = img.plane(Image::PLANE_LUMA);

	double dmax=static_cast<double>(img.maximum());
	for (size_t y=0; y<height; ++y)
	{
		if (!(y%NOTIFY_STEP))
			notifier(static_cast<float>(y)/static_cast<float>(height));

		size_t yo=y*width;
		for (size_t x=0; x<width; ++x)
		{
			size_t xyo=yo+x;
			pixel_rgb_to_luma(red[xyo],green[xyo],blue[xyo],luma[xyo],dmax);
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
