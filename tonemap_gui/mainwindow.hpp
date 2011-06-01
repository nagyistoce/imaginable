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

class MainWindow : public QMainWindow, private Ui::MainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = NULL);

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

	void setSaturation(int);
	void setBlur(int);
	void setMix(int);
	void resetSliders(void);

	void updateTimeout(void);

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
	size_t blur_in_pixels;

	size_t less_scaled_side;
	size_t scaled_blur_in_pixels;

	size_t m_lock_update;

	imaginable::Image original_image;
	imaginable::Image scaled_image;
	imaginable::Image tonemapped_scaled_image;

	static QPixmap image_to_pixmap(const imaginable::Image&);

	enum {
		UPDATE_ALL     = 1<<2,
		UPDATE_TONEMAP = 1<<1,
		UPDATE_PREVIEW = 1<<0
	};
	uint m_update_flags;

	QTimer update_timer;

	void update_all(void);
	void update_tonemap(void);
	void update_preview(void);

	void tonemap_notification(float);
};

#endif // IMAGINABLE__TONEMAP_GUI__MAINWINDOW__INCLUDED
