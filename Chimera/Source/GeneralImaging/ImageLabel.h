#pragma once

#include <qlabel.h>
#include <GeneralObjects/coordinate.h>

class ImageLabel : public QLabel{ 
	Q_OBJECT
	public:
		ImageLabel (QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags ());
		ImageLabel (const QString& text, QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags ());
		~ImageLabel ();
	protected:
		void mouseReleaseEvent (QMouseEvent* event);
	Q_SIGNALS:
		void mouseReleased (QMouseEvent* event);
};