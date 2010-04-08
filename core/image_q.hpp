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

#include <QtCore/QObject>
#include <QtCore/QStringList>


class Root_Q;
class Image_mainAdaptor;

class Image_Q : public Image
{
Q_OBJECT
public:
	Image_Q(QObject* = NULL);
	~Image_Q();

	bool init(QString);

	Q_PROPERTY(ushort maximum READ maximum WRITE setMaximum)

signals:
	void longProcessingStarted(void);
	void percentChanged(double);
	void longProcessingFinished(void);

public slots:
	ushort maximum (void) const      { return Image::maximum(); }
	void setMaximum(ushort maximum)  { return Image::setMaximum(maximum); }

	static QString colourSpaceToString(int);

	static QString colourPlaneToString(int);

private:
	friend class Root_Q;

	QString m_DBusIFaceNodeName;
	QString m_DBusIFaceImageNodeName;
	Image_mainAdaptor* m_DBusIFaceMain;

	bool set_DBus_main(void);
	void hide_DBus_main(void);


	virtual void onSetBusy(void);
	virtual void onSetPercent(void);
};

#endif // IMAGINABLE__IMAGE_Q__INCLUDED
