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


//#include <iostream>
#include <cmath>
#include <list>

#include "exception.hpp"
#include "roof.hpp"


namespace imaginable
{

typedef double (*roof_function)(double prev, double current, double step);

double roof_positive(double prev, double current, double step)
{
	return ((prev - step) > current) ? (prev - step) : current;
}

double roof_negative(double prev, double current, double step)
{
	return ((prev - step) < current) ? (prev - step) : current;
}

void roof_linear_straight(Image& img, unsigned planeName, double step, const Progress_notifier &notifier)
{
	if (!img.hasData())
		throw exception(exception::NO_IMAGE);

	if ((step == 0.0) || (step > static_cast<double>(Image::MAXIMUM)) || (step < -static_cast<double>(Image::MAXIMUM)))
		throw exception(exception::INVALID_RADIUS);

	Image::Pixel *plane=img.plane(planeName);

	roof_function roof = (step > 0.0) ? &roof_positive : &roof_negative;

	size_t w = img.width();
	size_t h = img.height();

	if (w>1)
	{
		for (size_t y=0; y<h; ++y)
		{
			notifier.update(0.00+0.25*static_cast<double>(y)/static_cast<double>(h));

			ssize_t yxs=y*w+1;
			ssize_t yxe=y*w+w-1;

			double prev = plane[yxs-1];

			for (ssize_t yxo=yxs; yxo<=yxe; ++yxo)
			{
				prev = roof(prev, static_cast<double>(plane[yxo]), step);
				plane[yxo] = static_cast<Image::Pixel>(prev);
			}
		}

		for (size_t y=0; y<h; ++y)
		{
			notifier.update(0.25+0.25*static_cast<double>(y)/static_cast<double>(h));

			ssize_t yxs=y*w+w-2;
			ssize_t yxe=y*w;

			double prev = plane[yxs+1];

			for (ssize_t yxo=yxs; yxo>=yxe; --yxo)
			{
				prev = roof(prev, static_cast<double>(plane[yxo]), step);
				plane[yxo] = static_cast<Image::Pixel>(prev);
			}
		}
	}

	if (h > 1)
	{
		for (size_t x=0; x<w; ++x)
		{
			notifier.update(0.50+0.25*static_cast<double>(x)/static_cast<double>(w));

			ssize_t yxs=w+x;
			ssize_t yxe=(h-1)*w+x;

			double prev = plane[yxs-w];

			for (ssize_t yxo=yxs; yxo<=yxe; yxo+=w)
			{
				prev = roof(prev, static_cast<double>(plane[yxo]), step);
				plane[yxo] = static_cast<Image::Pixel>(prev);
			}
		}

		for (size_t x=0; x<w; ++x)
		{
			notifier.update(0.75+0.25*static_cast<double>(x)/static_cast<double>(w));

			ssize_t yxs=(h-2)*w+x;
			ssize_t yxe=x;

			double prev = plane[yxs+w];

			for (ssize_t yxo=yxs; yxo>=yxe; yxo-=w)
			{
				prev = roof(prev, static_cast<double>(plane[yxo]), step);
				plane[yxo] = static_cast<Image::Pixel>(prev);
			}
		}
	}
	notifier.update(1.0);
}

void roof_linear_diagonal(Image& img, unsigned planeName, double step, const Progress_notifier &notifier)
{

}

void roof_linear_8(Image& img, unsigned planeName, double step, const Progress_notifier &notifier)
{
	roof_linear_straight(img, planeName, step, Scaled_progress_notifier(notifier, 0  , 0.5));
	roof_linear_diagonal(img, planeName, step, Scaled_progress_notifier(notifier, 0.5, 0.5));
}

//bool printing_enabled = false;

class Precalculated_tails : public std::map<size_t, double>
{
public:
	Precalculated_tails(void)
		: std::map<size_t, double>()
	{}

	virtual double tail(size_t distance) = 0;

	virtual double intersection(double peak1, double position1, double peak2, double position2) = 0;
};

class Parabolic_tails : public Precalculated_tails
{
public:
	Parabolic_tails(double k)
		: Precalculated_tails()
		, koeff(k)
		, koeff2(k*k)
	{}

	double tail(size_t distance)
	{
		double tail;
		Precalculated_tails::iterator P = find(distance);
		if (P == end())
		{
			double dk = static_cast<double>(distance)/koeff;
			tail = dk * dk;
			(*this)[distance] = tail;
		}
		else
			tail = P->second;
		return tail;
	}

	double intersection(double peak1, double position1, double peak2, double position2)
	{
		return ((position1 + position2) - koeff2*(peak1 - peak2)/(position1 - position2))/2.0;
	}

private:
	double koeff;
	double koeff2;
};

class Exponential_tails : public Precalculated_tails
{
public:
	Exponential_tails(double a_, size_t radius_, Image::Pixel m_)
		: Precalculated_tails()
		, a(a_)
		, radius(static_cast<double>(radius_))
		, m(static_cast<double>(m_))
	{
		m_e_a_1 = m/pow(M_E, a)-1;
		a_r = a / radius;
	}

