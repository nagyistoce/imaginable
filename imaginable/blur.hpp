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


#ifndef IMAGINABLE__TOOLS_BLUR__INCLUDED
#define IMAGINABLE__TOOLS_BLUR__INCLUDED


#include "tools.hpp"


namespace imaginable
{
	template <typename PIXEL, typename ALPHA_PIXEL>
		void universal_box_blur(PIXEL *plane,ssize_t width,ssize_t height,size_t radius,ALPHA_PIXEL *alpha,progress_notifier notifier);

	template <typename PIXEL, typename ALPHA_PIXEL>
		void universal_gaussian_blur(PIXEL *plane,ssize_t width,ssize_t height,size_t radius,ALPHA_PIXEL *alpha,progress_notifier notifier);


	void box_blur(Image& img,unsigned plane,size_t radius,bool use_alpha=false,progress_notifier notifier = dont_notify);

	inline void box_blur(Image& img,unsigned plane,size_t radius,progress_notifier notifier)
		{ box_blur(img,plane,radius,false,notifier); }

	inline void box_blur_alpha(Image& img,unsigned plane,size_t radius,progress_notifier notifier = dont_notify)
		{ box_blur(img,plane,radius,true,notifier); }


	void gaussian_blur(Image& img,unsigned plane,size_t radius,bool use_alpha=false,progress_notifier notifier = dont_notify);

	inline void gaussian_blur(Image& img,unsigned plane,size_t radius,progress_notifier notifier)
		{ gaussian_blur(img,plane,radius,false,notifier); }

	inline void gaussian_blur_alpha(Image& img,unsigned plane,size_t radius,progress_notifier notifier = dont_notify)
		{ gaussian_blur(img,plane,radius,true,notifier); }

#if 0
	Image::Pixel* box_blur(Image& img,unsigned src_plane,unsigned blurred_plane,size_t radius,bool use_alpha);

	inline Image::Pixel* box_blur(Image& img,unsigned plane,size_t radius,bool use_alpha)
	{
		Image::Pixel* ret=box_blur(img,plane,Image::PLANE__INTERNAL,radius,use_alpha);
		img.removePlane(plane);
		img.renamePlane(Image::PLANE__INTERNAL,plane);
		return ret;
	}
#endif
}

#endif // IMAGINABLE__TOOLS_BLUR__INCLUDED
