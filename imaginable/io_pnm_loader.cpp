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


#include <QtCore/QtEndian>

#include <boost/scoped_ptr.hpp>
#include <boost/scoped_array.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/lexical_cast.hpp>

#include <vector>

#include "io_pnm_loader.hpp"


namespace imaginable {

class PNM_base_loader
{
protected:
	std::istream& m_stream;

	bool loadAndUnpackPlanes(Image::Pixel** plane,size_t depth);

public:
	PNM_base_loader(std::istream&,Image*);
	virtual ~PNM_base_loader();

	Image* image;
	virtual bool loadHeader(void)=0;
	virtual void loadImage (void)=0;
};

class PNM_legacy_loader: public PNM_base_loader
{
protected:
	std::vector<size_t> m_header;

public:
	PNM_legacy_loader(std::istream&,Image*);
	virtual ~PNM_legacy_loader();

	void loadLegacyHeader(size_t);
};

#define CLASS_DECL(CLASS,PARENT,FUNCTION) \
class CLASS: public PARENT                \
{                                         \
public:                                   \
	CLASS(std::istream&,Image*);          \
	virtual ~CLASS();                     \
	virtual FUNCTION(void);               \
};

CLASS_DECL(PBM_loader,PNM_legacy_loader,bool loadHeader)
CLASS_DECL(PBMA_loader,PBM_loader,void loadImage)
CLASS_DECL(PBMB_loader,PBM_loader,void loadImage)

CLASS_DECL(PGM_PPM_loader,PNM_legacy_loader,bool loadHeader)
CLASS_DECL(PGMA_loader,PGM_PPM_loader,void loadImage)
CLASS_DECL(PGMB_loader,PGM_PPM_loader,void loadImage)
CLASS_DECL(PPMA_loader,PGM_PPM_loader,void loadImage)
CLASS_DECL(PPMB_loader,PGM_PPM_loader,void loadImage)

#undef CLASS_DECL

class PAM_loader: public PNM_base_loader
{
protected:
	std::string m_tupltype;
	size_t m_depth;

public:
	PAM_loader(std::istream&,Image*);
	virtual ~PAM_loader();

