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


#include <cmath>
#include <cstring>

#include "exception.hpp"
#include "gamma.hpp"
#include "blur.hpp"
#include "tonemap.hpp"
#include "roof.hpp"


namespace imaginable {


static const Image::Pixel HDRI_MAXIMUM=Image::MAXIMUM;
static const Image::Pixel LDRI_MAXIMUM=static_cast<uint8_t>(-1);


enum
{
	IMAGE__PLANE__MIN_LUMA=Image::PLANE__USER,
	IMAGE__PLANE__MAX_LUMA,
	IMAGE__PLANE__MIDDLE_LUMA,
	IMAGE__PLANE__RANGE_LUMA,
	IMAGE__PLANE__ALPHA_BACKUP
};

typedef struct Minmax_roof_parameters
{
	enum {
		FUNCTION_PARABOLIC,
		FUNCTION_EXPONENTIAL
	} function;
	double exponential_factor;
	size_t minmax_size;
}Minmax_roof_parameters;

void tonemap_local_minmax(Image& img, double colour_gamma, const Minmax_roof_parameters &minmax_roof_parameters, size_t blur_size, double min_range_factor, const Progress_notifier &notifier)
{
	if (!img.hasData())
		throw exception(exception::NO_IMAGE);

//	if(img.maximum()<=LDRI_MAXIMUM)
//		throw exception(exception::NOT_AN_HDRI);

	if (img.colourSpace() != Image::COLOURSPACE_HCY)
		throw exception(exception::INVALID_COLOUR_SPACE);

	if ((min_range_factor <= 0.) || (min_range_factor > 1.))
		throw exception(exception::INVALID_DATA);

//	Image::Pixel *alpha=img.plane(Image::PLANE_ALPHA);

	size_t w = img.width();
	size_t h = img.height();
	size_t square = w*h;


	if (img.maximum() != HDRI_MAXIMUM)
	{
		double scale=static_cast<double>(HDRI_MAXIMUM)/static_cast<double>(img.maximum());
		{
			Image::PlaneNames planes = img.planeNames();
			size_t plane_count = planes.size();
			for (Image::PlaneNames::iterator I=planes.begin(); I!=planes.end(); ++I)
			{
				Image::Pixel *plane=img.plane(*I);

				if (img.maximum() == LDRI_MAXIMUM)
					for (size_t p=0; p<square; ++p)
					{
						if (!(p % w))
							notifier.update(0+1./6.*static_cast<double>(p)/static_cast<double>(square)/plane_count);

						plane[p] <<= 8;
					}
				else
					for (size_t p=0; p<square; ++p)
					{
						if (!(p % w))
							notifier.update(0+1./6.*static_cast<double>(p)/static_cast<double>(square)/plane_count);

						plane[p] = static_cast<Image::Pixel>(static_cast<double>(plane[p])*scale);
					}
			}
			notifier.update(1.);
		}
		img.setMaximum(HDRI_MAXIMUM);
	}

	img.addPlane(IMAGE__PLANE__MIN_LUMA);
	img.addPlane(IMAGE__PLANE__MAX_LUMA);

//	minmax(img, Image::PLANE_LUMA, IMAGE__PLANE__MIN_LUMA, IMAGE__PLANE__MAX_LUMA, blur_size, notifier);

	Image::Pixel *maxPlane=img.plane(IMAGE__PLANE__MAX_LUMA);
	Image::Pixel *minPlane=img.plane(IMAGE__PLANE__MIN_LUMA);
	Image::Pixel *luma=img.plane(Image::PLANE_LUMA);

	memcpy(minPlane, luma, square*sizeof(Image::Pixel));
	memcpy(maxPlane, luma, square*sizeof(Image::Pixel));

	switch (minmax_roof_parameters.function)
	{
	case Minmax_roof_parameters::FUNCTION_PARABOLIC:
		roof_parabolic(img, IMAGE__PLANE__MAX_LUMA,  static_cast<double>(minmax_roof_parameters.minmax_size)/sqrt(static_cast<double>(img.maximum())), Scaled_progress_notifier(notifier, 1./6., 1./6.));
		roof_parabolic(img, IMAGE__PLANE__MIN_LUMA, -static_cast<double>(minmax_roof_parameters.minmax_size)/sqrt(static_cast<double>(img.maximum())), Scaled_progress_notifier(notifier, 2./6., 1./6.));
		break;
	case Minmax_roof_parameters::FUNCTION_EXPONENTIAL:
		roof_exponential(img, IMAGE__PLANE__MAX_LUMA,  minmax_roof_parameters.exponential_factor, minmax_roof_parameters.minmax_size, Scaled_progress_notifier(notifier, 1./6., 1./6.));
		roof_exponential(img, IMAGE__PLANE__MIN_LUMA, -minmax_roof_parameters.exponential_factor, minmax_roof_parameters.minmax_size, Scaled_progress_notifier(notifier, 2./6., 1./6.));
		break;
	}


	/* */
	img.addPlane(IMAGE__PLANE__MIDDLE_LUMA);
	img.addPlane(IMAGE__PLANE__RANGE_LUMA);
	Image::Pixel *middlePlane=img.plane(IMAGE__PLANE__MIDDLE_LUMA);
	Image::Pixel *rangePlane=img.plane(IMAGE__PLANE__RANGE_LUMA);
	for (size_t y=0; y<h; ++y)
	{
		notifier.update(3./6.+1./6.*static_cast<double>(y)/static_cast<double>(h));

		size_t yo=y*w;
		for (size_t x=0; x<w; ++x)
		{
			size_t yxo = yo+x;
			middlePlane[yxo] = static_cast<Image::Pixel>( (static_cast<uint32_t>(maxPlane[yxo]) + static_cast<uint32_t>(minPlane[yxo])) / 2);
			rangePlane[yxo] = maxPlane[yxo] - minPlane[yxo];
		}
	}

	bool alpha_present = img.hasPlane(Image::PLANE_ALPHA);
	if (alpha_present)
		img.renamePlane(Image::PLANE_ALPHA, IMAGE__PLANE__ALPHA_BACKUP);
	img.renamePlane(IMAGE__PLANE__RANGE_LUMA, Image::PLANE_ALPHA);
	gaussian_blur_alpha(img, IMAGE__PLANE__MIDDLE_LUMA, blur_size);
	img.removePlane(Image::PLANE_ALPHA);
	img.removePlane(IMAGE__PLANE__RANGE_LUMA);
	if (alpha_present)
		img.renamePlane(IMAGE__PLANE__ALPHA_BACKUP, Image::PLANE_ALPHA);
	for (size_t y=0; y<h; ++y)
	{
		notifier.update(4./6.+1./6.*static_cast<double>(y)/static_cast<double>(h));

		size_t yo=y*w;
		for (size_t x=0; x<w; ++x)
		{
			size_t yxo = yo+x;

			Image::Pixel middlePixel = middlePlane[yxo];
			ssize_t bigger_delta = static_cast<ssize_t>(maxPlane[yxo] - middlePixel) - static_cast<ssize_t>(middlePixel - minPlane[yxo]);
			if (bigger_delta > 0)
			{
				ssize_t minPixel = 2*static_cast<ssize_t>(middlePixel) - maxPlane[yxo];
				if (minPixel < 0)
					minPixel = 0;
				minPlane[yxo] = static_cast<Image::Pixel>(minPixel);
			}
			else if (bigger_delta < 0)
			{
				ssize_t maxPixel = 2*static_cast<ssize_t>(middlePixel) - minPlane[yxo];
				if (maxPixel > 0xffff)
					maxPixel = 0xffff;
				maxPlane[yxo] = static_cast<Image::Pixel>(maxPixel);
			}

			uint32_t delta = maxPlane[yxo] - minPlane[yxo];
			middlePixel = minPlane[yxo] + delta/2;
			if (delta < HDRI_MAXIMUM * min_range_factor)
			{
				delta = HDRI_MAXIMUM * min_range_factor;
				ssize_t minAdjusted = static_cast<ssize_t>(middlePixel) - static_cast<ssize_t>(delta)/2;
				ssize_t maxAdjusted = static_cast<ssize_t>(middlePixel) + static_cast<ssize_t>(delta)/2;
				if (minAdjusted < 0)
				{
					maxAdjusted -= minAdjusted;
					minAdjusted = 0;
				}
				else if (maxAdjusted > HDRI_MAXIMUM)
				{
					minAdjusted -= maxAdjusted-HDRI_MAXIMUM;
					maxAdjusted = HDRI_MAXIMUM;
				}
				minPlane[yxo] = minAdjusted;
				maxPlane[yxo] = maxAdjusted;
			}
			luma[yxo] = static_cast<Image::Pixel>((static_cast<uint32_t>(luma[yxo] - minPlane[yxo]) * HDRI_MAXIMUM ) / delta);
		}
	}
	img.removePlane(IMAGE__PLANE__MIDDLE_LUMA);
	/* */

	/* */
	{
		Image::Pixel chroma_map[HDRI_MAXIMUM+1];
		for (size_t p=0; p<HDRI_MAXIMUM+1; ++p)
		{
			double value = gamma(static_cast<double>(p)/static_cast<double>(HDRI_MAXIMUM), colour_gamma);
			chroma_map[p] =
				(value > 1.) ? HDRI_MAXIMUM :
				( (value < 0.) ? 0 :
				static_cast<Image::Pixel>(HDRI_MAXIMUM * value) );
		}

		Image::Pixel *chroma=img.plane(Image::PLANE_CHROMA);
		for (size_t p=0; p<square; ++p)
		{
			if (!(p % w))
				notifier.update(5./6.+1./6.*static_cast<double>(p)/static_cast<double>(square));

			chroma[p] = chroma_map[chroma[p]];
		}
	}
	/* */

/*
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
				if (!(p%w))
					notifier.update(3*0.25+0.25*static_cast<double>(p)/static_cast<double>(size));

				hue[p] >>= 8;
			}
		else if (img.maximum() != LDRI_MAXIMUM)
			for (size_t p=0; p<size; ++p)
			{
				if (!(p%w))
					notifier.update(3*0.25+0.25*static_cast<double>(p)/static_cast<double>(size));

				hue[p] = static_cast<Image::Pixel>(scale*static_cast<double>(hue[p]));
			}
		notifier.update(1.);
	}

	img.setMaximum(LDRI_MAXIMUM);
*/

	/* */
	img.removePlane(IMAGE__PLANE__MAX_LUMA);
	img.removePlane(IMAGE__PLANE__MIN_LUMA);
	/*/
	img.removePlane(Image::PLANE_HUE);
	img.removePlane(Image::PLANE_CHROMA);
	img.renamePlane(Image::PLANE_LUMA     , Image::PLANE_BLUE);
	img.renamePlane(IMAGE__PLANE__MIN_LUMA, Image::PLANE_GREEN);
	img.renamePlane(IMAGE__PLANE__MAX_LUMA, Image::PLANE_RED);
	/* */
}

void tonemap_local_minmax_parabolic(Image& img, double colour_gamma, size_t minmax_size, size_t blur_size, double min_range_factor, const Progress_notifier &notifier)
{
	Minmax_roof_parameters minmax_roof_parameters;
	minmax_roof_parameters.function = Minmax_roof_parameters::FUNCTION_PARABOLIC;
	minmax_roof_parameters.minmax_size = minmax_size;
	tonemap_local_minmax(img, colour_gamma, minmax_roof_parameters, blur_size, min_range_factor, notifier);
}

void tonemap_local_minmax_exponential(Image& img, double colour_gamma, double exponential_factor, size_t minmax_size, size_t blur_size, double min_range_factor, const Progress_notifier &notifier)
{
	Minmax_roof_parameters minmax_roof_parameters;
	minmax_roof_parameters.function = Minmax_roof_parameters::FUNCTION_EXPONENTIAL;
	minmax_roof_parameters.exponential_factor = exponential_factor;
	minmax_roof_parameters.minmax_size = minmax_size;
	tonemap_local_minmax(img, colour_gamma, minmax_roof_parameters, blur_size, min_range_factor, notifier);
}

}
