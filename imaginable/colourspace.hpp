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


#ifndef IMAGINABLE__TOOLS_COLOURSPACE__INCLUDED
#define IMAGINABLE__TOOLS_COLOURSPACE__INCLUDED


#include "tools.hpp"


namespace imaginable
{
	void rgb_to_hsv(double r,double g,double b,double &h,double &s,double &v);
	void hsv_to_rgb(double h,double s,double v,double &r,double &g,double &b);

	void rgb_to_hsl(double r,double g,double b,double &h,double &s,double &l);
	void hsl_to_rgb(double h,double s,double l,double &r,double &g,double &b);

	void rgb_to_lightness(double r,double g,double b,double &l);

	void rgb_to_hcy(double r,double g,double b,double &h,double &c,double &y);
	void hcy_to_rgb(double h,double c,double y,double &r,double &g,double &b);

	void rgb_to_luma(double r,double g,double b,double &l);


	void rgb_to_hsv(Image& img,bool keep_rgb=false,progress_notifier notifier = dont_notify);
	void hsv_to_rgb(Image& img,bool keep_hsv=false,progress_notifier notifier = dont_notify);

	void rgb_to_hsl(Image& img,bool keep_rgb=false,progress_notifier notifier = dont_notify);
	void hsl_to_rgb(Image& img,bool keep_hsl=false,progress_notifier notifier = dont_notify);

	void rgb_to_lightness(Image& img,bool keep_rgb=false,progress_notifier notifier = dont_notify);

	void rgb_to_hcy(Image& img,bool keep_rgb=false,progress_notifier notifier = dont_notify);
	void hcy_to_rgb(Image& img,bool keep_hcy=false,progress_notifier notifier = dont_notify);

	void rgb_to_luma(Image& img,bool keep_rgb=false,progress_notifier notifier = dont_notify);


	inline void rgb_to_hsv(Image& img,progress_notifier notifier) { rgb_to_hsv(img,false,notifier); }
	inline void hsv_to_rgb(Image& img,progress_notifier notifier) { hsv_to_rgb(img,false,notifier); }

	inline void rgb_to_hsl(Image& img,progress_notifier notifier) { rgb_to_hsl(img,false,notifier); }
	inline void hsl_to_rgb(Image& img,progress_notifier notifier) { hsl_to_rgb(img,false,notifier); }

	inline void rgb_to_lightness(Image& img,progress_notifier notifier) { rgb_to_lightness(img,false,notifier); }

	inline void rgb_to_hcy(Image& img,progress_notifier notifier) { rgb_to_hcy(img,false,notifier); }
	inline void hcy_to_rgb(Image& img,progress_notifier notifier) { hcy_to_rgb(img,false,notifier); }

	inline void rgb_to_luma(Image& img,progress_notifier notifier) { rgb_to_lightness(img,false,notifier); }
}

#endif // IMAGINABLE__TOOLS_COLOURSPACE__INCLUDED
