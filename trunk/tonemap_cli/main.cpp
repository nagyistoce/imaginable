/*************
**
** Project:      Imaginable :: tonemap (CLI)
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


#include <boost/algorithm/string.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/format.hpp>

#include <cerrno>
#include <iostream>
#include <fstream>

#include <imaginable/image.hpp>
#include <imaginable/io_pnm_loader.hpp>
#include <imaginable/io_pam_saver.hpp>
#include <imaginable/io_qt.hpp>
#include <imaginable/colourspace.hpp>
#include <imaginable/tonemap.hpp>

#include "version.hpp"


void hello(void)
{
	std::cerr<<(boost::format(gettext(
		"%|s|: HDRI to LDRI tonemap converter\n"
		"(High dynamic range image to low dynamic range image tonemap converter)\n"
		"2009 - %|d| (c) Kuzma Shapran   <Kuzma [dot] Shapran [at] gmail [dot] com>\n"
		"Version: %|s|\n"
		"\n"
		))
		%program_invocation_short_name
		%version::year()
		%version::full_string()
		).str();
}

void usage(void)
{
	std::cerr<<(boost::format(gettext(
		"Usage:\n"
		"%|s| <input_HDR_image> <output_LDR_image> <saturation_gamma> -g <lightness_factor>\n"
		"%|s| <input_HDR_image> <output_LDR_image> <saturation_gamma> -l { -x <blur_px_size> | -b <blur_size_factor>} <mix_factor>\n"
		"\n"
		"Use '-' sign as filename to use input and/or output streams instead of files\n"
		"-g  -- use global tonemap function\n"
		"-l  -- use local tonemap function\n"
		"saturation_gamma:   type: double   range: [-10.0 .. 10.0]\n"
		"lightness_factor:   type: double   range: [0.0 .. 10.0]\n"
		"blur_px_size:       type: unsigned integer  (in pixels)\n"
		"blur_size_factor:   type: double   range: (0.0 .. 1.0]\n"
		"mix_factor:         type: double   range: [0.0 .. 1.0]\n"
		))
		%program_invocation_short_name
		%program_invocation_short_name
		).str();
}

int main(int argc,char* argv[])
{
	std::string input_name;
	bool use_stdin;
	std::string output_name;
	bool use_stdout;
	bool tonemap_global_function;
	double saturation_gamma;
	double lightness_factor;
	bool blur_in_pixels=false;
	size_t blur_px_size;
	double blur_size_factor=0.;
	double mix_factor;

	hello();

	try
	{
		char* tail;

		if(argc<=4) throw true;

		input_name=std::string(argv[1]);
		if(input_name.empty())
		{
			std::cerr<<gettext("Empty input name\n");
			throw false;
		}
		use_stdin= (input_name == "-");

		output_name=std::string(argv[2]);
		if(output_name.empty())
		{
			std::cerr<<gettext("Empty output name\n");
			throw false;
		}
		use_stdout= (output_name == "-");

		saturation_gamma=strtod(argv[3],&tail);
		if( (*tail) || (saturation_gamma<-10.) || (saturation_gamma>10.) )
		{
			std::cerr<<gettext("Invalid saturation_gamma value\n");
			throw false;
		}

		if(std::string(argv[4]) == "-g")
			tonemap_global_function=true;
		else if(std::string(argv[4]) == "-l")
			tonemap_global_function=false;
		else
		{
			std::cerr<<gettext("Invalid tonemap function flag\n");
			throw false;
		}

		if(tonemap_global_function)
		{
			if(argc<=5) throw true;

			lightness_factor=strtod(argv[5],&tail);
			if( (*tail) || (lightness_factor<-10.) || (lightness_factor>10.) )
			{
				std::cerr<<gettext("Invalid lightness_factor value\n");
				throw false;
			}
		}
		else
		{
			if(argc<=7) throw true;

			if(std::string(argv[5]) == "-x")
				blur_in_pixels=true;
			else if(std::string(argv[5]) == "-b")
				blur_in_pixels=false;
			else
			{
				std::cerr<<gettext("Invalid blur flag\n");
				throw false;
			}

			if(blur_in_pixels)
			{
				blur_px_size=static_cast<size_t>(strtoull(argv[6],&tail,10));
				if( (*tail) || (!blur_px_size) || (blur_px_size>10000) )
				{
					std::cerr<<gettext("Invalid blur_px_size value\n");
					throw false;
				}
			}
			else
			{
				blur_size_factor=strtod(argv[6],&tail);
				if( (*tail) || (blur_size_factor<=0.) || (blur_size_factor>1.) )
				{
					std::cerr<<gettext("Invalid blur_size_factor value\n");
					throw false;
				}
			}

			mix_factor=strtod(argv[7],&tail);
			if( (*tail) || (mix_factor<0.) || (mix_factor>1.) )
			{
				std::cerr<<gettext("Invalid mix_factor value\n");
				throw false;
			}
		}
	}
	catch(bool)
	{
		usage();
		return 1;
	}


	std::cerr<<(boost::format(gettext(
		"Converting HDRI %|s| to LDRI %|s|\n"
		"Saturation gamma: %|.3f|\n"
		"Tonemap function: %|s|\n"
		))
		%(use_stdin ?std::string( "<input stream>"): input_name)
		%(use_stdout?std::string("<output stream>"):output_name)
		%saturation_gamma
		%(tonemap_global_function?"global":"local")
		).str();

	if(tonemap_global_function)
	{
		std::cerr<<(boost::format(gettext(
			"Lightness factor: %|.3f|\n"
			))
			%lightness_factor
			).str();
	}
	else
	{
		if(blur_in_pixels)
			std::cerr<<(boost::format(gettext(
				"Blur: %|d| px\n"
				))
				%blur_px_size
				).str();
		else
			std::cerr<<(boost::format(gettext(
				"Blur: %|.3f|%%\n"
				))
				%(blur_size_factor*100.)
				).str();
		std::cerr<<(boost::format(gettext(
			"Mix factor: %|.3f|\n"
			))
			%mix_factor
			).str();
	}

	boost::scoped_ptr<imaginable::Image> img(new imaginable::Image);

	std::cerr<<gettext("Loading: ...");
	if(use_stdin)
		std::cin >> imaginable::PNM_loader(*img.get());
	else
	{
		if(boost::algorithm::iends_with(input_name,".pnm")
		|| boost::algorithm::iends_with(input_name,".pam") )
		{
			std::ifstream stream(input_name.c_str());
			if(stream.good())
				stream >> imaginable::PNM_loader(*img.get());
		}
		else
		{
			QImage qimage(input_name.c_str());
			if(!qimage.isNull())
				qimage >> imaginable::QImage_loader(*img.get());
		}
	}
	std::cerr<<"\b\b\b"<<gettext("done\n");

	if(!blur_in_pixels)
	{
		blur_px_size=static_cast<imaginable::Image::pixel>(static_cast<double>(std::min(img->width(),img->height()))*blur_size_factor);
		std::cerr<<(boost::format(gettext(
			"Blur: %|d| px\n"
			))
			%blur_px_size
			).str();
	}

	std::cerr<<gettext("Converting: ...");
	try
	{
		imaginable::rgb_to_hsl(*img.get(),false);
		if(tonemap_global_function)
			imaginable::tonemap_global(*img.get(),saturation_gamma,lightness_factor);
		else
			imaginable::tonemap_local(*img.get(),saturation_gamma,blur_px_size,mix_factor/2.);
		imaginable::hsl_to_rgb(*img.get(),false);
		std::cerr<<"\b\b\b"<<gettext("done\n");
	}
	catch(const std::exception& e)
	{
		std::cerr<<"\b\b\b"<<(boost::format(gettext("failed: %|s|\n")) %e.what() ).str();
		return 1;
	}

	std::cerr<<gettext("Saving: ...");
	if( (img.get()) && (img->hasData()) )
	{
		if(use_stdout)
			std::cout << imaginable::PAM_saver(*img.get());
		else
		{
			if( boost::algorithm::iends_with(output_name,".pam") )
			{
				std::ofstream stream(output_name.c_str());
				if(stream.good())
					stream << imaginable::PAM_saver(*img.get());
			}
			else
			{
				QImage qimage;
				qimage << imaginable::QImage_saver(*img.get());
				qimage.save(output_name.c_str());
			}
		}
		std::cerr<<"\b\b\b"<<gettext("done\n");
	}
	else
		std::cerr<<gettext("Nothing to save.\n");
}
