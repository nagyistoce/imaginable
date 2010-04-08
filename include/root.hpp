/* * * * * *
 *
 * Title:     Imaginable
 * Created:   2010-02-16
 * Author:    Kuzma Shapran <Kuzma.Shapran@gmail.com>
 * License:   GPLv3
 *
 * * * * * */
// $Id: root.hpp 9 2010-03-20 09:53:22Z Kuzma.Shapran $

#ifndef IMAGINABLE__ROOT__INCLUDED
#define IMAGINABLE__ROOT__INCLUDED


#include <QtCore/QtGlobal>

#include <sys/syslog.h> // for log levels


class Image;

class Root
{
public:
	Root(void) {}
	virtual ~Root() {}

	virtual Image* image(qulonglong) =0;
	virtual bool hasImage(qulonglong) const =0;

	virtual void pluginMessage(int level,QString plugin,qulonglong Id,QString message) const =0;
};

#endif // IMAGINABLE__ROOT__INCLUDED
