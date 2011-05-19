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

#include <cmath>
#include <vector>
#include <cstring>

#include "tools_gamma.hpp"
#include "tools_blur.hpp"
#include "tools_tonemap.hpp"


namespace imaginable {


static const Image::pixel HDRI_MAXIMUM=0xffff;
static const Image::pixel LDRI_MAXIMUM=0xff;
static const size_t TABLE_POWER=2;
static const size_t TABLE_SIZE=256*(1<<TABLE_POWER);

void tonemap_global(Image& img,double saturation_gamma,double lightness_factor)
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
					++histo[lightness[p]>>(8-TABLE_POWER)];
					++avg;
				}
			}
			else
			{
				for(size_t p=0;p<size;++p)
				{
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
					if(alpha[p])
					{
						++histo[lightness[p]>>(8-TABLE_POWER)];
						++avg;
					}
			}
			else
			{
				for(size_t p=0;p<size;++p)
					if(alpha[p])
					{
						++histo[static_cast<size_t>(scale4*static_cast<double>(lightness[p]))];
						++avg;
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
			lightness[p]=table[static_cast<size_t>(scale4*static_cast<double>(lightness[p]))];
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
			saturation[p]=curve[static_cast<size_t>(scale4*static_cast<double>(saturation[p]))];
	}

	{
		Image::pixel* hue=img.plane(Image::PLANE_HUE);

		if(img.maximum()==HDRI_MAXIMUM)
			for(size_t p=0;p<size;++p)
				hue[p]>>=8;
		else
			for(size_t p=0;p<size;++p)
				hue[p]=static_cast<Image::pixel>(scale*static_cast<double>(hue[p]));
	}

	img.setMaximum(LDRI_MAXIMUM);
}

enum
{
	IMAGE__PLANE__BLURRED_LIGHTNESS=Image::PLANE__USER
};

void tonemap_local(Image& img,double saturation_gamma,Image::pixel blur_size,double mix_factor)
{
	if(!img.hasData())
		throw exception(exception::NO_IMAGE);

	if(img.maximum()<=LDRI_MAXIMUM)
		throw exception(exception::NOT_AN_HDRI);

	if(img.colourSpace()!=Image::IMAGE_HSL)
		throw exception(exception::INVALID_COLOUR_SPACE);


	size_t size=img.width()*img.height();

	{
		Image::pixel* alpha=img.plane(Image::PLANE_ALPHA);
		Image::pixel* lightness=img.plane(Image::PLANE_HSL_LIGHTNESS);
		img.addPlane(IMAGE__PLANE__BLURRED_LIGHTNESS);
		Image::pixel* blurred_lightness=img.plane(IMAGE__PLANE__BLURRED_LIGHTNESS);

		memcpy(blurred_lightness,lightness,size*sizeof(Image::pixel));
		//blur lightness2
		box_blur(img,IMAGE__PLANE__BLURRED_LIGHTNESS,blur_size,alpha);
		//invert blurred lightness2
		for(size_t p=0;p<size;++p)
			blurred_lightness[p]=img.maximum()-blurred_lightness[p];

		int64_t avg_blur=0;
		size_t avg_blur_count=0;
//		int64_t median=img.maximum()/2;

		Image::pixel edge_low=HDRI_MAXIMUM;
		Image::pixel edge_high=0;
		//mix them
		for(size_t p=0;p<size;++p)
		{
			if(!alpha || alpha[p])
			{
				avg_blur+=abs(static_cast<int64_t>(blurred_lightness[p])-static_cast<int64_t>(lightness[p]));
				++avg_blur_count;
				Image::pixel value=static_cast<Image::pixel>(
					(1.-mix_factor)*static_cast<double>(lightness[p])+
					mix_factor     *static_cast<double>(blurred_lightness[p]) );
				edge_low=std::min(edge_low,value);
				edge_high=std::max(edge_high,value);
				lightness[p]=value;
			}
		}
		avg_blur*=2./avg_blur_count;
//std::cout<<"edges: "<<edge_low<<" .. "<<edge_high;
		edge_high=(HDRI_MAXIMUM-edge_high);
//std::cout<<" ("<<edge_high<<")"<<std::endl<<std::flush;
		//Image::pixel avg_edge=std::min(edge_low,static_cast<Image::pixel>(HDRI_MAXIMUM-edge_high));
//std::cout<<"avg_blur = "<<avg_blur<<std::endl<<std::flush;
		double shift_l=edge_low;//avg_edge;
		double scale_l=static_cast<double>(LDRI_MAXIMUM)/(static_cast<double>(img.maximum())-edge_low-edge_high/*2.*avg_edge*/);

		img.removePlane(IMAGE__PLANE__BLURRED_LIGHTNESS);
//		img.removePlane(Image::PLANE_HSL_SATURATION);
//		img.renamePlane(IMAGE__PLANE__BLURRED_LIGHTNESS,Image::PLANE_HSL_SATURATION);

//		double shift_l=median-avg_blur;
//		double scale_l=static_cast<double>(LDRI_MAXIMUM)/static_cast<double>(2*avg_blur);
//		shift_l=0.;
//		scale_l=static_cast<double>(LDRI_MAXIMUM)/(static_cast<double>(img.maximum())-2.*shift_l);

//std::cout<<"Compression = "<<(256.*scale_l)<<std::endl<<std::flush;

		for(size_t p=0;p<size;++p)
		{
			double value=scale_l*(static_cast<double>(lightness[p])-shift_l);
			if(value>static_cast<double>(LDRI_MAXIMUM))
				value=static_cast<double>(LDRI_MAXIMUM);
			else if(value<0.)
				value=0.;
			lightness[p]=static_cast<Image::pixel>(value);
		}
	}

	double scale=static_cast<double>(LDRI_MAXIMUM)/static_cast<double>(img.maximum());
	double scale4=scale*static_cast<double>(1<<TABLE_POWER);
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
			saturation[p]=curve[static_cast<size_t>(scale4*static_cast<double>(saturation[p]))];
	}

	{
		Image::pixel* hue=img.plane(Image::PLANE_HUE);

		if(img.maximum()==HDRI_MAXIMUM)
			for(size_t p=0;p<size;++p)
				hue[p]>>=8;
		else
			for(size_t p=0;p<size;++p)
				hue[p]=static_cast<Image::pixel>(scale*static_cast<double>(hue[p]));
	}

	img.setMaximum(LDRI_MAXIMUM);
}

