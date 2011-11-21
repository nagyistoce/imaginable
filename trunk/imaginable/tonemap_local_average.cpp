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


#include <cstring>

#include "exception.hpp"
#include "gamma.hpp"
#include "blur.hpp"
#include "tonemap.hpp"


namespace imaginable {


static const Image::Pixel HDRI_MAXIMUM=Image::MAXIMUM;
static const Image::Pixel LDRI_MAXIMUM=static_cast<uint8_t>(-1);

enum
{
	IMAGE__PLANE__BLURRED_LUMA=Image::PLANE__USER
};

void tonemap_local_average(Image& img, double colour_gamma, size_t blur_size, double mix_factor, const Progress_notifier &notifier)
{
	if (!img.hasData())
		throw exception(exception::NO_IMAGE);

//	if(img.maximum()<=LDRI_MAXIMUM)
//		throw exception(exception::NOT_AN_HDRI);

	if (img.colourSpace() != Image::COLOURSPACE_HCY)
		throw exception(exception::INVALID_COLOUR_SPACE);

	size_t width = img.width();
	size_t size = img.width() * img.height();

	{
		Image::Pixel *alpha=img.plane(Image::PLANE_ALPHA);
		Image::Pixel *luma=img.plane(Image::PLANE_LUMA);
		img.addPlane(IMAGE__PLANE__BLURRED_LUMA);
		Image::Pixel *blurred_luma=img.plane(IMAGE__PLANE__BLURRED_LUMA);

		memcpy(blurred_luma, luma, size*sizeof(Image::Pixel));

		//blur lightness2
		gaussian_blur_alpha(img, IMAGE__PLANE__BLURRED_LUMA, blur_size, Scaled_progress_notifier(notifier, 0., 0.25));

		//invert blurred lightness2
		for (size_t p=0; p<size; ++p)
		{
			if (!(p%width))
				notifier.update(  0.25+0.25*static_cast<double>(p)/static_cast<double>(size));

			blurred_luma[p] = img.maximum()-blurred_luma[p];
		}
		notifier.update(2*0.25);

		//int64_t avg_blur=0;
		//size_t avg_blur_count=0;
//		int64_t median=img.maximum()/2;

		Image::Pixel edge_low  = HDRI_MAXIMUM;
		Image::Pixel edge_high = 0;
		//mix them
		for (size_t p=0; p<size; ++p)
		{
			if (!(p%width))
				notifier.update(2*0.25+0.25*static_cast<double>(p)/static_cast<double>(size));

			if (!alpha || alpha[p])
			{
				//avg_blur += abs(static_cast<int64_t>(blurred_luma[p])-static_cast<int64_t>(luma[p]));
				//++avg_blur_count;
				Image::Pixel value = static_cast<Image::Pixel>(
					(1.-mix_factor)*static_cast<double>(luma[p])+
					mix_factor     *static_cast<double>(blurred_luma[p]) );
				edge_low  = std::min(edge_low, value);
				edge_high = std::max(edge_high, value);
				luma[p] = value;
			}
		}
		//avg_blur *= 2./avg_blur_count;
//std::cout<<"edges: "<<edge_low<<" .. "<<edge_high;
		//edge_high = (HDRI_MAXIMUM - edge_high);
//std::cout<<" ("<<edge_high<<")"<<std::endl<<std::flush;
		//Image::Pixel avg_edge=std::min(edge_low, static_cast<Image::Pixel>(HDRI_MAXIMUM-edge_high));
//std::cout<<"avg_blur = "<<avg_blur<<std::endl<<std::flush;
		double shift_l = edge_low;//avg_edge;
		double scale_l = static_cast<double>(LDRI_MAXIMUM)/(edge_high-edge_low/*static_cast<double>(img.maximum())-edge_low-edge_high*//*2.*avg_edge*/);
		notifier.update(3*0.25);

		img.removePlane(IMAGE__PLANE__BLURRED_LUMA);

//		double shift_l=median-avg_blur;
//		double scale_l=static_cast<double>(LDRI_MAXIMUM)/static_cast<double>(2*avg_blur);
//		shift_l=0.;
//		scale_l=static_cast<double>(LDRI_MAXIMUM)/(static_cast<double>(img.maximum())-2.*shift_l);

//std::cout<<"Compression = "<<(256.*scale_l)<<std::endl<<std::flush;

		if (size >= HDRI_MAXIMUM)
		{
			Image::Pixel luma_map[HDRI_MAXIMUM+1];
			for (size_t p=0; p<HDRI_MAXIMUM+1; ++p)
			{
				double value = scale_l*(static_cast<double>(p)-shift_l);
				if (value > static_cast<double>(LDRI_MAXIMUM))
					value = static_cast<double>(LDRI_MAXIMUM);
				else if (value < 0.)
					value = 0.;
				luma_map[p] = static_cast<Image::Pixel>(value);
			}
			for (size_t p=0; p<size; ++p)
				luma[p] = luma_map[luma[p]];
		}
		else
			for (size_t p=0; p<size; ++p)
			{
				double value = scale_l*(static_cast<double>(luma[p])-shift_l);
				if (value > static_cast<double>(LDRI_MAXIMUM))
					value = static_cast<double>(LDRI_MAXIMUM);
				else if (value < 0.)
					value = 0.;
				luma[p] = static_cast<Image::Pixel>(value);
			}
	}

	double scale=static_cast<double>(LDRI_MAXIMUM)/static_cast<double>(img.maximum());
	{
		Image::Pixel chroma_map[HDRI_MAXIMUM+1];
		for (size_t p=0; p<HDRI_MAXIMUM+1; ++p)
		{
			double value = static_cast<double>(LDRI_MAXIMUM)*gamma(static_cast<double>(p)/static_cast<double>(img.maximum()), colour_gamma);
			if (value > static_cast<double>(LDRI_MAXIMUM))
				value = static_cast<double>(LDRI_MAXIMUM);
			else if (value < 0.)
				value = 0.;
			chroma_map[p] = value;
		}

		Image::Pixel *chroma=img.plane(Image::PLANE_CHROMA);
		for (size_t p=0; p<size; ++p)
			chroma[p] = chroma_map[chroma[p]];
	}

	{
		Image::Pixel *hue=img.plane(Image::PLANE_HUE);

		if (img.maximum() == HDRI_MAXIMUM)
			for (size_t p=0; p<size; ++p)
			{
				if (!(p%width))
					notifier.update(3*0.25+0.25*static_cast<double>(p)/static_cast<double>(size));

				hue[p] >>= 8;
			}
		else if (img.maximum() != LDRI_MAXIMUM)
			for (size_t p=0; p<size; ++p)
			{
				if (!(p%width))
					notifier.update(3*0.25+0.25*static_cast<double>(p)/static_cast<double>(size));

				hue[p] = static_cast<Image::Pixel>(scale*static_cast<double>(hue[p]));
			}
		notifier.update(1.);
	}

	img.setMaximum(LDRI_MAXIMUM);
}

}
