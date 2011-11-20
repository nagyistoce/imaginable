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


#include <boost/scoped_array.hpp>
#include <boost/format.hpp>

#include "io_pam_saver.hpp"


namespace imaginable {

PAM_saver::PAM_saver(const Image& image)
	: m_image(image)
{
}

PAM_saver::~PAM_saver()
{
}

void PAM_saver::save(std::ostream& stream) const
{
	if(!m_image.hasData())
		return;

	boost::scoped_array<const Image::Pixel*> b_plane(new const Image::Pixel*[m_image.planesNumber()]);
	const Image::Pixel** planes=b_plane.get();
	size_t planes_known=0;
	Image::PlaneNames added;

	std::string tupltype;
	switch(m_image.colourSpace())
	{
		case Image::COLOURSPACE_GRAY:
			planes[planes_known++]=m_image.plane(Image::PLANE_GRAY);
			added.insert(Image::PLANE_GRAY);
			if(m_image.hasTransparency())
			{
				planes[planes_known++]=m_image.plane(Image::PLANE_ALPHA);
				added.insert(Image::PLANE_ALPHA);
				tupltype=(m_image.maximum()==1)?"BLACKANDWHITE_ALPHA":"GRAYSCALE_ALPHA";
			}
			else
				tupltype=(m_image.maximum()==1)?"BLACKANDWHITE":"GRAYSCALE";
		break;
		case Image::COLOURSPACE_RGB:
			planes[planes_known++]=m_image.plane(Image::PLANE_RED);
			planes[planes_known++]=m_image.plane(Image::PLANE_GREEN);
			planes[planes_known++]=m_image.plane(Image::PLANE_BLUE);
			added.insert(Image::PLANE_RED);
			added.insert(Image::PLANE_GREEN);
			added.insert(Image::PLANE_BLUE);
			if(m_image.hasTransparency())
			{
				planes[planes_known++]=m_image.plane(Image::PLANE_ALPHA);
				added.insert(Image::PLANE_ALPHA);
				tupltype="RGB_ALPHA";
			}
			else
				tupltype="RGB";
		break;
		default:;
	}

	Image::PlaneNames planeNames=m_image.planeNames();
	for(Image::PlaneNames::const_iterator I=planeNames.begin();I!=planeNames.end();++I)
	{
		if(added.find(*I)!=added.end())
			break;
		planes[planes_known++]=m_image.plane(*I);
		switch(*I)
		{
			#define CASE(VALUE) case Image::PLANE_##VALUE: if(!tupltype.empty()) tupltype+=' '; tupltype+=#VALUE; break;
			CASE(GRAY)
			CASE(ALPHA)
			CASE(RED)
			CASE(GREEN)
			CASE(BLUE)
			CASE(HUE)
			CASE(SATURATION)
			CASE(LIGHTNESS)
			CASE(VALUE)
			CASE(CHROMA)
			CASE(LUMA)
			#undef CASE
			default:
				if(!tupltype.empty())
					tupltype+=' ';
				tupltype+=(boost::format("USER%|d|") %(*I-Image::PLANE__USER) ).str();
		}
	}

	std::string comment;
	Image::TextKeys comments=m_image.text_keys();
	for(Image::TextKeys::const_iterator TT=comments.begin();TT!=comments.end();++TT)
		comment+=(boost::format("# %|s|: '%|s|'\n") %(*TT) %m_image.text(*TT) ).str();


	size_t mx=m_image.width();
	size_t my=m_image.height();

	stream<<(boost::format("P7\nWIDTH %|d|\nHEIGHT %|d|\nDEPTH %|d|\nMAXVAL %|d|\nTUPLTYPE %|s|\n%|s|ENDHDR\n")
		%mx
		%my
		%planes_known
		%m_image.maximum()
		%tupltype
		%comment).str();


	size_t depth=planes_known;
	size_t total=mx*my*depth;

	if(m_image.maximum()<=0xff)
	{
		boost::scoped_array<uint8_t> b_data(new uint8_t[total]);
		uint8_t* data=b_data.get();

		for(size_t y=0;y<my;++y)
		{
			size_t yo=y*mx;
			for(size_t x=0;x<mx;++x)
			{
				size_t xdo=yo+x;
				size_t xso=(yo+x)*depth;
				for(size_t p=0;p<depth;++p)
					data[xso+p]=static_cast<uint8_t>(planes[p][xdo]);
			}
		}

		stream.write(reinterpret_cast<char*>(data),total*sizeof(uint8_t));
	}
	else
	{
		boost::scoped_array<uint16_t> b_data(new uint16_t[total]);
		uint16_t* data=b_data.get();

		for(size_t y=0;y<my;++y)
		{
			size_t yo=y*mx;
			for(size_t x=0;x<mx;++x)
			{
				size_t xdo=yo+x;
				size_t xso=(yo+x)*depth;
				for(size_t p=0;p<depth;++p)
					data[xso+p]=__bswap_16(static_cast<uint16_t>(planes[p][xdo]));
			}
		}

		stream.write(reinterpret_cast<char*>(data),total*sizeof(uint16_t));
	}
}

}
