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


#include <boost/array.hpp>

#include <vector>

#include "exception.hpp"
#include "gamma.hpp"
#include "tonemap.hpp"


namespace imaginable {


static const Image::Pixel HDRI_MAXIMUM = Image::MAXIMUM;
static const Image::Pixel LDRI_MAXIMUM = static_cast<uint8_t>(-1);
static const size_t TABLE_POWER = 2;
static const size_t TABLE_SIZE = 0x100 * (1<<TABLE_POWER);

void tonemap_global(Image& img, double colour_gamma, double lightness_factor, const Progress_notifier &notifier)
{
	if(!img.hasData())
		throw exception(exception::NO_IMAGE);

//	if(img.maximum()<=LDRI_MAXIMUM)
//		throw exception(exception::NOT_AN_HDRI);

	if(img.colourSpace()!=Image::COLOURSPACE_HCY)
		throw exception(exception::INVALID_COLOUR_SPACE);


	size_t size=img.width()*img.height();

	double scale=static_cast<double>(LDRI_MAXIMUM)/static_cast<double>(img.maximum());
	double scale4=scale*static_cast<double>(1<<TABLE_POWER);
	{
		Image::Pixel* alpha=img.plane(Image::PLANE_ALPHA);
		Image::Pixel* luma =img.plane(Image::PLANE_LUMA);

		boost::array<size_t, TABLE_SIZE> histo = { { 0 } };
		boost::array<double, TABLE_SIZE> curve = { { 0. } };
		size_t avg=0;

		if(!alpha)
		{
			if(img.maximum()==HDRI_MAXIMUM)
			{
				for(size_t p=0;p<size;++p)
				{
					notifier.update(0.25*static_cast<double>(p)/static_cast<double>(size));

					++histo[luma[p]>>(8-TABLE_POWER)];
					++avg;
				}
			}
			else
			{
				for(size_t p=0;p<size;++p)
				{
					notifier.update(0.25*static_cast<double>(p)/static_cast<double>(size));

					++histo[static_cast<size_t>(scale4*static_cast<double>(luma[p]))];
					++avg;
				}
			}
		}
		else
		{
			if(img.maximum()==HDRI_MAXIMUM)
			{
				for(size_t p=0;p<size;++p)
				{
					notifier.update(0.25*static_cast<double>(p)/static_cast<double>(size));

					if(alpha[p])
					{
						++histo[luma[p]>>(8-TABLE_POWER)];
						++avg;
					}
				}
			}
			else
			{
				for(size_t p=0;p<size;++p)
				{
					notifier.update(0.25*static_cast<double>(p)/static_cast<double>(size));

					if(alpha[p])
					{
						++histo[static_cast<size_t>(scale4*static_cast<double>(luma[p]))];
						++avg;
					}
				}
			}
		}
		avg/=TABLE_SIZE;

		double fp1=1.+lightness_factor;
		double fm1=1./fp1;
		//apply factor
		for(size_t i=0;i<curve.size();++i)
			curve[i]=(histo[i]>avg)?fp1:fm1;

		for(size_t i=1;i<curve.size();++i)
			curve[i]+=curve[i-1];

		double curve_scale=static_cast<double>(curve.size()-1)/curve[curve.size()-1];
		for(size_t i=0;i<curve.size();++i)
			curve[i]*=curve_scale;

		{//blur
			double sum=0.;
			size_t half_range=3;
			size_t mem_range=half_range+1;
			size_t full_range=mem_range+half_range;
			std::vector<double> acc;
			acc.reserve(full_range);
			for(size_t i=0;i<half_range;++i)
			{
				acc.push_back(0.);
				sum+=curve[i];
			}
			size_t count=half_range;
			acc.push_back(0.);
			for(size_t i=0;i<curve.size();++i)
			{
				acc[i%mem_range]=curve[i];
				if(i<curve.size()-half_range)
				{
					sum+=curve[i+half_range];
					++count;
				}
				if(i>=half_range)
				{
					sum-=acc[(i-half_range)%mem_range];
					--count;
				}
				curve[i]=sum/static_cast<double>(count);
			}
		}

		boost::array<Image::Pixel, TABLE_SIZE> table;

		for(size_t i=0;i<curve.size();++i)
		{
			double value=curve[i]/(1<<TABLE_POWER);
			if(value>static_cast<double>(LDRI_MAXIMUM))
				value=static_cast<double>(LDRI_MAXIMUM);
			else if(value<0.)
				value=0.;
			table[i]=static_cast<Image::Pixel>(value);
		}

		for(size_t p=0;p<size;++p)
		{
			notifier.update(0.25+0.25*static_cast<double>(p)/static_cast<double>(size));

			luma[p]=table[static_cast<size_t>(scale4*static_cast<double>(luma[p]))];
		}
	}

	{
		Image::Pixel* chroma=img.plane(Image::PLANE_CHROMA);

		boost::array<Image::Pixel, TABLE_SIZE> curve = { { 0 } };

		for(size_t i=0;i<curve.size();++i)
		{
			double value=static_cast<double>(LDRI_MAXIMUM)*gamma(static_cast<double>(i)/static_cast<double>(TABLE_SIZE), colour_gamma);
			if(value>static_cast<double>(LDRI_MAXIMUM))
				value=static_cast<double>(LDRI_MAXIMUM);
			else if(value<0.)
				value=0.;
			curve[i]=static_cast<Image::Pixel>(value);
		}

		for(size_t p=0;p<size;++p)
		{
			notifier.update(0.5+0.25*static_cast<double>(p)/static_cast<double>(size));

			chroma[p]=curve[static_cast<size_t>(scale4*static_cast<double>(chroma[p]))];
		}
	}

	{
		Image::Pixel* hue=img.plane(Image::PLANE_HUE);

		if(img.maximum()==HDRI_MAXIMUM)
			for(size_t p=0;p<size;++p)
			{
				notifier.update(0.75+0.25*static_cast<double>(p)/static_cast<double>(size));

				hue[p]>>=8;
			}
		else
			for(size_t p=0;p<size;++p)
			{
				notifier.update(0.75+0.25*static_cast<double>(p)/static_cast<double>(size));

				hue[p]=static_cast<Image::Pixel>(scale*static_cast<double>(hue[p]));
			}
	}

	img.setMaximum(LDRI_MAXIMUM);
}

}
