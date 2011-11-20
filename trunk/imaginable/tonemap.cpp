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


#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>

#include <boost/scoped_array.hpp>
#include <boost/shared_array.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>

#include <cmath>
#include <vector>
#include <cstring>

#include <stdio.h>
#include <iostream>

#include "gamma.hpp"
#include "blur.hpp"
#include "tonemap.hpp"
#include "roof.hpp"


namespace imaginable {


static const Image::Pixel HDRI_MAXIMUM=Image::MAXIMUM;
static const Image::Pixel LDRI_MAXIMUM=static_cast<uint8_t>(-1);
static const size_t TABLE_POWER=2;
static const size_t TABLE_SIZE=0x100 * (1<<TABLE_POWER);

void tonemap_global(Image& img,double colour_gamma,double lightness_factor,progress_notifier notifier)
{
	if(!img.hasData())
		throw exception(exception::NO_IMAGE);

	if(img.maximum()<=LDRI_MAXIMUM)
		throw exception(exception::NOT_AN_HDRI);

	if(img.colourSpace()!=Image::COLOURSPACE_HCY)
		throw exception(exception::INVALID_COLOUR_SPACE);


	size_t size=img.width()*img.height();

	double scale=static_cast<double>(LDRI_MAXIMUM)/static_cast<double>(img.maximum());
	double scale4=scale*static_cast<double>(1<<TABLE_POWER);
	{
		Image::Pixel* alpha=img.plane(Image::PLANE_ALPHA);
		Image::Pixel* luma =img.plane(Image::PLANE_LUMA);

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

					++histo[luma[p]>>(8-TABLE_POWER)];
					++avg;
				}
			}
			else
			{
				for(size_t p=0;p<size;++p)
				{
					notifier(0.25*static_cast<float>(p)/static_cast<float>(size));

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
					notifier(0.25*static_cast<float>(p)/static_cast<float>(size));

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
					notifier(0.25*static_cast<float>(p)/static_cast<float>(size));

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

		boost::array<Image::Pixel,TABLE_SIZE> table;

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
			notifier(0.25+0.25*static_cast<float>(p)/static_cast<float>(size));

			luma[p]=table[static_cast<size_t>(scale4*static_cast<double>(luma[p]))];
		}
	}

	{
		Image::Pixel* chroma=img.plane(Image::PLANE_CHROMA);

		boost::array<Image::Pixel,TABLE_SIZE> curve = { { 0 } };

		for(size_t i=0;i<curve.size();++i)
		{
			double value=static_cast<double>(LDRI_MAXIMUM)*gamma(static_cast<double>(i)/static_cast<double>(TABLE_SIZE),colour_gamma);
			if(value>static_cast<double>(LDRI_MAXIMUM))
				value=static_cast<double>(LDRI_MAXIMUM);
			else if(value<0.)
				value=0.;
			curve[i]=static_cast<Image::Pixel>(value);
		}

		for(size_t p=0;p<size;++p)
		{
			notifier(0.5+0.25*static_cast<float>(p)/static_cast<float>(size));

			chroma[p]=curve[static_cast<size_t>(scale4*static_cast<double>(chroma[p]))];
		}
	}

	{
		Image::Pixel* hue=img.plane(Image::PLANE_HUE);

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

				hue[p]=static_cast<Image::Pixel>(scale*static_cast<double>(hue[p]));
			}
	}

	img.setMaximum(LDRI_MAXIMUM);
}

enum
{
	IMAGE__PLANE__BLURRED_LUMA=Image::PLANE__USER
};

#define NOTIFY_STEP 50