#if 0
void tone_map_20091013(Image& img,double saturation_compression,double lightness_compression)
{
	if(!img.hasData())
		throw exception(exception::NO_IMAGE);

	if(img.maximum()<=LDRI_MAXIMUM)
		throw exception(exception::NOT_AN_HDRI);

	if(img.colourSpace()!=Image::IMAGE_HSL)
		throw exception(exception::INVALID_COLOUR_SPACE);


	size_t width =img.width();
	size_t height=img.height();
	size_t size  =width*height;

	std::map<unsigned int,double> compression;
	compression[Image::PLANE_HSL_SATURATION]=saturation_compression;
	compression[Image::PLANE_HSL_LIGHTNESS ]= lightness_compression;

	double scale=static_cast<double>(LDRI_MAXIMUM)/static_cast<double>(img.maximum());
	Image::t_planeNames planes=img.planeNames();
	for(Image::t_planeNames::const_iterator I=planes.begin();I!=planes.end();++I)
	{
		Image::pixel* plane=img.plane(*I);

		switch(*I)
		{
			case Image::PLANE_HUE_SECTOR:
			break;
			case Image::PLANE_HSL_SATURATION:
			case Image::PLANE_HSL_LIGHTNESS:
			{
				boost::array<size_t,256> histo = { { 0 } };

				for(size_t y=0;y<height;++y)
				{
					size_t yo=y*width;
					for(size_t x=0;x<width;++x)
					{
						size_t xo=yo+x;
						++histo[static_cast<size_t>(scale*static_cast<double>(plane[xo]))];
					}
				}

				size_t gap_count =size/ 1000;
				size_t edge_count=size/10000;

				size_t dark_edge=std::find_if(histo.begin(),histo.end(),std::bind2nd(std::greater<size_t>(),edge_count))-histo.begin();
				if(dark_edge>=histo.size())
				{
					std::cout<<(boost::format(gettext("Cannot find dark edge in %|s| channel.\n")) %((*I==Image::PLANE_HSL_SATURATION)?gettext("saturation"):gettext("lightness")) ).str();
					throw exception(exception::INVALID_DATA);
				}
				size_t gap_right=std::find_if(histo.begin()+dark_edge,histo.end(),std::bind2nd(std::greater<size_t>(),gap_count))-histo.begin();
				if(gap_right>=histo.size())
				{
					std::cout<<(boost::format(gettext("Cannot find dark gap right side in %|s| channel.\n")) %((*I==Image::PLANE_HSL_SATURATION)?gettext("saturation"):gettext("lightness")) ).str();
					throw exception(exception::INVALID_DATA);
				}
				std::vector<std::pair<size_t,size_t> > gap;
				size_t gap_left;
				while(true)
				{
					gap_left=std::find_if(histo.begin()+gap_right,histo.end(),std::bind2nd(std::less<size_t>(),gap_count))-histo.begin();
					if(gap_left>=histo.size())
						break;

					gap_right=std::find_if(histo.begin()+gap_left,histo.end(),std::bind2nd(std::greater<size_t>(),gap_count))-histo.begin();
					if(gap_right>=histo.size())
						break;

					gap.push_back(std::make_pair(gap_left,gap_right));
				}

				size_t light_edge=histo.size()-(std::find_if(histo.rbegin(),histo.rbegin()+histo.size()-gap_left,std::bind2nd(std::greater<size_t>(),edge_count))-histo.rbegin());

				//	gap.clear();
				//	dark_edge=0;
				//	light_edge=255;

				size_t gaps=gap.size();
				/* */ // debug info [
				std::cout<<(boost::format(
					"%|s| channel info:\n"
					"dark_edge=%|d|\n"
					"light_edge=%|d|\n"
					"gaps=%|d|\n")
					%((*I==Image::PLANE_HSL_SATURATION)?gettext("Saturation"):gettext("Lightness"))
					%dark_edge
					%light_edge
					%gaps).str();
				/* */ // ]
				size_t total_gaps=0;
				for(size_t i=0;i<gaps;++i)
				{
					/* */std::cout<<(boost::format("gap[%|d|]: %|d| .. %|d| = %|d|\n") %i %gap[i].first %gap[i].second %(gap[i].second-gap[i].first) ).str();
					total_gaps+=gap[i].second-gap[i].first;
				}

				size_t compressed_range=light_edge-dark_edge-total_gaps;
				/* */std::cout<<(boost::format("Compressed range=%|d|\n") %compressed_range ).str();

				if(!compressed_range)
				{
					std::cout<<(boost::format(gettext("Cannot calculate compressed range in %|s| channel.\n")) %((*I==Image::PLANE_HSL_SATURATION)?gettext("saturation"):gettext("lightness")) ).str();
					throw exception(exception::INVALID_DATA);
				}


				boost::array<double,257> curve;

				double contrast_curve=pow(2.,compression[*I]);

				curve[0]=0.;

				for(size_t x=0;x<dark_edge;++x)
					curve[x+1]=1.;

				for(size_t x=dark_edge;x<light_edge;++x)
					curve[x+1]=contrast_curve;

				for(size_t x=light_edge;x<histo.size();++x)
					curve[x+1]=1.;

				for(size_t i=0;i<gaps;++i)
					for(size_t x=gap[i].first;x<gap[i].second;++x)
						curve[x+1]=1.;

				size_t m=curve.size();
				for(size_t x=1;x<m;++x)
					curve[x]+=curve[x-1];

				--m;
				double curve_max=curve[m];

				for(size_t x=0;x<m;++x)
					curve[x]=(curve[x]+curve[x+1])/(2.*curve_max)*256/(static_cast<double>(x)+0.5);

				/*
				for(size_t i=0;i<256;++i)
					std::cout<<curve[i]<<"\n";
				*/

				for(size_t y=0;y<height;++y)
				{
					size_t yo=y*width;
					for(size_t x=0;x<width;++x)
					{
						size_t xo=yo+x;
						double value=static_cast<double>(plane[xo])*curve[static_cast<size_t>(scale*static_cast<double>(plane[xo]))];
						if(value>65535.)
							value=65535.;
						plane[xo]=static_cast<Image::pixel>(scale*value);
					}
				}
			}
			break;
			default:
				for(size_t y=0;y<height;++y)
				{
					size_t yo=y*width;
					for(size_t x=0;x<width;++x)
					{
						size_t xo=yo+x;
						plane[xo]=static_cast<Image::pixel>(scale*static_cast<double>(plane[xo]));
					}
				}
		}
	}
	img.setMaximum(LDRI_MAXIMUM);
}
#endif

}
