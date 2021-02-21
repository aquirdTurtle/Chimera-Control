#include "stdafx.h"
#include "ImageLabel.h"
#include <QEvent.h>

ImageLabel::ImageLabel(QWidget* parent, Qt::WindowFlags fl) : QLabel(parent, fl) {}
ImageLabel::ImageLabel (const QString& text, QWidget* parent, Qt::WindowFlags f)
	: QLabel (text, parent, f) {};

ImageLabel::~ImageLabel () {
	QLabel::~QLabel ();
};

void ImageLabel::mouseReleaseEvent (QMouseEvent* event) {
	emit mouseReleased (event);
	QLabel::mouseReleaseEvent (event);
}

