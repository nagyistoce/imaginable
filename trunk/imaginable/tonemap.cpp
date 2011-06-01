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
#include <boost/bind.hpp>

#include <cmath>
#include <vector>
#include <cstring>

#include "gamma.hpp"
#include "blur.hpp"
#include "tonemap.hpp"


namespace imaginable {


static const Image::pixel HDRI_MAXIMUM=Image::MAXIMUM;
static const Image::pixel LDRI_MAXIMUM=static_cast<uint8_t>(-1);
static const size_t TABLE_POWER=2;
static const size_t TABLE_SIZE=0x100 * (1<<TABLE_POWER);

void tonemap_global(Image& img,double saturation_gamma,double lightness_factor,progress_notifier notifier)
{
	if(!img.hasData())
		throw exception(exception::NO_IMAGE);

	if(img.maximum()<=LDRI_MAXIMUM)
		throw exception(exception::NOT_AN_HDRI);

	if(img.colourSpace()!=Image::IMAGE_HSL)
		throw exception(exception::INVALID_COLOUR_SPACE);


	size_t size=img.width()*img.height();

	double scale=static_cast<double>(LDRI_MAXIMUM)/static_cast<double>(img.maximum());
	double scale4=scale*static_cast<double>(1<<TABLE_POWER);
	{
		Image::pixel* alpha     =img.plane(Image::PLANE_ALPHA);
		Image::pixel* lightness =img.plane(Image::PLANE_HSL_LIGHTNESS);

		boost::array<size_t,TABLE_SIZE> histo = { { 0 } };
		boost::array<double,TABLE_SIZE> curve = { { 0. } };
		size_t avg=0;

		if(!alpha)
		{
			if(img.maximum()==HDRI_MAXIMUM)
			{
				for(size_t p=0;p<size;++p)
				{
					notifier(0.25*static_cast<float>(p)/static_cast<float>(size));

					++histo[lightness[p]>>(8-TABLE_POWER)];
					++avg;
				}
			}
			else
			{
				for(size_t p=0;p<size;++p)
				{
					notifier(0.25*static_cast<float>(p)/static_cast<float>(size));

					++histo[static_cast<size_t>(scale4*static_cast<double>(lightness[p]))];
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
					notifier(0.25*static_cast<float>(p)/static_cast<float>(size));

					if(alpha[p])
					{
						++histo[lightness[p]>>(8-TABLE_POWER)];
						++avg;
					}
				}
			}
			else
			{
				for(size_t p=0;p<size;++p)
				{
					notifier(0.25*static_cast<float>(p)/static_cast<float>(size));

					if(alpha[p])
					{
						++histo[static_cast<size_t>(scale4*static_cast<double>(lightness[p]))];
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

		boost::array<Image::pixel,TABLE_SIZE> table;

		for(size_t i=0;i<curve.size();++i)
		{
			double value=curve[i]/(1<<TABLE_POWER);
			if(value>static_cast<double>(LDRI_MAXIMUM))
				value=static_cast<double>(LDRI_MAXIMUM);
			else if(value<0.)
				value=0.;
			table[i]=static_cast<Image::pixel>(value);
		}

		for(size_t p=0;p<size;++p)
		{
			notifier(0.25+0.25*static_cast<float>(p)/static_cast<float>(size));

			lightness[p]=table[static_cast<size_t>(scale4*static_cast<double>(lightness[p]))];
		}
	}

	{
		Image::pixel* saturation=img.plane(Image::PLANE_HSL_SATURATION);

		boost::array<Image::pixel,TABLE_SIZE> curve = { { 0 } };

		for(size_t i=0;i<curve.size();++i)
		{
			double value=static_cast<double>(LDRI_MAXIMUM)*gamma(static_cast<double>(i)/static_cast<double>(TABLE_SIZE),saturation_gamma);
			if(value>static_cast<double>(LDRI_MAXIMUM))
				value=static_cast<double>(LDRI_MAXIMUM);
			else if(value<0.)
				value=0.;
			curve[i]=static_cast<Image::pixel>(value);
		}

		for(size_t p=0;p<size;++p)
		{
			notifier(0.5+0.25*static_cast<float>(p)/static_cast<float>(size));

			saturation[p]=curve[static_cast<size_t>(scale4*static_cast<double>(saturation[p]))];
		}
	}

	{
		Image::pixel* hue=img.plane(Image::PLANE_HUE);

		if(img.maximum()==HDRI_MAXIMUM)
			for(size_t p=0;p<size;++p)
			{
				notifier(0.75+0.25*static_cast<float>(p)/static_cast<float>(size));

				hue[p]>>=8;
			}
		else
			for(size_t p=0;p<size;++p)
			{
				notifier(0.75+0.25*static_cast<float>(p)/static_cast<float>(size));

				hue[p]=static_cast<Image::pixel>(scale*static_cast<double>(hue[p]));
			}
	}

	img.setMaximum(LDRI_MAXIMUM);
}

enum
{
	IMAGE__PLANE__BLURRED_LIGHTNESS=Image::PLANE__USER
};

#define NOTIFY_STEP 50

void tonemap_local(Image& img,double saturation_gamma,size_t blur_size,double mix_factor,progress_notifier notifier)
{
	if (!img.hasData())
		throw exception(exception::NO_IMAGE);

//	if(img.maximum()<=LDRI_MAXIMUM)
//		throw exception(exception::NOT_AN_HDRI);

	if (img.colourSpace() != Image::IMAGE_HSL)
		throw exception(exception::INVALID_COLOUR_SPACE);

	size_t size = img.width() * img.height();

	{
		Image::pixel *alpha=img.plane(Image::PLANE_ALPHA);
		Image::pixel *lightness=img.plane(Image::PLANE_HSL_LIGHTNESS);
		img.addPlane(IMAGE__PLANE__BLURRED_LIGHTNESS);
		Image::pixel *blurred_lightness=img.plane(IMAGE__PLANE__BLURRED_LIGHTNESS);

		memcpy(blurred_lightness,lightness,size*sizeof(Image::pixel));

		//blur lightness2
		/* */
		size_t blur_size_3 = blur_size/3;
		if (blur_size_3)
			box_blur(img,IMAGE__PLANE__BLURRED_LIGHTNESS,blur_size_3,alpha,boost::bind(&scaled_notifier,notifier,0.        ,0.25/3.,_1));
		if (blur_size_3)
			box_blur(img,IMAGE__PLANE__BLURRED_LIGHTNESS,blur_size_3,alpha,boost::bind(&scaled_notifier,notifier,1./3.*0.25,0.25/3.,_1));
		blur_size_3 = blur_size-2*(blur_size_3);
		if (blur_size_3)
			box_blur(img,IMAGE__PLANE__BLURRED_LIGHTNESS,blur_size_3,alpha,boost::bind(&scaled_notifier,notifier,2./3.*0.25,0.25/3.,_1));
		/*/
		box_blur(img,IMAGE__PLANE__BLURRED_LIGHTNESS,blur_size,alpha,boost::bind(&scaled_notifier,notifier,0.,0.25,_1));
		/ * */
		notifier(1*0.25);

		//invert blurred lightness2
		for (size_t p=0; p<size; ++p)
		{
			if (!(p%NOTIFY_STEP))
				notifier(  0.25+0.25*static_cast<float>(p)/static_cast<float>(size));

			blurred_lightness[p] = img.maximum()-blurred_lightness[p];
		}
		notifier(2*0.25);

		//int64_t avg_blur=0;
		//size_t avg_blur_count=0;
//		int64_t median=img.maximum()/2;

		Image::pixel edge_low  = HDRI_MAXIMUM;
		Image::pixel edge_high = 0;
		//mix them
		for (size_t p=0; p<size; ++p)
		{
			if (!(p%NOTIFY_STEP))
				notifier(2*0.25+0.25*static_cast<float>(p)/static_cast<float>(size));

			if (!alpha || alpha[p])
			{
				//avg_blur += abs(static_cast<int64_t>(blurred_lightness[p])-static_cast<int64_t>(lightness[p]));
				//++avg_blur_count;
				Image::pixel value = static_cast<Image::pixel>(
					(1.-mix_factor)*static_cast<double>(lightness[p])+
					mix_factor     *static_cast<double>(blurred_lightness[p]) );
				edge_low  = std::min(edge_low,value);
				edge_high = std::max(edge_high,value);
				lightness[p] = value;
			}
		}
		//avg_blur *= 2./avg_blur_count;
//std::cout<<"edges: "<<edge_low<<" .. "<<edge_high;
		//edge_high = (HDRI_MAXIMUM - edge_high);
//std::cout<<" ("<<edge_high<<")"<<std::endl<<std::flush;
		//Image::pixel avg_edge=std::min(edge_low,static_cast<Image::pixel>(HDRI_MAXIMUM-edge_high));
//std::cout<<"avg_blur = "<<avg_blur<<std::endl<<std::flush;
		double shift_l = edge_low;//avg_edge;
		double scale_l = static_cast<double>(LDRI_MAXIMUM)/(edge_high-edge_low/*static_cast<double>(img.maximum())-edge_low-edge_high*//*2.*avg_edge*/);
		notifier(3*0.25);

		img.removePlane(IMAGE__PLANE__BLURRED_LIGHTNESS);
//		img.removePlane(Image::PLANE_HSL_SATURATION);
//		img.renamePlane(IMAGE__PLANE__BLURRED_LIGHTNESS,Image::PLANE_HSL_SATURATION);

//		double shift_l=median-avg_blur;
//		double scale_l=static_cast<double>(LDRI_MAXIMUM)/static_cast<double>(2*avg_blur);
//		shift_l=0.;
//		scale_l=static_cast<double>(LDRI_MAXIMUM)/(static_cast<double>(img.maximum())-2.*shift_l);

//std::cout<<"Compression = "<<(256.*scale_l)<<std::endl<<std::flush;

		if (size >= HDRI_MAXIMUM)
		{
			Image::pixel lightness_map[HDRI_MAXIMUM+1];
			for (size_t p=0; p<HDRI_MAXIMUM+1; ++p)
			{
				double value = scale_l*(static_cast<double>(p)-shift_l);
				if (value > static_cast<double>(LDRI_MAXIMUM))
					value = static_cast<double>(LDRI_MAXIMUM);
				else if (value < 0.)
					value = 0.;
				lightness_map[p] = static_cast<Image::pixel>(value);
			}
			for (size_t p=0; p<size; ++p)
				lightness[p] = lightness_map[lightness[p]];
		}
		else
			for (size_t p=0; p<size; ++p)
			{
				double value = scale_l*(static_cast<double>(lightness[p])-shift_l);
				if (value > static_cast<double>(LDRI_MAXIMUM))
					value = static_cast<double>(LDRI_MAXIMUM);
				else if (value < 0.)
					value = 0.;
				lightness[p] = static_cast<Image::pixel>(value);
			}
	}

	double scale=static_cast<double>(LDRI_MAXIMUM)/static_cast<double>(img.maximum());
	{
		Image::pixel saturation_map[HDRI_MAXIMUM+1];
		for (size_t p=0; p<HDRI_MAXIMUM+1; ++p)
		{
			double value = static_cast<double>(LDRI_MAXIMUM)*gamma(static_cast<double>(p)/static_cast<double>(img.maximum()),saturation_gamma);
			if (value > static_cast<double>(LDRI_MAXIMUM))
				value = static_cast<double>(LDRI_MAXIMUM);
			else if (value < 0.)
				value = 0.;
			saturation_map[p] = value;
		}

		Image::pixel *saturation=img.plane(Image::PLANE_HSL_SATURATION);
		for (size_t p=0; p<size; ++p)
			saturation[p] = saturation_map[saturation[p]];
	}

	{
		Image::pixel *hue=img.plane(Image::PLANE_HUE);

		if (img.maximum() == HDRI_MAXIMUM)
			for (size_t p=0; p<size; ++p)
			{
				if (!(p%NOTIFY_STEP))
					notifier(3*0.25+0.25*static_cast<float>(p)/static_cast<float>(size));

				hue[p] >>= 8;
			}
		else if (img.maximum() != LDRI_MAXIMUM)
			for (size_t p=0; p<size; ++p)
			{
				if (!(p%NOTIFY_STEP))
					notifier(3*0.25+0.25*static_cast<float>(p)/static_cast<float>(size));

				hue[p] = static_cast<Image::pixel>(scale*static_cast<double>(hue[p]));
			}
		notifier(1.);
	}

	img.setMaximum(LDRI_MAXIMUM);
}

}
