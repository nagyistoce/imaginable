/* * * * * *
 *
 * Project:   Imaginable
 * Created:   2010-03-18
 * Author:    Kuzma Shapran <Kuzma.Shapran@gmail.com>
 * License:   GPLv3
 *
 * * * * * */
// $Id$

#ifndef IMAGINABLE__IMAGE_Q__INCLUDED
#define IMAGINABLE__IMAGE_Q__INCLUDED


#include "image.hpp"
#include "types.hpp"

#include <QtCore/QObject>
#include <QtCore/QStringList>


class Image_Q : public QObject, protected Image
{
Q_OBJECT
public:
	explicit Image_Q(QObject* = NULL);
	Image_Q(const Image_Q&,QObject* = NULL);

	bool init_DBus(QString);

	Q_PROPERTY(ushort maximum READ maximum WRITE setMaximum)

signals:

public slots:
	unsigned width (void) const      { return Image::width(); }
	bool  setWidth (unsigned width)  { return Image::setWidth(width); }

	unsigned height(void) const      { return Image::height(); }
	bool  setHeight(unsigned height) { return Image::setHeight(height); }

	unsigned square(void) const      { return Image::square(); }
	bool  setSize  (unsigned width,unsigned height) { return Image::setSize(width,height); }

	ushort maximum (void) const      { return Image::maximum(); }
	void setMaximum(ushort maximum)  { return Image::setMaximum(maximum); }

	bool empty  (void) const { return Image::empty  (); }
	bool hasData(void) const { return Image::hasData(); }

	void clear(void) { Image::clear(); }

	static QString colourSpaceToString(unsigned);
	unsigned colourSpace(void) const { return Image::colourSpace(); }

	static QString colourPlaneToString(int);
	QintList planes (void) const;
	bool     hasPlane   (int colourPlane) const { return Image::hasPlane(colourPlane); }
	bool     hasAlpha   (void) const { return Image::hasAlpha   (); }
	unsigned planesCount(void) const { return Image::planesCount(); }

	bool     addPlane   (int colourPlane) { return Image::addPlane   (colourPlane); }
	bool     removePlane(int colourPlane) { return Image::removePlane(colourPlane); }
	bool     movePlane  (int from,int to) { return Image::movePlane(from,to); }

	bool     planeHasName  (int colourPlane) const { return Image::planeHasName(colourPlane); }
	QString  planeName     (int colourPlane) const { return QString::fromUtf8(Image::planeName(colourPlane).c_str()); }
	bool     setPlaneName  (int colourPlane,const QString& name) { return Image::setPlaneName(colourPlane,name.toUtf8().data()); }
	bool     erasePlaneName(int colourPlane)       { return Image::erasePlaneName(colourPlane); }

	bool        hasText     (const QString& key)                      const { return Image::hasText(key.toLocal8Bit().data()); }
	QString     text        (const QString& key)                      const { return QString::fromLocal8Bit(Image::text(key.toLocal8Bit().data()).c_str()); }
	QStringList textKeys    (void)                                    const;
	bool        setText     (const QString& key,const QString& value) { return Image::setText(key.toLocal8Bit().data(),value.toLocal8Bit().data()); }
	bool        eraseText   (const QString& key)                      { return Image::eraseText(key.toLocal8Bit().data()); }
	void        clearTexts  (void)                                    { Image::clearTexts(); }

private slots:

private:

};

#endif // IMAGINABLE__IMAGE_Q__INCLUDED
