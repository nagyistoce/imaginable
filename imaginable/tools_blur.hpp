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
	void box_blur(Image& img,unsigned plane,const size_t& radius,bool use_alpha=false);
	inline void box_blur_alpha(Image& img,unsigned plane,const size_t& radius)
		{ box_blur(img,plane,radius,true); }
#if 0
	Image::pixel* box_blur(Image& img,unsigned src_plane,unsigned blurred_plane,const size_t& radius,bool use_alpha);

	inline Image::pixel* box_blur(Image& img,unsigned plane,const size_t& radius,bool use_alpha)
	{
		Image::pixel* ret=box_blur(img,plane,Image::PLANE__INTERNAL,radius,use_alpha);
		img.removePlane(plane);
		img.renamePlane(Image::PLANE__INTERNAL,plane);
		return ret;
	}
#endif
}

#endif // IMAGINABLE__TOOLS_BLUR__INCLUDED
