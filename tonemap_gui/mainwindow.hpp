/*************
**
** Project:      Imaginable :: tonemap (GUI)
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


#ifndef IMAGINABLE__TONEMAP_GUI__MAINWINDOW__INCLUDED
#define IMAGINABLE__TONEMAP_GUI__MAINWINDOW__INCLUDED


#include <imaginable/image.hpp>

#include <QtCore/QTimer>

#include "ui_mainwindow.h"


class QProgressBar;
class QLabel;
class Notification_signaller;

class MainWindow : public QMainWindow, private Ui::MainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = NULL);

	bool loadFile(QString);

public slots:
	void fileOpen(void);
	void fileSave(void);
	void fileSaveAs(void);

	void zoomIn(void);
	void zoomOut(void);
	void zoomOne(void);
	void zoomToFit(void);

	void horizontallySlided(int);
	void verticallySlided(int);

	void showOriginal(bool);

	void previewResized(int,int);
	void previewShifted(int,int);
	void previewImageDropped(QImage);
	void previewUrlDropped(QString);

	void setSaturation(int);
	void setLightness(int);
	void setMinMax(int);
	void setBlur(int);
	void setMix(int);
	void resetSliders(void);

	void progress_updated(double);

	void updateTimeout(void);

	void methodGlobal(bool);
	void methodLocalAverage(bool);
	void methodLocalMinmaxParabolic(bool);
	void methodLocalMinmaxExponential(bool);

private:
	QLabel *current_zoom;
	QProgressBar *progress_bar;

	QString last_user_dir;
	QString open_filters;
	QString save_filters;
	QString safe_as_file_name;

	enum {
		ZOOM_CUSTOM,
		ZOOM_ONE,
		ZOOM_TO_FIT
	} current_zoom_type;
	double zoom;

	size_t scaled_width;
	size_t scaled_height;

	size_t less_side;
	size_t minmax_in_pixels;
	size_t blur_in_pixels;

	size_t less_scaled_side;
	size_t scaled_minmax_in_pixels;
	size_t scaled_blur_in_pixels;

	size_t m_lock_update;

	imaginable::SharedImage original_image;
	imaginable::SharedImage scaled_image;
	imaginable::SharedImage tonemapped_scaled_image;

	double min_zoom;


//	static QPixmap image_to_pixmap(const imaginable::Image&);
	static QImage image_to_qimage(const imaginable::Image&);

// scale      move|blur      sat|mix
// scale |-> precrop (+blur) -> tonemap -> crop -> preview
//       :
//        -> crop > preview original

	enum {
		UPDATE_SCALE   = 1<<2,
		UPDATE_TONEMAP = 1<<1,
		UPDATE_VIEWS   = 1<<0
	};
	uint m_update_flags;
	uint m_prev_update_flags;

	QTimer update_timer;

	void imageLoaded(void);

	void update_scale  (void);
	void update_tonemap(void);
	void update_views  (void);

	Notification_signaller *notification_signaller;

	void restrict_zoom_buttons(void);

	enum {
		METHOD_GLOBAL,
		METHOD_LOCAL_AVERAGE,
		METHOD_LOCAL_MINMAX_PARABOLIC,
		METHOD_LOCAL_MINMAX_EXPONENTIAL
	} current_method;

	void tonemap(imaginable::Image&, size_t minmax_in_pixels, size_t blur_in_pixels);
};

#endif // IMAGINABLE__TONEMAP_GUI__MAINWINDOW__INCLUDED
