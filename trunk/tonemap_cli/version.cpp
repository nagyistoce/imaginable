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


#include <cstdio>

#if !defined(__linux__)
#	include <cstdlib>
#endif

#include <QtCore/QDateTime>

#include "version.hpp"


namespace version {

	unsigned major_ =
#include "version-major.inl"
	;

	unsigned minor_ =
#include "version-minor.inl"
	;

	const char revision_[] =
#include "version-revision.auto.inl"
	;

	unsigned number_ =
#include "version-number.auto.inl"
	;

	const char label_[] =
#include "version-label.inl"
#ifdef _DEBUG
	"-debug"
#endif
	;

	time_t time_ =
#include "version-time.auto.inl"
	;

	QDateTime dateTime = QDateTime::fromTime_t(time_);


	unsigned    major   (void) { return major_; }
	unsigned    minor   (void) { return minor_; }
	const char* revision(void) { return revision_; }
	unsigned    number  (void) { return number_; }
	const char* label   (void) { return label_; }
	time_t      time    (void) { return time_; }


	char *full_string_ = NULL;

	const char* full_string(void)
	{
		if (!full_string_)
		{
#ifdef __linux__
			if (asprintf(&full_string_, "%d.%d.%s.%d%s%s",
				major_,
				minor_,
				revision_,
				number_,
				(label_[0] != '-') ? "-" : "",
				label_ ) < 0)
			{
				full_string_ = NULL;
			}
#else
			int length = snprintf(full_string_, 0, "%d.%d.%s.%d%s%s",
				major_,
				minor_,
				revision_,
				number_,
				(label_[0] != '-') ? "-" : "",
				label_ );
			full_string_ = static_cast<char*>(malloc(length));
			snprintf(full_string_, length, "%d.%d.%s.%d%s%s",
				major_,
				minor_,
				revision_,
				number_,
				(label_[0] != '-') ? "-" : "",
				label_ );
#endif
		}
		return full_string_;
	}


	unsigned year   (void) { return dateTime.date().year();   }
	unsigned month  (void) { return dateTime.date().month();  }
	unsigned day    (void) { return dateTime.date().day();    }
	unsigned hour   (void) { return dateTime.time().hour();   }
	unsigned minute (void) { return dateTime.time().minute(); }
	unsigned second (void) { return dateTime.time().second(); }


	char *ubuntu_style_string_ = NULL;

	const char* ubuntu_style_string(void)
	{
		if (!ubuntu_style_string_)
		{
#ifdef __linux__
			if (asprintf(&ubuntu_style_string_, "%02d.%02d", dateTime.date().year()-2000, dateTime.date().month()) < 0)
				full_string_ = NULL;
#else
			int length = snprintf(ubuntu_style_string_, 0, "%02d.%02d", dateTime.date().year()-2000, dateTime.date().month());
			ubuntu_style_string_ = static_cast<char*>(malloc(length));
			snprintf(ubuntu_style_string_, length, "%02d.%02d", dateTime.date().year()-2000, dateTime.date().month());
#endif
		}
		return ubuntu_style_string_;
	}

	inline double ubuntu_style(void)
	{
		return static_cast<double>(dateTime.date().year()-2000) + static_cast<double>(dateTime.date().month())/100.;
	}

}