void tonemap_local_average(Image& img,double colour_gamma,size_t blur_size,double mix_factor,progress_notifier notifier)
{
	if (!img.hasData())
		throw exception(exception::NO_IMAGE);

//	if(img.maximum()<=LDRI_MAXIMUM)
//		throw exception(exception::NOT_AN_HDRI);

	if (img.colourSpace() != Image::COLOURSPACE_HCY)
		throw exception(exception::INVALID_COLOUR_SPACE);

	size_t size = img.width() * img.height();

	{
		Image::Pixel *alpha=img.plane(Image::PLANE_ALPHA);
		Image::Pixel *luma=img.plane(Image::PLANE_LUMA);
		img.addPlane(IMAGE__PLANE__BLURRED_LUMA);
		Image::Pixel *blurred_luma=img.plane(IMAGE__PLANE__BLURRED_LUMA);

		memcpy(blurred_luma,luma,size*sizeof(Image::Pixel));

		//blur lightness2
		gaussian_blur_alpha(img,IMAGE__PLANE__BLURRED_LUMA,blur_size,boost::bind(&scaled_notifier,notifier,0.,0.25,_1));

		//invert blurred lightness2
		for (size_t p=0; p<size; ++p)
		{
			if (!(p%NOTIFY_STEP))
				notifier(  0.25+0.25*static_cast<float>(p)/static_cast<float>(size));

			blurred_luma[p] = img.maximum()-blurred_luma[p];
		}
		notifier(2*0.25);

		//int64_t avg_blur=0;
		//size_t avg_blur_count=0;
//		int64_t median=img.maximum()/2;

		Image::Pixel edge_low  = HDRI_MAXIMUM;
		Image::Pixel edge_high = 0;
		//mix them
		for (size_t p=0; p<size; ++p)
		{
			if (!(p%NOTIFY_STEP))
				notifier(2*0.25+0.25*static_cast<float>(p)/static_cast<float>(size));

			if (!alpha || alpha[p])
			{
				//avg_blur += abs(static_cast<int64_t>(blurred_luma[p])-static_cast<int64_t>(luma[p]));
				//++avg_blur_count;
				Image::Pixel value = static_cast<Image::Pixel>(
					(1.-mix_factor)*static_cast<double>(luma[p])+
					mix_factor     *static_cast<double>(blurred_luma[p]) );
				edge_low  = std::min(edge_low,value);
				edge_high = std::max(edge_high,value);
				luma[p] = value;
			}
		}
		//avg_blur *= 2./avg_blur_count;
//std::cout<<"edges: "<<edge_low<<" .. "<<edge_high;
		//edge_high = (HDRI_MAXIMUM - edge_high);
//std::cout<<" ("<<edge_high<<")"<<std::endl<<std::flush;
		//Image::Pixel avg_edge=std::min(edge_low,static_cast<Image::Pixel>(HDRI_MAXIMUM-edge_high));
//std::cout<<"avg_blur = "<<avg_blur<<std::endl<<std::flush;
		double shift_l = edge_low;//avg_edge;
		double scale_l = static_cast<double>(LDRI_MAXIMUM)/(edge_high-edge_low/*static_cast<double>(img.maximum())-edge_low-edge_high*//*2.*avg_edge*/);
		notifier(3*0.25);

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
			double value = static_cast<double>(LDRI_MAXIMUM)*gamma(static_cast<double>(p)/static_cast<double>(img.maximum()),colour_gamma);
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
				if (!(p%NOTIFY_STEP))
					notifier(3*0.25+0.25*static_cast<float>(p)/static_cast<float>(size));

				hue[p] >>= 8;
			}
		else if (img.maximum() != LDRI_MAXIMUM)
			for (size_t p=0; p<size; ++p)
			{
				if (!(p%NOTIFY_STEP))
					notifier(3*0.25+0.25*static_cast<float>(p)/static_cast<float>(size));

				hue[p] = static_cast<Image::Pixel>(scale*static_cast<double>(hue[p]));
			}
		notifier(1.);
	}

	img.setMaximum(LDRI_MAXIMUM);
}


enum
{
	IMAGE__PLANE__MIN_LUMA=Image::PLANE__USER,
	IMAGE__PLANE__MAX_LUMA,
	IMAGE__PLANE__MIDDLE_LUMA,
	IMAGE__PLANE__RANGE_LUMA,
	IMAGE__PLANE__ALPHA_BACKUP
};
#if 0
//typedef std::pair<ssize_t,ssize_t> Pos;
typedef struct Pos
{
	ssize_t x;
	ssize_t y;

	Pos(void)
		: x(0)
		, y(0)
	{}

	Pos(ssize_t x_,ssize_t y_)
		: x(x_)
		, y(y_)
	{}

	Pos& operator = (const Pos& rs)
	{
		x = rs.x;
		y = rs.y;
		return *this;
	}

	Pos& set(ssize_t x_,ssize_t y_)
	{
		x = x_;
		y = y_;
		return *this;
	}

	bool operator < (const Pos& rs) const
	{
		if (x != rs.x)
			return x < rs.x;
		return y < rs.y;
	}
} Pos;

