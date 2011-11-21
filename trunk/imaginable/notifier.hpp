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


#ifndef IMAGINABLE__NOTIFIER__INCLUDED
#define IMAGINABLE__NOTIFIER__INCLUDED


namespace imaginable
{
	class Progress_notifier
	{
	public:
		Progress_notifier(void)
		{}

		virtual ~Progress_notifier()
		{}

		virtual void update(double) const = 0;
	};


	class Dont_notify : public Progress_notifier
	{
	public:
		Dont_notify(void)
			: Progress_notifier()
		{}

		virtual ~Dont_notify()
		{}

		virtual void update(double) const
		{}
	};

	extern Dont_notify dont_notify;


	class Stacked_progress_notifier : public Progress_notifier
	{
	public:
		Stacked_progress_notifier(const Progress_notifier &notifier)
			: Progress_notifier()
			, m_notifier(notifier)
		{}

		virtual ~Stacked_progress_notifier()
		{}

	protected:
		const Progress_notifier &m_notifier;
	};


	class Timed_progress_notifier : public Stacked_progress_notifier
	{
	public:
		Timed_progress_notifier(const Progress_notifier &notifier, double timeout=0.333)
			: Stacked_progress_notifier(notifier)
			, m_timeout(timeout)
			, m_last_update(0)
		{}

		virtual ~Timed_progress_notifier()
		{}

		virtual void update(double value) const
		{
			double current = this->now();
			if (current - m_last_update > m_timeout)
			{
				m_last_update = current;
				m_notifier.update(value);
			}
		}

	protected:
		virtual double now(void) const;

	private:
		mutable double m_timeout;
		mutable double m_last_update;
	};


	class Scaled_progress_notifier : public Stacked_progress_notifier
	{
	public:
		Scaled_progress_notifier(const Progress_notifier &notifier, double offset, double scale)
			: Stacked_progress_notifier(notifier)
			, m_offset(offset)
			, m_scale(scale)
		{}

		virtual ~Scaled_progress_notifier()
		{}

		virtual void update(double value) const
		{
			m_notifier.update(m_offset + m_scale * value);
		}

	private:
		const double m_offset;
		const double m_scale;
	};
}

#endif // IMAGINABLE__NOTIFIER__INCLUDED