	double tail(size_t distance)
	{
		double tail;
		Precalculated_tails::iterator P = find(distance);
		if (P == end())
		{
			tail = m_e_a_1 * (pow(M_E, (a * static_cast<double>(distance) / radius)) - 1);
			(*this)[distance] = tail;
		}
		else
			tail = P->second;
		return tail;
	}

	double intersection(double peak1, double position1, double peak2, double position2)
	{
		return log((peak1-peak2) * pow(M_E, a_r*(position1+position2)) / (m_e_a_1 * (pow(M_E, a_r*position2) - pow(M_E, a_r*position1)))) / a_r;
	}

private:
	double a;
	double radius;
	double m;
	double m_e_a_1;
	double a_r;
};

typedef struct square_line
{
	Precalculated_tails &precalculated_tails;
	Image::Pixel peak;
	size_t position;
	size_t ends;

	square_line(Precalculated_tails &precalculated_tails_, double peak_, size_t position_ = 0)
		: precalculated_tails(precalculated_tails_)
		, peak(peak_)
		, position(position_)
		, ends(0)
	{}

	double tail(size_t position_) const
	{
		return std::max(0., real_tail(position_));
	}

	double real_tail(size_t position_) const
	{
		return static_cast<double>(peak) - precalculated_tails.tail(position_ - position);
	}
} square_line;

typedef std::list <square_line> square_lines;

void print(const square_lines &lines, size_t position)
{
	for (square_lines::const_iterator I = lines.begin(); I!=lines.end(); ++I)
		std::cout << "[#" << I->position << ".." << I->ends << ':' << I->peak << '~' << I->tail(position) << ']';
	std::cout << std::endl;
}

void calc_tail(square_lines &lines, Image::Pixel &this_point, size_t &position, Precalculated_tails &precalculated_tails)
{
	++position;

//	if (printing_enabled) std::cout << "----------------------------------- #" << position << ':' << this_point << std::endl;

	double tail = lines.front().tail(position);

//	if (printing_enabled) std::cout << '~' << tail << std::endl;
	square_line new_line(precalculated_tails, this_point, position);

	if (tail <= this_point)
	{
//		if (printing_enabled) std::cout << "[x]" << std::endl;
		lines.clear();
		lines.push_back(new_line);
//		if (printing_enabled) std::cout << '=' << this_point << std::endl;
	}
	else
	{
		while (lines.front().ends == position)
		{
//			if (printing_enabled) std::cout << "-#" << lines.front().position << ':' << lines.front().peak << std::endl;
			lines.pop_front();
		}

		for (square_lines::iterator I = lines.begin(); I != lines.end(); ++I)
		{
			if (I->tail(position) < new_line.peak)
			{
//				if (printing_enabled) std::cout << "-#" << I->position << ':' << I->peak << "~" << I->tail(position) << " < " << new_line.position << ':' << new_line.peak << std::endl;

				lines.erase(I, lines.end());
				if (!lines.empty())
					lines.back().ends = 0;
				break;
			}
		}
//		if (printing_enabled) print(lines, position);

		size_t i_ends = 0;
		while (!lines.empty())
		{
			square_line& last = lines.back();
			double ends = precalculated_tails.intersection(last.peak, last.position, new_line.peak, new_line.position);
			if (ends > 0.0)
			{
				i_ends = ends;
//				if (printing_enabled) std::cout << "ends = " << ends
//					<< " [#" << last.position << ".." << last.ends << ':' << last.peak << '~' << last.tail(i_ends) << '~' << last.tail(i_ends+1) << "] <> [#" << new_line.position << ".." << new_line.ends << ':' << new_line.peak << '~' << new_line.tail(i_ends) << '~' << new_line.tail(i_ends+1) << ']' << std::endl;
				bool can_exit = true;
				if (i_ends <= position)
				{
//					if (printing_enabled) std::cout << "-#" << last.position << ':' << last.peak << std::endl;
					lines.pop_back();
					can_exit = false;
				}
				else
				{
					if (lines.size() >= 2)
					{
						square_lines::iterator I = lines.end();
						--I;
						--I;
						if (i_ends <= I->ends)
						{
//							if (printing_enabled) std::cout << "-#" << last.position << ':' << last.peak << std::endl;
							lines.pop_back();
							lines.back().ends = 0;
							can_exit = false;
						}
					}
				}
				if (can_exit || lines.empty())
				{
					break;
				}
			}
		}
		if (!lines.empty())
			lines.back().ends = i_ends;
//		if (printing_enabled) std::cout << "+#" << new_line.position << ':' << new_line.peak << std::endl;
		lines.push_back(new_line);

//		if (printing_enabled) std::cout << '~' << tail << std::endl;

		this_point = tail;
	}
//	if (printing_enabled) print(lines, position);
}

void roof_functional(Image& img, unsigned planeName, bool invert, Precalculated_tails &precalculated_tails, const Progress_notifier &notifier)
{
	if (!img.hasData())
		throw exception(exception::NO_IMAGE);

	Image::Pixel *plane=img.plane(planeName);

	size_t w = img.width();
	size_t h = img.height();

	Image::Pixel m = img.maximum();

	if (invert)
	{
		for (size_t y=0; y<h; ++y)
		{
			size_t yo = y*w;
			for (size_t x=0; x<w; ++x)
			{
				Image::Pixel &point = plane[yo + x];
				point = m - point;
			}
		}
	}
//	std::cout << "k = " << k << std::endl;

	square_lines lines;
	size_t position;

	if (w > 1)
	{
		/* */
		for (size_t y=0; y<h; ++y)
		{
			notifier.update(0.00+0.25*static_cast<double>(y)/static_cast<double>(h));

			ssize_t yxs = y*w+1;
			ssize_t yxe = y*w+w-1;
			ssize_t yxp = yxs-1;

			position = 0;
			lines.clear();
			lines.push_back(square_line(precalculated_tails, plane[yxp]));

			// std::cout << "-----------------------------------" << std::endl
			// 	<< "y = " << y << std::endl;
			// print(lines, position);
			for (ssize_t yxo=yxs; yxo<=yxe; ++yxo)
				calc_tail(lines, plane[yxo], position, precalculated_tails);
		}
		/* */

		/* */
		for (size_t y=0; y<h; ++y)
		{
			notifier.update(0.25+0.25*static_cast<double>(y)/static_cast<double>(h));

			ssize_t yxs = y*w+w-2;
			ssize_t yxe = y*w;
			ssize_t yxp = yxs+1;

			position = 0;
			lines.clear();
			lines.push_back(square_line(precalculated_tails, plane[yxp]));

			// std::cout << "-----------------------------------" << std::endl
			// 	<< "y = " << y << std::endl;
			// print(lines, position);
			for (ssize_t yxo=yxs; yxo>=yxe; --yxo)
				calc_tail(lines, plane[yxo], position, precalculated_tails);
		}
		/* */
	}

	if (h > 1)
	{
		/* */
		for (size_t x=0; x<w; ++x)
		{
			notifier.update(0.50+0.25*static_cast<double>(x)/static_cast<double>(w));

			ssize_t yxs = w+x;
			ssize_t yxe = (h-1)*w+x;
			ssize_t yxp = yxs-w;

			position = 0;
			lines.clear();
			lines.push_back(square_line(precalculated_tails, plane[yxp]));

			// std::cout << "-----------------------------------" << std::endl
			// 	<< "x = " << x << std::endl;
			// print(lines, position);
			for (ssize_t yxo=yxs; yxo<=yxe; yxo+=w)
				calc_tail(lines, plane[yxo], position, precalculated_tails);
		}
		/* */

		/* */
		for (size_t x=0; x<w; ++x)
		{
			notifier.update(0.75+0.25*static_cast<double>(x)/static_cast<double>(w));

			ssize_t yxs = (h-2)*w+x;
			ssize_t yxe = x;
			ssize_t yxp = yxs+w;

			position = 0;
			lines.clear();
			lines.push_back(square_line(precalculated_tails, plane[yxp]));
			// std::cout << "-----------------------------------" << std::endl
			// 	<< "x = " << x << std::endl;
			// print(lines, position);
			for (ssize_t yxo=yxs; yxo>=yxe; yxo-=w)
				calc_tail(lines, plane[yxo], position, precalculated_tails);
		}
		/* */
	}

	if (invert)
	{
		for (size_t y=0; y<h; ++y)
		{
			size_t yo = y*w;
			for (size_t x=0; x<w; ++x)
			{
				Image::Pixel &point = plane[yo + x];
				point = m - point;
			}
		}
	}
	notifier.update(1.0);
}

void roof_parabolic(Image& img, unsigned planeName, double k, const Progress_notifier &notifier)
{
	if (k == 0.0)
		throw exception(exception::INVALID_RADIUS);

	Parabolic_tails precalculated_tails(fabs(k));
	roof_functional(img, planeName, k < 0.0, precalculated_tails, notifier);
}

void roof_exponential(Image& img, unsigned planeName, double a, size_t radius, const Progress_notifier &notifier)
{
	if (a == 0.0)
		throw exception(exception::INVALID_RADIUS);
	if (radius == 0.0)
		throw exception(exception::INVALID_RADIUS);

	Exponential_tails precalculated_tails(fabs(a), radius, img.maximum());
	roof_functional(img, planeName, a < 0.0, precalculated_tails, notifier);
}


}