typedef struct MinMaxPoint
{
	Pos pos;
	Image::Pixel min_value;
	Image::Pixel max_value;

	MinMaxPoint(Pos pos_,Image::Pixel min_value_,Image::Pixel max_value_)
		: pos(pos_)
		, min_value(min_value_)
		, max_value(max_value_)
	{}

} MinMaxPoint;

struct min_value {};
struct max_value {};
struct pos {};

typedef boost::multi_index::multi_index_container<
	MinMaxPoint,
	boost::multi_index::indexed_by<
		boost::multi_index::ordered_non_unique<boost::multi_index::tag<min_value>,boost::multi_index::member<MinMaxPoint,Image::Pixel,&MinMaxPoint::min_value> > ,
		boost::multi_index::ordered_non_unique<boost::multi_index::tag<max_value>,boost::multi_index::member<MinMaxPoint,Image::Pixel,&MinMaxPoint::max_value> > ,
		boost::multi_index::ordered_unique    <boost::multi_index::tag<pos>      ,boost::multi_index::member<MinMaxPoint,Pos         ,&MinMaxPoint::pos> >
	>
> MinMax_MI;
typedef MinMax_MI::index<min_value>::type MinMaxPoint_by_min_value;
typedef MinMax_MI::index<max_value>::type MinMaxPoint_by_max_value;
typedef MinMax_MI::index<pos>::type       MinMaxPoint_by_pos;