	virtual bool loadHeader(void);
	virtual void loadImage(void);
};





PNM_loader::PNM_loader(Image& image)
	: m_image(image)
{
}

PNM_loader::~PNM_loader()
{
}

void PNM_loader::load(std::istream& stream)
{
	#define PNM_TYPE(NUMBER) ('P'|(NUMBER<<8))
	static const uint16_t PNM_PBMA=PNM_TYPE('1');
	static const uint16_t PNM_PGMA=PNM_TYPE('2');
	static const uint16_t PNM_PPMA=PNM_TYPE('3');
	static const uint16_t PNM_PBMB=PNM_TYPE('4');
	static const uint16_t PNM_PGMB=PNM_TYPE('5');
	static const uint16_t PNM_PPMB=PNM_TYPE('6');
	static const uint16_t PNM_PAM =PNM_TYPE('7');
	#undef PNM_TYPE

	m_image.clear();

	uint16_t header;
	stream.read(reinterpret_cast<char*>(&header),sizeof(header));
	if(stream.good())
	{
		boost::scoped_ptr<PNM_base_loader> loader(NULL);
		switch(header)
		{
		case PNM_PBMA:
			loader.reset(new PBMA_loader(stream,&m_image));
			break;
		case PNM_PGMA:
			loader.reset(new PGMA_loader(stream,&m_image));
			break;
		case PNM_PPMA:
			loader.reset(new PPMA_loader(stream,&m_image));
			break;
		case PNM_PBMB:
			loader.reset(new PBMB_loader(stream,&m_image));
			break;
		case PNM_PGMB:
			loader.reset(new PGMB_loader(stream,&m_image));
			break;
		case PNM_PPMB:
			loader.reset(new PPMB_loader(stream,&m_image));
			break;
		case PNM_PAM:
			loader.reset(new PAM_loader (stream,&m_image));
			break;
		}
		if(loader.get())
			if(loader->loadHeader())
				loader->loadImage();
	}
}


PNM_base_loader::PNM_base_loader(std::istream& stream,Image* p_image)
	: m_stream(stream)
	, image(p_image)
{}

PNM_base_loader::~PNM_base_loader()
{}


#define CLASS_DEF(CLASS,PARENT) \
CLASS::CLASS(std::istream& stream,Image* image) : PARENT(stream,image) {} \
CLASS::~CLASS() {}


CLASS_DEF(PNM_legacy_loader,PNM_base_loader)
CLASS_DEF(PBM_loader,PNM_legacy_loader)
CLASS_DEF(PBMA_loader,PBM_loader)
CLASS_DEF(PBMB_loader,PBM_loader)
CLASS_DEF(PGM_PPM_loader,PNM_legacy_loader)
CLASS_DEF(PGMA_loader,PGM_PPM_loader)
CLASS_DEF(PGMB_loader,PGM_PPM_loader)
CLASS_DEF(PPMA_loader,PGM_PPM_loader)
CLASS_DEF(PPMB_loader,PGM_PPM_loader)
CLASS_DEF(PAM_loader,PNM_base_loader)

#undef CLASS_DEF

void PNM_legacy_loader::loadLegacyHeader(size_t parts)
{
	std::vector<std::string> str_header;

	m_header.clear();
	while( (str_header.size()<parts) && (m_stream.good()) )
	{
		std::string line;
		std::getline(m_stream,line);
		boost::algorithm::trim_left(line);
		size_t hash=line.find('#');
		if(hash!=std::string::npos)
			line=line.substr(0,hash);
		if(!line.empty())
		{
			std::vector<std::string> tokens;
			boost::algorithm::split(tokens,line,boost::algorithm::is_any_of(" \t\n"));
			std::copy(tokens.begin(),tokens.end(),std::back_inserter<std::vector<std::string> >(str_header));
		}
	}

	if(str_header.size()!=parts)
		return;

	for(size_t i=0;i<parts;++i)
		try{
			m_header.push_back(boost::lexical_cast<size_t>(str_header[i]));
		}catch(...){}

	if(m_header.size()!=parts)
		m_header.clear();
}

bool PBM_loader::loadHeader(void)
{
	loadLegacyHeader(2);
	if(m_header.empty())
		return false;

	if( (!m_header[0])
	||  (!m_header[1]) )
		return false;

	size_t square=m_header[0]*m_header[1];
	if( (square/m_header[0]) != m_header[1] )
		return false;

	image->setSize(m_header[0],m_header[1]);
	image->setMaximum(1);
	return true;
}

bool PGM_PPM_loader::loadHeader(void)
{
	loadLegacyHeader(3);
	if(m_header.empty())
		return false;

	if( (!m_header[0])
	||  (!m_header[1]) )
		return false;

	if( ( m_header[2] > Image::MAXIMUM )
	||  (!m_header[2]) )
		return false;

	size_t square=m_header[0]*m_header[1];
	if( (square/m_header[0]) != m_header[1] )
		return false;

	image->setSize(m_header[0],m_header[1]);
	image->setMaximum(m_header[2]);
	return true;
}

bool PAM_loader::loadHeader(void)
{
	if(!m_stream.good())
		return false;

	size_t width=0;
	size_t height=0;
	size_t maxval=0;
	m_depth=0;

	bool ok=true;
	for(bool end=false;(!end) && ok && (m_stream.good());)
	{
		std::string line;
		std::getline(m_stream,line);
		boost::algorithm::trim_left(line);
		size_t hash=line.find('#');
		if(hash!=std::string::npos)
			line=line.substr(0,hash);
		if(!line.empty())
		{
			std::vector<std::string> tokens;
			boost::algorithm::split(tokens,line,boost::algorithm::is_any_of(" \t\n"));

			if(tokens[0]=="ENDHDR")
			{
				ok=(tokens.size()==1);
				end=true;
			}
			else if(tokens[0]=="WIDTH")
			{
				ok=false;
				if( (tokens.size()==2)
				&&  (!width) )
				{
					try{
						width=boost::lexical_cast<size_t>(tokens[1]);
						if(width)
							ok=true;
					}catch(...){}
				}
			}
			else if(tokens[0]=="HEIGHT")
			{
				ok=false;
				if( (tokens.size()==2)
				&&  (!height) )
				{
					try{
						height=boost::lexical_cast<size_t>(tokens[1]);
						if(height)
							ok=true;
					}catch(...){}
				}
			}
			else if(tokens[0]=="DEPTH")
			{
				ok=false;
				if( (tokens.size()==2)
				&&  (!m_depth) )
				{
					try{
						m_depth=boost::lexical_cast<size_t>(tokens[1]);
						if(m_depth)
							ok=true;
					}catch(...){}
				}
			}
			else if(tokens[0]=="MAXVAL")
			{
				ok=false;
				if( (tokens.size()==2)
				&&  (!maxval) )
				{
					try{
						maxval=boost::lexical_cast<size_t>(tokens[1]);
						if( maxval && (maxval<=Image::MAXIMUM) )
							ok=true;
					}catch(...){}
				}
			}
			else if(tokens[0]=="TUPLTYPE")
			{
				if(!m_tupltype.empty())
					m_tupltype+=' ';
				m_tupltype+=boost::algorithm::trim_left_copy(line.substr(tokens[0].length()));
			}
		}
	}
	if(!ok)
		return ok;

	size_t square=width*height;
	if( (square/width) != height )
		return false;

	image->setSize(width,height);
	image->setMaximum(static_cast<Image::Pixel>(maxval));
	image->setText("TUPLTYPE",m_tupltype);

	return true;
}


void PBMA_loader::loadImage(void)
{
	if(image->addPlane(Image::PLANE_GRAY))
	{
		Image::Pixel* planes=image->plane(Image::PLANE_GRAY);
		size_t total=image->width()*image->height();

		image->setMaximum(1);

		char ch;
		size_t Pixel=0;
		while( (Pixel<total) && m_stream.good() )
		{
			m_stream.read(&ch,sizeof(ch));
			if(m_stream.good())
			{
				switch(ch)
				{
					case '0':
					case '1':
						planes[Pixel]=(ch=='1')?0:1;
						++Pixel;
					break;
					case '#':
					{
						std::string comment;
						std::getline(m_stream,comment);
					}
					break;
				}
			}
		}
		if(Pixel==total)
			return;
	}
	image->clear();
}

void PGMA_loader::loadImage(void)
{
	if(image->addPlane(Image::PLANE_GRAY))
	{
		Image::Pixel* planes=image->plane(Image::PLANE_GRAY);
		size_t total=image->width()*image->height();

		size_t cur_pixel=0;
		while( (cur_pixel<total) && (m_stream.good()) )
		{
			std::string line;
			std::getline(m_stream,line);
			boost::algorithm::trim_left(line);
			size_t hash=line.find('#');
			if(hash!=std::string::npos)
				line=line.substr(0,hash);
			if(!line.empty())
			{
				std::vector<std::string> tokens;
				boost::algorithm::split(tokens,line,boost::algorithm::is_any_of(" \t\n"));

				size_t m=tokens.size();
				for(size_t i=0;i<m;++i)
					try{
						size_t value=boost::lexical_cast<size_t>(tokens[i]);
						planes[cur_pixel]=value;
						++cur_pixel;
					}catch(...){}
			}
		}

		if(cur_pixel==total)
			return;
	}
	image->clear();
}

void PPMA_loader::loadImage(void)
{
	if( (image->addPlane(Image::PLANE_RED))
	&&  (image->addPlane(Image::PLANE_GREEN))
	&&  (image->addPlane(Image::PLANE_BLUE)) )
	{
		Image::Pixel* planes[3];
		planes[0]=image->plane(Image::PLANE_RED);
		planes[1]=image->plane(Image::PLANE_GREEN);
		planes[2]=image->plane(Image::PLANE_BLUE);
		size_t current_plane=0;

		size_t total=image->width()*image->height()*3;

		size_t cur_pixel=0;
		while( (cur_pixel<total) && (m_stream.good()) )
		{
			std::string line;
			std::getline(m_stream,line);
			boost::algorithm::trim_left(line);
			size_t hash=line.find('#');
			if(hash!=std::string::npos)
				line=line.substr(0,hash);
			if(!line.empty())
			{
				std::vector<std::string> tokens;
				boost::algorithm::split(tokens,line,boost::algorithm::is_any_of(" \t\n"));

				size_t m=tokens.size();
				for(size_t i=0;i<m;++i)
					try{
						size_t value=boost::lexical_cast<size_t>(tokens[i]);
						planes[current_plane][cur_pixel]=value;
						current_plane=(current_plane+1)%3;
						if(!current_plane)
							++cur_pixel;
					}catch(...){}
			}
		}

		if( (cur_pixel==total)
		&& (!current_plane) )
			return;
	}
	image->clear();
}

void PBMB_loader::loadImage(void)
{
	if(image->addPlane(Image::PLANE_GRAY))
	{
		Image::Pixel* planes=image->plane(Image::PLANE_GRAY);

		size_t mx=image->width();
		size_t my=image->height();
		size_t row=((mx+7)/8);
		size_t total=row*my;

		boost::scoped_array<uint8_t> b_data(new uint8_t[total]);
		uint8_t* data=b_data.get();
		m_stream.read(reinterpret_cast<char*>(data),total);

		if(m_stream.good())
		{
			for(size_t y=0;y<my;++y)
			{
				size_t ydo=y*mx;
				size_t yso=y*row;
				for(size_t x=0;x<mx;++x)
					planes[ydo+x]=((data[yso+x/8]>>(7-(x%8)))&1)?0:1;
			}
			return;
		}
	}
	image->clear();
}

void PGMB_loader::loadImage(void)
{
	if(image->addPlane(Image::PLANE_GRAY))
	{
		Image::Pixel* planes=image->plane(Image::PLANE_GRAY);

		if(loadAndUnpackPlanes(&planes,1))
			return;
	}
	image->clear();
}

void PPMB_loader::loadImage(void)
{
	if( (image->addPlane(Image::PLANE_RED))
	&&  (image->addPlane(Image::PLANE_GREEN))
	&&  (image->addPlane(Image::PLANE_BLUE)) )
	{
		Image::Pixel* planes[3];
		planes[0]=image->plane(Image::PLANE_RED);
		planes[1]=image->plane(Image::PLANE_GREEN);
		planes[2]=image->plane(Image::PLANE_BLUE);

		if(loadAndUnpackPlanes(planes,3))
			return;
	}
	image->clear();
}

void PAM_loader::loadImage(void)
{
	boost::scoped_array<Image::Pixel*> b_plane(new Image::Pixel*[m_depth]);
	Image::Pixel** planes=b_plane.get();
	size_t planes_known=0;

	do{
		if(m_tupltype=="BLACKANDWHITE")
		{
			if(m_depth<1)
				break;
			if(image->addPlane(Image::PLANE_GRAY))
				planes[planes_known++]=image->plane(Image::PLANE_GRAY);
			else
				break;
		}
		else if(m_tupltype=="BLACKANDWHITE_ALPHA")
		{
			if(m_depth<2)
				break;
			if( (image->addPlane(Image::PLANE_GRAY))
			&&  (image->addPlane(Image::PLANE_ALPHA)) )
			{
				planes[planes_known++]=image->plane(Image::PLANE_GRAY);
				planes[planes_known++]=image->plane(Image::PLANE_ALPHA);
			}
			else
				break;
		}
		else if(m_tupltype=="GRAYSCALE")
		{
			if(m_depth<1)
				break;
			if(image->addPlane(Image::PLANE_GRAY))
				planes[planes_known++]=image->plane(Image::PLANE_GRAY);
			else
				break;
		}
		else if(m_tupltype=="GRAYSCALE_ALPHA")
		{
			if(m_depth<2)
				break;
			if( (image->addPlane(Image::PLANE_GRAY))
			&&  (image->addPlane(Image::PLANE_ALPHA)) )
			{
				planes[planes_known++]=image->plane(Image::PLANE_GRAY);
				planes[planes_known++]=image->plane(Image::PLANE_ALPHA);
			}
			else
				break;
		}
		else if(m_tupltype=="RGB")
		{
			if(m_depth<3)
				break;
			if( (image->addPlane(Image::PLANE_RED))
			&&  (image->addPlane(Image::PLANE_GREEN))
			&&  (image->addPlane(Image::PLANE_BLUE)) )
			{
				planes[planes_known++]=image->plane(Image::PLANE_RED);
				planes[planes_known++]=image->plane(Image::PLANE_GREEN);
				planes[planes_known++]=image->plane(Image::PLANE_BLUE);
			}
			else
				break;
		}
		else if(m_tupltype=="RGB_ALPHA")
		{
			if(m_depth<4)
				break;
			if( (image->addPlane(Image::PLANE_RED))
			&&  (image->addPlane(Image::PLANE_GREEN))
			&&  (image->addPlane(Image::PLANE_BLUE))
			&&  (image->addPlane(Image::PLANE_ALPHA)) )
			{
				planes[planes_known++]=image->plane(Image::PLANE_RED);
				planes[planes_known++]=image->plane(Image::PLANE_GREEN);
				planes[planes_known++]=image->plane(Image::PLANE_BLUE);
				planes[planes_known++]=image->plane(Image::PLANE_ALPHA);
			}
			else
				break;
		}
		size_t m=m_depth-planes_known;
		for(size_t i=0;i<m;++i)
		{
			if(image->addPlane(Image::PLANE__USER+i))
				planes[planes_known++]=image->plane(Image::PLANE__USER+i);
			else
				break;
		}
		if(m_depth!=planes_known)
			break;

		if(loadAndUnpackPlanes(planes,m_depth))
			return;

	}while(false);
	image->clear();
}


bool PNM_base_loader::loadAndUnpackPlanes(Image::Pixel** planes,size_t depth)
{
	size_t mx=image->width();
	size_t my=image->height();
	size_t total=mx*my*depth;

	if(image->maximum()<=0xff)
	{
		boost::scoped_array<uint8_t> b_data(new uint8_t[total]);
		uint8_t* data=b_data.get();
		m_stream.read(reinterpret_cast<char*>(data),total);

		if(m_stream.good())
		{
			for(size_t y=0;y<my;++y)
			{
				size_t yo=y*mx;
				for(size_t x=0;x<mx;++x)
				{
					size_t xdo=yo+x;
					size_t xso=(yo+x)*depth;
					for(size_t p=0;p<depth;++p)
						planes[p][xdo]=static_cast<Image::Pixel>(data[xso+p]);
				}
			}
			return true;
		}
	}
	else
	{
		boost::scoped_array<uint16_t> b_data(new uint16_t[total]);
		uint16_t* data=b_data.get();
		m_stream.read(reinterpret_cast<char*>(data),total*sizeof(uint16_t));

		if(m_stream.good())
		{
			for(size_t y=0;y<my;++y)
			{
				size_t yo=y*mx;
				for(size_t x=0;x<mx;++x)
				{
					size_t xdo=yo+x;
					size_t xso=(yo+x)*depth;
					for(size_t p=0;p<depth;++p)
						planes[p][xdo]=static_cast<Image::Pixel>(qFromBigEndian(data[xso+p]));
				}
			}
			return true;
		}
	}
	return false;
}

}
