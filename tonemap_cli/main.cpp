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
#include <imaginable/time.hpp>
#include <imaginable/version.hpp>

#include "version.hpp"


void hello(void)
{
	std::cerr << (boost::format(gettext(
		"%|s|: HDRI to LDRI tonemap converter\n"
		"(High dynamic range image to low dynamic range image tonemap converter)\n"
		"2009 - %|d| (c) Kuzma Shapran   <Kuzma [dot] Shapran [at] gmail [dot] com>\n"
		"Version: %|s|\n"
		"Imaginable version: %|s|\n"
		"\n"
		))
		%program_invocation_short_name
		%version::year()
		%version::full_string()
		%imaginable::version::full_string()
		).str();
}

void usage(void)
{
	std::cerr << (boost::format(gettext(
		"Usage:\n"
		"%|s| options\n"
		"\n"
		"Main options are:\n"
		"  -i <input_HDR_image>\n"
		"    default: stdin\n"
		"  -o <output_LDR_image>\n"
		"    default: stdout\n"
		"\n"
		"  -m <method_name>\n"
		"    Valid method names are:\n"
		"      g global\n"
		"      a average\n"
		"      p parabolic\n"
		"      e exponential\n"
		"\n"
		"Global method options are:\n"
		"  -s <saturation_gamma>\n"
		"    value type: double   range: [-10.0 .. 10.0]  default: 0.0\n"
		"  -l <lightness_factor>\n"
		"    value type: double   range: [0.0 .. 10.0]\n"
		"\n"
		"Average method options are:\n"
		"  -s <saturation_gamma>\n"
		"    value type: double   range: [-10.0 .. 10.0]  default: 0.0\n"
		"  -bp <blur_px_size>\n"
		"    value type: unsigned integer  (radius in pixels)\n"
		"  -bf <blur_size_factor>\n"
		"    value type: double   range: (0.0 .. 1.0] -- part of the smallest side\n"
		"Note: Use only one of -bp and -bf options\n"
		"  -f <mix_factor>\n"
		"    value type: double   range: [0.0 .. 1.0]  default: 0.5\n"
		"\n"
		"Parabolic minmax method options are:\n"
		"  -s <saturation_gamma>\n"
		"    value type: double   range: [-10.0 .. 10.0]  default: 0.0\n"
		"  -mp <minmax_px_size>\n"
		"    value type: unsigned integer  (radius in pixels)\n"
		"  -mf <minmax_size_factor>\n"
		"    value type: double   range: (0.0 .. 1.0] -- part of the smallest side\n"
		"Note: Use only one of -mp and -mf options\n"
		"  -bp <blur_px_size>\n"
		"    value type: unsigned integer  (radius in pixels)\n"
		"  -bf <blur_size_factor>\n"
		"    value type: double   range: (0.0 .. 1.0] -- part of the smallest side\n"
		"Note: Use only one of -bp and -bf options\n"
		"  -r <min_range_factor>\n"
		"    value type: double   range: (0.0 .. 1.0] -- part of full luma range  default: 0.25\n"
		"\n"
		"Exponential minmax method options are:\n"
		"  -s <saturation_gamma>\n"
		"    value type: double   range: [-10.0 .. 10.0]  default: 0.0\n"
		"  -e <exponential_factor>\n"
		"    value type: double   range: (0.0 .. 10.0]  default: 1.0\n"
		"  -mp <minmax_px_size>\n"
		"    value type: unsigned integer  (radius in pixels)\n"
		"  -mf <minmax_size_factor>\n"
		"    value type: double   range: (0.0 .. 1.0] -- part of the smallest side\n"
		"Note: Use only one of -mp and -mf options\n"
		"  -bp <blur_px_size>\n"
		"    value type: unsigned integer  (radius in pixels)\n"
		"  -bf <blur_size_factor>\n"
		"    value type: double   range: (0.0 .. 1.0] -- part of the smallest side\n"
		"Note: Use only one of -bp and -bf options\n"
		"  -r <min_range_factor>\n"
		"    value type: double   range: (0.0 .. 1.0] -- part of full luma range  default: 0.25\n"
		))
		%program_invocation_short_name
		).str();
}

double last_printer_percent = 0.0;

void percent_printer(float value)
{
	double now = getHighPrecTime();
	if (now - last_printer_percent > 1./3.)
	{
		last_printer_percent = now;
		std::cout << "\b\b\b\b\b\b" << (boost::format(gettext("%|5.1f|%%")) %(value*100) ).str() << std::flush;
	}
}