void minmax(Image& img,unsigned planeName,unsigned minPlaneName,unsigned maxPlaneName,size_t radius,progress_notifier notifier)
{
	if (!img.hasData())
		throw exception(exception::NO_IMAGE);
	if (!img.hasPlane(planeName))
		throw exception(exception::INVALID_PLANE);
	if( (!radius) || (radius>(std::max(img.width(),img.height()))) )
		throw exception(exception::INVALID_RADIUS);

	Image::Pixel *minPlane=img.plane(minPlaneName);
	Image::Pixel *maxPlane=img.plane(maxPlaneName);

	if (!minPlane || !maxPlane)
		return;

	Image::Pixel const *plane=img.plane(planeName);

	size_t w = img.width();
	size_t h = img.height();

	int p = log(radius)/log(2);
	if (p < 3)
		p = 0;
	else
		p -= 3;
	size_t scale_factor = radius/(1 << p);

	ssize_t s_w = (w+scale_factor-1)/scale_factor;
	ssize_t s_h = (h+scale_factor-1)/scale_factor;
	size_t scaled_size = s_h*s_w;

	boost::shared_array<Image::Pixel> scaled_min_plane_bs(new Image::Pixel[scaled_size]);
	Image::Pixel *scaled_min_plane  = scaled_min_plane_bs.get();
	memset(scaled_min_plane,0xff,scaled_size*sizeof(Image::Pixel));

	boost::shared_array<Image::Pixel> scaled_max_plane_bs(new Image::Pixel[scaled_size]);
	Image::Pixel *scaled_max_plane  = scaled_max_plane_bs.get();
	memset(scaled_max_plane,0,scaled_size*sizeof(Image::Pixel));

	for (size_t y=0; y<h; ++y)
	{
		notifier(0.2*static_cast<float>(y)/static_cast<float>(h));

		size_t yo = y*w;
		size_t syo = (y/scale_factor)*s_w;
		for (size_t x=0; x<w; ++x)
		{
			Image::Pixel pixel = plane[yo+x];
			size_t sxyo = syo + x/scale_factor;
			if (scaled_min_plane[sxyo] > pixel)
				scaled_min_plane[sxyo] = pixel;
			if (scaled_max_plane[sxyo] < pixel)
				scaled_max_plane[sxyo] = pixel;
		}
	}


/* */
	size_t scaled_radius = (radius+scale_factor-1)/scale_factor;
	ssize_t scaled_diameter = scaled_radius*2 + 1;

	boost::shared_array<Pos> indexes_minus_bs(new Pos[scaled_diameter]);
	Pos *indexes_minus = indexes_minus_bs.get();

	boost::shared_array<Pos> indexes_plus_bs(new Pos[scaled_diameter]);
	Pos *indexes_plus = indexes_plus_bs.get();

	size_t ind_i=0;
	ssize_t srad = scaled_radius;
	ssize_t rad2 = scaled_radius*scaled_radius;
	for (ssize_t y=-srad; y<=srad; ++y)
	{
		ssize_t y2 = y*y;
		ssize_t prev_rad2 = y2;
		for (ssize_t x=1; x<=(srad+1); ++x)
		{
			ssize_t cur_rad2 = y2 + x*x;

			if ( (prev_rad2 < rad2) && (cur_rad2 >= rad2) )
			{
				indexes_minus[ind_i]  .set(-x-1,y);
				indexes_plus [ind_i++].set( x  ,y);
			}

			prev_rad2 = cur_rad2;
		}
	}


	boost::scoped_array<Image::Pixel> scaled_min_plane2_bs(new Image::Pixel[scaled_size]);
	Image::Pixel *scaled_min_plane2 = scaled_min_plane2_bs.get();

	boost::scoped_array<Image::Pixel> scaled_max_plane2_bs(new Image::Pixel[scaled_size]);
	Image::Pixel *scaled_max_plane2 = scaled_max_plane2_bs.get();

//	printf("\n");
	for (ssize_t y=0; y<s_h; ++y)
	{
		notifier(0.2+0.2*static_cast<float>(y)/static_cast<float>(s_h));

		MinMax_MI minmax_block;
		MinMaxPoint_by_pos &MM_pos=minmax_block.get<pos>();
		MinMaxPoint_by_min_value &MM_min_value=minmax_block.get<min_value>();
		MinMaxPoint_by_max_value &MM_max_value=minmax_block.get<max_value>();

		for (ssize_t x=-srad; x<s_w; ++x)
		{
			for (ssize_t i=0; i<scaled_diameter; ++i)
			{
				Pos pos = indexes_minus[i];
				pos.x += x;
				pos.y += y;
				if ( (pos.x >= 0)
				&&   (pos.x <  s_w)
				&&   (pos.y >= 0)
				&&   (pos.y <  s_h) )
					MM_pos.erase(pos);

				pos = indexes_plus[i];
				pos.x += x;
				pos.y += y;
				if ( (pos.x >= 0)
				&&   (pos.x <  s_w)
				&&   (pos.y >= 0)
				&&   (pos.y <  s_h) )
					MM_pos.insert(MinMaxPoint(pos,scaled_min_plane[pos.y*s_w+pos.x],scaled_max_plane[pos.y*s_w+pos.x]));
			}

			// set minPlane[] maxPlane[]
			if (x >= 0)
			{
				size_t p=y*s_w+x;
				scaled_min_plane2[p]=(   MM_min_value.begin())->min_value;
				scaled_max_plane2[p]=(--(MM_max_value.end()) )->max_value;
//		printf(" %02x|%02x",minPlane[p],maxPlane[p]);
			}
		}
//		printf("\n");
	}
	indexes_plus_bs.reset();
	indexes_minus_bs.reset();
/* */
	scaled_max_plane_bs.reset();
	scaled_min_plane_bs.reset();


	for (size_t y=0; y<h; ++y)
	{
		notifier(0.4+0.2*static_cast<float>(y)/static_cast<float>(h));

		size_t yo = y*w;
		size_t syo = (y/scale_factor)*s_w;
		for (size_t x=0; x<w; ++x)
		{
			size_t xyo = yo+x;
			size_t sxyo = syo + x/scale_factor;
			minPlane[xyo] = scaled_min_plane2[sxyo];
			maxPlane[xyo] = scaled_max_plane2[sxyo];
		}
	}
	scaled_max_plane2_bs.reset();
	scaled_min_plane2_bs.reset();

//	gaussian_blur(img,minPlaneName,12,boost::bind(&scaled_notifier,notifier,0.6,0.2,_1));
//	gaussian_blur(img,maxPlaneName,12,boost::bind(&scaled_notifier,notifier,0.8,0.2,_1));

	gaussian_blur(img,minPlaneName,radius/2.,boost::bind(&scaled_notifier,notifier,0.6,0.2,_1));
	gaussian_blur(img,maxPlaneName,radius/2.,boost::bind(&scaled_notifier,notifier,0.8,0.2,_1));
}
#endif

