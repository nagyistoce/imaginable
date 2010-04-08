/* * * * * *
 *
 * Title:     Imaginable
 * Created:   2010-03-06
 * Author:    Kuzma Shapran <Kuzma.Shapran@gmail.com>
 * License:   GPLv3
 *
 * * * * * */
// $Id$

#ifndef IMAGINABLE__OPTIONS__INCLUDED
#define IMAGINABLE__OPTIONS__INCLUDED


#include <QtCore/QMap>
#include <QtCore/QStringList>
#include <QtCore/QVariant>


class Options : public QObject
{
	Q_OBJECT

public:
	Options(QObject* =NULL);

	void setFlag(const char* name) { setProperty(name,QVariant(false)); }

	void setAlias(QString alias,QString option);

	void setInfo(QString option,QString info);


	bool parse(QStringList);
	bool parsed(void) const { return m_parsed; }

	QString info(void) const { makeInfo(); return m_info; }


	bool flag(const char* name) const { return property(name).toBool(); }

	QStringList unnamed(void) const
	{ return m_optionUnnamed; }

protected:
	QMap<QString,QString> m_optionAlias;
	QMap<QString,QString> m_optionInfo;

	void makeInfo(void) const;
	mutable QString m_info;

	QStringList m_optionUnnamed;

	bool m_parsed;
};

#endif // IMAGINABLE__OPTIONS__INCLUDED