int main(int argc,char* argv[])
{
	std::string input_name;
	bool use_stdin = true;

	std::string output_name;
	bool use_stdout = true;

	char method_name = ' ';

	double saturation_gamma = 0.0;

	double lightness_factor;
	bool lightness_factor_specified = false;

	bool blur_in_pixels=false;
	size_t blur_px_size;
	double blur_size_factor=0.;
	bool blur_specfified = false;

	double mix_factor = 0.5;

	double exponential_factor = 1.0;

	bool minmax_in_pixels=false;
	size_t minmax_px_size;
	double minmax_size_factor=0.;
	bool minmax_specfified = false;

	double min_range_factor = 0.25;

	hello();

	try
	{
		char* tail;

		for (int argi = 1; argi<argc; ++argi)
		{
			if (!strcmp(argv[argi],"-i"))
			{
				++argi;
				if (argi == argc)
				{
					std::cerr << gettext("Missed input name") << std::endl;
					throw false;
				}
				input_name = std::string(argv[argi]);
				if(input_name.empty())
				{
					std::cerr << gettext("Empty input name") << std::endl;
					throw false;
				}
				use_stdin = false;
			}
			else if (!strcmp(argv[argi],"-o"))
			{
				++argi;
				if (argi == argc)
				{
					std::cerr << gettext("Missed output name") << std::endl;
					throw false;
				}
				output_name = std::string(argv[argi]);
				if(output_name.empty())
				{
					std::cerr << gettext("Empty output name") << std::endl;
					throw false;
				}
				use_stdout = false;
			}
			else if (!strcmp(argv[argi],"-m"))
			{
				++argi;
				if (argi == argc)
				{
					std::cerr << gettext("Missed method name") << std::endl;
					throw false;
				}
				if (((!strcmp(argv[argi],"g"))) || (!strcmp(argv[argi],"global"))
				||  ((!strcmp(argv[argi],"a"))) || (!strcmp(argv[argi],"average"))
				||  ((!strcmp(argv[argi],"p"))) || (!strcmp(argv[argi],"parabolic"))
				||  ((!strcmp(argv[argi],"e"))) || (!strcmp(argv[argi],"exponential")))
					method_name = argv[argi][0];
				else
				{
					std::cerr << gettext("Invalid method name") << std::endl;
					throw false;
				}
			}
			else if (!strcmp(argv[argi],"-s"))
			{
				++argi;
				if (argi == argc)
				{
					std::cerr << gettext("Missed saturation_gamma value") << std::endl;
					throw false;
				}
				saturation_gamma = strtod(argv[argi],&tail);
				if( (*tail) || (saturation_gamma < -10.) || (saturation_gamma > 10.) )
				{
					std::cerr << gettext("Invalid saturation_gamma value") << std::endl;
					throw false;
				}
			}
			else if (!strcmp(argv[argi],"-l"))
			{
				++argi;
				if (argi == argc)
				{
					std::cerr << gettext("Missed lightness_factor value") << std::endl;
					throw false;
				}
				lightness_factor = strtod(argv[argi],&tail);
				if( (*tail) || (lightness_factor < -10.) || (lightness_factor > 10.) )
				{
					std::cerr << gettext("Invalid lightness_factor value") << std::endl;
					throw false;
				}
				lightness_factor_specified = true;
			}
			else if (!strcmp(argv[argi],"-bp"))
			{
				++argi;
				if (argi == argc)
				{
					std::cerr << gettext("Missed blur_px_size value") << std::endl;
					throw false;
				}
				blur_px_size = static_cast<size_t>(strtoull(argv[argi],&tail,10));
				if( (*tail) || (!blur_px_size) || (blur_px_size > 10000) )
				{
					std::cerr << gettext("Invalid blur_px_size value") << std::endl;
					throw false;
				}
				blur_in_pixels = true;
				blur_specfified = true;
			}
			else if (!strcmp(argv[argi],"-bf"))
			{
				++argi;
				if (argi == argc)
				{
					std::cerr << gettext("Missed blur_size_factor value") << std::endl;
					throw false;
				}
				blur_size_factor = strtod(argv[argi],&tail);
				if( (*tail) || (blur_size_factor <= 0.) || (blur_size_factor > 1.) )
				{
					std::cerr << gettext("Invalid blur_size_factor value") << std::endl;
					throw false;
				}
				blur_in_pixels = false;
				blur_specfified = true;
			}
			else if (!strcmp(argv[argi],"-e"))
			{
				++argi;
				if (argi == argc)
				{
					std::cerr << gettext("Missed exponential_factor value") << std::endl;
					throw false;
				}
				exponential_factor = strtod(argv[argi],&tail);
				if( (*tail) || (exponential_factor <= 0.) || (exponential_factor > 10.) )
				{
					std::cerr << gettext("Invalid exponential_factor value") << std::endl;
					throw false;
				}
			}
			else if (!strcmp(argv[argi],"-f"))
			{
				++argi;
				if (argi == argc)
				{
					std::cerr << gettext("Missed mix_factor value") << std::endl;
					throw false;
				}
				mix_factor = strtod(argv[argi],&tail);
				if( (*tail) || (mix_factor < 0.) || (mix_factor > 1.) )
				{
					std::cerr << gettext("Invalid mix_factor value") << std::endl;
					throw false;
				}
			}
			else if (!strcmp(argv[argi],"-mp"))
			{
				++argi;
				if (argi == argc)
				{
					std::cerr << gettext("Missed minmax_px_size value") << std::endl;
					throw false;
				}
				minmax_px_size = static_cast<size_t>(strtoull(argv[argi],&tail,10));
				if( (*tail) || (!minmax_px_size) || (minmax_px_size > 10000) )
				{
					std::cerr << gettext("Invalid minmax_px_size value") << std::endl;
					throw false;
				}
				minmax_in_pixels = true;
				minmax_specfified = true;
			}
			else if (!strcmp(argv[argi],"-mf"))
			{
				++argi;
				if (argi == argc)
				{
					std::cerr << gettext("Missed minmax_size_factor value") << std::endl;
					throw false;
				}
				minmax_size_factor = strtod(argv[argi],&tail);
				if( (*tail) || (minmax_size_factor <= 0.) || (minmax_size_factor > 1.) )
				{
					std::cerr << gettext("Invalid minmax_size_factor value") << std::endl;
					throw false;
				}
				minmax_in_pixels = false;
				minmax_specfified = true;
			}
			else if (!strcmp(argv[argi],"-r"))
			{
				++argi;
				if (argi == argc)
				{
					std::cerr << gettext("Missed min_range_factor value") << std::endl;
					throw false;
				}
				min_range_factor = strtod(argv[argi],&tail);
				if( (*tail) || (min_range_factor <= 0.) || (min_range_factor > 1.) )
				{
					std::cerr << gettext("Invalid min_range_factor value") << std::endl;
					throw false;
				}
			}
		}
		if (method_name == ' ') // not specified
		{
			std::cerr << gettext("Missed lightness_factor value") << std::endl;
			throw false;
		}
		switch (method_name)
		{
		case 'g':
			if (!lightness_factor_specified)
			{
				std::cerr << gettext("Missed method name") << std::endl;
				throw false;
			}
			break;

		case 'a':
			if (!blur_specfified)
			{
				std::cerr << gettext("Missed blur radius") << std::endl;
				throw false;
			}
			break;

		case 'p':
			if (!minmax_specfified)
			{
				std::cerr << gettext("Missed minmax radius") << std::endl;
				throw false;
			}
			if (!blur_specfified)
			{
				std::cerr << gettext("Missed blur radius") << std::endl;
				throw false;
			}

		case 'e':
			if (!minmax_specfified)
			{
				std::cerr << gettext("Missed minmax radius") << std::endl;
				throw false;
			}
			if (!blur_specfified)
			{
				std::cerr << gettext("Missed blur radius") << std::endl;
				throw false;
			}
			break;
		}
	}
	catch (bool)
	{
		usage();
		return 1;
	}

	std::cout << (boost::format(gettext("Input HDRI: %|s|")) %(use_stdin ? std::string("<input stream>") : input_name) ).str() << std::endl;
	std::cout << (boost::format(gettext("Output LDRI: %|s|")) %(use_stdout ? std::string("<output stream>") : output_name) ).str() << std::endl;

	switch (method_name)
	{
	case 'g':
		std::cout << (boost::format(gettext("Saturation gamma: %|.3f|")) %saturation_gamma ).str() << std::endl;
		std::cout << (boost::format(gettext("Lightness factor: %|.3f|")) %lightness_factor ).str() << std::endl;
		break;

	case 'a':
		std::cout << (boost::format(gettext("Saturation gamma: %|.3f|")) %saturation_gamma ).str() << std::endl;
		if(blur_in_pixels)
			std::cout << (boost::format(gettext("Blur radius: %|d| px")) %blur_px_size ).str() << std::endl;
		else
			std::cout << (boost::format(gettext("Blur radius: %|.3f|%%")) %(blur_size_factor*100.) ).str() << std::endl;
		std::cout << (boost::format(gettext("Mix factor: %|.3f|")) %mix_factor ).str() << std::endl;
		break;

	case 'p':
		std::cout << (boost::format(gettext("Saturation gamma: %|.3f|")) %saturation_gamma ).str() << std::endl;
		if(minmax_in_pixels)
			std::cout << (boost::format(gettext("Minmax radius: %|d| px")) %minmax_px_size ).str() << std::endl;
		else
			std::cout << (boost::format(gettext("Minmax radius: %|.3f|%%")) %(minmax_size_factor*100.) ).str() << std::endl;
		if(blur_in_pixels)
			std::cout << (boost::format(gettext("Blur radius: %|d| px")) %blur_px_size ).str() << std::endl;
		else
			std::cout << (boost::format(gettext("Blur radius: %|.3f|%%")) %(blur_size_factor*100.) ).str() << std::endl;
		std::cout << (boost::format(gettext("Min range factor: %|.3f|")) %min_range_factor ).str() << std::endl;
		break;

	case 'e':
		std::cout << (boost::format(gettext("Saturation gamma: %|.3f|")) %saturation_gamma ).str() << std::endl;
		std::cout << (boost::format(gettext("Exponential factor: %|.3f|")) %exponential_factor ).str() << std::endl;
		if(minmax_in_pixels)
			std::cout << (boost::format(gettext("Minmax radius: %|d| px")) %minmax_px_size ).str() << std::endl;
		else
			std::cout << (boost::format(gettext("Minmax radius: %|.3f|%%")) %(minmax_size_factor*100.) ).str() << std::endl;
		if(blur_in_pixels)
			std::cout << (boost::format(gettext("Blur radius: %|d| px")) %blur_px_size ).str() << std::endl;
		else
			std::cout << (boost::format(gettext("Blur radius: %|.3f|%%")) %(blur_size_factor*100.) ).str() << std::endl;
		std::cout << (boost::format(gettext("Min range factor: %|.3f|")) %min_range_factor ).str() << std::endl;
		break;
	}

	boost::scoped_ptr<imaginable::Image> img(new imaginable::Image);

	std::cout << gettext("Loading: ...") << std::flush;
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
	std::cout << "\b\b\b"<<gettext("done") << std::endl;

	switch (method_name)
	{
	case 'p':
	case 'e':
		if(!minmax_in_pixels)
		{
			minmax_px_size=static_cast<imaginable::Image::Pixel>(static_cast<double>(std::min(img->width(),img->height()))*minmax_size_factor);
			std::cout << (boost::format(gettext("Minmax radius: %|d| px")) %minmax_px_size ).str() << std::endl;
		}
	// FALL THROUGH
	case 'a':
		if(!blur_in_pixels)
		{
			blur_px_size=static_cast<imaginable::Image::Pixel>(static_cast<double>(std::min(img->width(),img->height()))*blur_size_factor);
			std::cout << (boost::format(gettext("Blur radius: %|d| px")) %blur_px_size ).str() << std::endl;
		}
		break;
	}

	std::cout << gettext("Converting: ...   ") << std::flush;
	try
	{
		imaginable::rgb_to_hcy(*img.get(), false);
		switch (method_name)
		{
		case 'g':
			imaginable::tonemap_global(*img.get(), saturation_gamma, lightness_factor, &percent_printer);
			break;
		case 'a':
			imaginable::tonemap_local_average(*img.get(), saturation_gamma, blur_px_size, mix_factor/2., &percent_printer);
			break;
		case 'p':
			imaginable::tonemap_local_minmax_parabolic(*img.get(), saturation_gamma, minmax_px_size, blur_px_size, min_range_factor, &percent_printer);
			break;
		case 'e':
			imaginable::tonemap_local_minmax_exponential(*img.get(), saturation_gamma, exponential_factor, minmax_px_size, blur_px_size, min_range_factor, &percent_printer);
			break;
		}
		imaginable::hcy_to_rgb(*img.get(), false);
		std::cout << "\b\b\b\b\b\b      \b\b\b\b\b\b" << gettext("done") << std::endl;
	}
	catch(const std::exception& e)
	{
		std::cout << std::endl << (boost::format(gettext("failed: %|s|")) %e.what() ).str() << std::endl;
		return 1;
	}

	std::cout << gettext("Saving: ...") << std::flush;
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
		std::cout << "\b\b\b" << gettext("done") << std::endl;
	}
	else
		std::cout << "\b\b\b" << gettext("Nothing to save.") << std::endl;
}