typedef struct Minmax_roof_parameters
{
	enum {
		FUNCTION_PARABOLIC,
		FUNCTION_EXPONENTIAL
	} function;
	double exponential_factor;
	size_t minmax_size;
}Minmax_roof_parameters;

void tonemap_local_minmax(Image& img, double colour_gamma, const Minmax_roof_parameters &minmax_roof_parameters, size_t blur_size, double min_range_factor, progress_notifier notifier)
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
							notifier(0+1./6.*static_cast<float>(p)/static_cast<float>(square)/plane_count);

						plane[p] <<= 8;
					}
				else
					for (size_t p=0; p<square; ++p)
					{
						if (!(p % w))
							notifier(0+1./6.*static_cast<float>(p)/static_cast<float>(square)/plane_count);

						plane[p] = static_cast<Image::Pixel>(static_cast<double>(plane[p])*scale);
					}
			}
			notifier(1.);
		}
		img.setMaximum(HDRI_MAXIMUM);
	}

	img.addPlane(IMAGE__PLANE__MIN_LUMA);
	img.addPlane(IMAGE__PLANE__MAX_LUMA);

//	minmax(img,Image::PLANE_LUMA,IMAGE__PLANE__MIN_LUMA,IMAGE__PLANE__MAX_LUMA,blur_size,notifier);

	Image::Pixel *maxPlane=img.plane(IMAGE__PLANE__MAX_LUMA);
	Image::Pixel *minPlane=img.plane(IMAGE__PLANE__MIN_LUMA);
	Image::Pixel *luma=img.plane(Image::PLANE_LUMA);

	memcpy(minPlane,luma,square*sizeof(Image::Pixel));
	memcpy(maxPlane,luma,square*sizeof(Image::Pixel));

	switch (minmax_roof_parameters.function)
	{
	case Minmax_roof_parameters::FUNCTION_PARABOLIC:
		roof_parabolic(img, IMAGE__PLANE__MAX_LUMA,  static_cast<double>(minmax_roof_parameters.minmax_size)/sqrt(static_cast<double>(img.maximum())), boost::bind(&scaled_notifier,notifier,1./6.,1./6.,_1));
		roof_parabolic(img, IMAGE__PLANE__MIN_LUMA, -static_cast<double>(minmax_roof_parameters.minmax_size)/sqrt(static_cast<double>(img.maximum())), boost::bind(&scaled_notifier,notifier,2./6.,1./6.,_1));
		break;
	case Minmax_roof_parameters::FUNCTION_EXPONENTIAL:
		roof_exponential(img, IMAGE__PLANE__MAX_LUMA,  minmax_roof_parameters.exponential_factor, minmax_roof_parameters.minmax_size, boost::bind(&scaled_notifier,notifier,1./6.,1./6.,_1));
		roof_exponential(img, IMAGE__PLANE__MIN_LUMA, -minmax_roof_parameters.exponential_factor, minmax_roof_parameters.minmax_size, boost::bind(&scaled_notifier,notifier,2./6.,1./6.,_1));
		break;
	}


	/* */
	img.addPlane(IMAGE__PLANE__MIDDLE_LUMA);
	img.addPlane(IMAGE__PLANE__RANGE_LUMA);
	Image::Pixel *middlePlane=img.plane(IMAGE__PLANE__MIDDLE_LUMA);
	Image::Pixel *rangePlane=img.plane(IMAGE__PLANE__RANGE_LUMA);
	for (size_t y=0; y<h; ++y)
	{
		notifier(3./6.+1./6.*static_cast<float>(y)/static_cast<float>(h));

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
		img.renamePlane(Image::PLANE_ALPHA,IMAGE__PLANE__ALPHA_BACKUP);
	img.renamePlane(IMAGE__PLANE__RANGE_LUMA,Image::PLANE_ALPHA);
	gaussian_blur_alpha(img,IMAGE__PLANE__MIDDLE_LUMA,blur_size);
	img.removePlane(Image::PLANE_ALPHA);
	img.removePlane(IMAGE__PLANE__RANGE_LUMA);
	if (alpha_present)
		img.renamePlane(IMAGE__PLANE__ALPHA_BACKUP,Image::PLANE_ALPHA);
	for (size_t y=0; y<h; ++y)
	{
		notifier(4./6.+1./6.*static_cast<float>(y)/static_cast<float>(h));

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
			double value = gamma(static_cast<double>(p)/static_cast<double>(HDRI_MAXIMUM),colour_gamma);
			chroma_map[p] =
				(value > 1.) ? HDRI_MAXIMUM :
				( (value < 0.) ? 0 :
				static_cast<Image::Pixel>(HDRI_MAXIMUM * value) );
		}

		Image::Pixel *chroma=img.plane(Image::PLANE_CHROMA);
		for (size_t p=0; p<square; ++p)
		{
			if (!(p % w))
				notifier(5./6.+1./6.*static_cast<float>(p)/static_cast<float>(square));

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
			double value = static_cast<double>(LDRI_MAXIMUM)*gamma(static_cast<double>(p)/static_cast<double>(img.maximum()),colour_gamma);
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
				if (!(p%NOTIFY_STEP))
					notifier(3*0.25+0.25*static_cast<float>(p)/static_cast<float>(size));

				hue[p] >>= 8;
			}
		else if (img.maximum() != LDRI_MAXIMUM)
			for (size_t p=0; p<size; ++p)
			{
				if (!(p%NOTIFY_STEP))
					notifier(3*0.25+0.25*static_cast<float>(p)/static_cast<float>(size));

				hue[p] = static_cast<Image::Pixel>(scale*static_cast<double>(hue[p]));
			}
		notifier(1.);
	}

	img.setMaximum(LDRI_MAXIMUM);
*/

	/* */
	img.removePlane(IMAGE__PLANE__MAX_LUMA);
	img.removePlane(IMAGE__PLANE__MIN_LUMA);
	/*/
	img.removePlane(Image::PLANE_HUE);
	img.removePlane(Image::PLANE_CHROMA);
	img.renamePlane(Image::PLANE_LUMA     ,Image::PLANE_BLUE);
	img.renamePlane(IMAGE__PLANE__MIN_LUMA,Image::PLANE_GREEN);
	img.renamePlane(IMAGE__PLANE__MAX_LUMA,Image::PLANE_RED);
	/* */
}

void tonemap_local_minmax_parabolic(Image& img, double colour_gamma, size_t minmax_size, size_t blur_size, double min_range_factor, progress_notifier notifier)
{
	Minmax_roof_parameters minmax_roof_parameters;
	minmax_roof_parameters.function = Minmax_roof_parameters::FUNCTION_PARABOLIC;
	minmax_roof_parameters.minmax_size = minmax_size;
	tonemap_local_minmax(img, colour_gamma, minmax_roof_parameters, blur_size, min_range_factor, notifier);
}

void tonemap_local_minmax_exponential(Image& img, double colour_gamma, double exponential_factor, size_t minmax_size, size_t blur_size, double min_range_factor, progress_notifier notifier)
{
	Minmax_roof_parameters minmax_roof_parameters;
	minmax_roof_parameters.function = Minmax_roof_parameters::FUNCTION_EXPONENTIAL;
	minmax_roof_parameters.exponential_factor = exponential_factor;
	minmax_roof_parameters.minmax_size = minmax_size;
	tonemap_local_minmax(img, colour_gamma, minmax_roof_parameters, blur_size, min_range_factor, notifier);
}

}
