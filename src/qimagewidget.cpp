#include <QDebug>
#include <QGestureEvent>
#include <QImage>
#include <QImageWidget/qimagewidget.h>
#include <QLabel>
#include <QPainter>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QScrollArea>
#include <QScrollBar>
#include <QTouchEvent>
#include <stdlib.h>

class QImageWidgetPrivate {

public:
  QImageWidgetPrivate() {}
  QImage image;
  bool fitToWidget;
  double scaleFactor = 1.0;
  double horizontalOffset = 0;
  double verticalOffset = 0;
  double rotationAngle = 0;
  double currentStepScaleFactor = 1;
};

QImageWidget::QImageWidget(QWidget *parent)
    : QWidget(parent), d(new QImageWidgetPrivate) {}

bool QImageWidget::fitToWidget() const { return d->fitToWidget; }

void QImageWidget::setFitToWidget(bool on) {
  if (d->fitToWidget == on)
    return;
  d->fitToWidget = on;
}

void QImageWidget::zoom(double factor, bool animated) {
  scaleImage(factor, animated);
}

void QImageWidget::reset(bool animated) {
  if (d->image.isNull())
    return;

  auto w = size().width();
  auto h = size().height();
  double iw = d->image.width();
  double ih = d->image.height();

  double scale = 1.0;

  if (iw > w || ih > h) {
    iw = w / iw;
    ih = h / ih;
    scale = std::min(iw, ih) - 0.02;
  }

  if (animated) {
    QAnimationGroup *group = new QParallelAnimationGroup();
    if (d->rotationAngle != 0) {
      QPropertyAnimation *a = new QPropertyAnimation(this, "rotationAngle");
      a->setStartValue(d->rotationAngle);
      a->setEndValue(0);
      group->addAnimation(a);
    }

    QPropertyAnimation *a = new QPropertyAnimation(this, "scaleFactor");
    a->setStartValue(d->scaleFactor);
    a->setEndValue(scale);
    group->addAnimation(a);

    connect(group, &QAnimationGroup::finished, group,
            &QAnimationGroup::deleteLater);

    group->start();
  } else {
    d->rotationAngle = 0;
    d->scaleFactor = scale;
    update();
  }
}

void QImageWidget::setImage(const QImage &image) {
  d->image = image;
  this->update();
}

QImage QImageWidget::image() const { return d->image; }

/*void QImageWidget::adjustScrollBar(QScrollBar *scrollBar, double factor) {
  scrollBar->setValue(int(factor * scrollBar->value() +
                          ((factor - 1) * scrollBar->pageStep() / 2)));
}*/

void QImageWidget::scaleImage(double factor, bool animated) {
  Q_ASSERT(!d->image.isNull());

  if (animated) {
    QPropertyAnimation *a = new QPropertyAnimation(this, "scaleFactor");
    a->setStartValue(d->scaleFactor);
    a->setEndValue(d->scaleFactor + factor);

    connect(a, &QPropertyAnimation::finished, a,
            &QPropertyAnimation::deleteLater);

    a->start();

  } else {
    d->scaleFactor = factor;
    update();
  }
  // d->scaleFactor *= factor;
  // d->imageWidget->resize(d->scaleFactor * d->imageWidget->pixmap()->size());

  // adjustScrollBar(d->scroll->horizontalScrollBar(), factor);
  // adjustScrollBar(d->scroll->verticalScrollBar(), factor);
}

void QImageWidget::resizeEvent(QResizeEvent *event) {
  QWidget::resizeEvent(event);
  auto g = this->geometry();

  if (d->fitToWidget)
    this->reset();
}

bool QImageWidget::handleNativeGestureEvents(QNativeGestureEvent *gesture) {

  switch (gesture->gestureType()) {
  case Qt::ZoomNativeGesture: {
    auto value = gesture->value();

    d->scaleFactor += value;
    if (d->scaleFactor <= 0.05) {
      d->scaleFactor = 0.05;
    } else if (d->scaleFactor >= 4.0) {
      d->scaleFactor = 4.0;
    }
  }; break;
  case Qt::RotateNativeGesture:
    d->rotationAngle += gesture->value();
    break;
  case Qt::SmartZoomNativeGesture: {
    QPropertyAnimation *a = new QPropertyAnimation(this, "scaleFactor");
    a->setDuration(500);
    a->setStartValue(d->scaleFactor);
    a->setEndValue(d->scaleFactor + gesture->value());
    connect(a, &QPropertyAnimation::finished, a,
            &QPropertyAnimation::deleteLater);
    a->start();
  };
    return true;
  default:
    return false;
  }

  this->update();
  return true;
}

bool QImageWidget::handleWheelEvents(QWheelEvent *wheel) {
  QPointF delta = wheel->pixelDelta();

  d->horizontalOffset += delta.x();
  d->verticalOffset += delta.y();

  auto iw = (d->image.width() * d->scaleFactor) / 2;
  auto ih = (d->image.height() * d->scaleFactor) / 2;

  auto w = abs((int)d->horizontalOffset);
  auto h = abs((int)d->verticalOffset);
  if (w > iw)
    d->horizontalOffset = d->horizontalOffset > 0 ? iw : -iw;
  if (h > ih)
    d->verticalOffset = d->verticalOffset > 0 ? ih : -ih;
  update();
  return true;
}

bool QImageWidget::event(QEvent *event) {
  auto totalScaleFactor = d->scaleFactor;

  switch (event->type()) {
  case QEvent::TouchBegin:
  case QEvent::TouchUpdate:
  case QEvent::TouchEnd: {
    QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
    QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();
    if (touchPoints.count() == 2) {
      // determine scale factor
      const QTouchEvent::TouchPoint &touchPoint0 = touchPoints.first();
      const QTouchEvent::TouchPoint &touchPoint1 = touchPoints.last();
      qreal currentScaleFactor =
          QLineF(touchPoint0.pos(), touchPoint1.pos()).length() /
          QLineF(touchPoint0.startPos(), touchPoint1.startPos()).length();
      if (touchEvent->touchPointStates() & Qt::TouchPointReleased) {
        // if one of the fingers is released, remember the current scale
        // factor so that adding another finger later will continue zooming
        // by adding new scale factor to the existing remembered value.
        totalScaleFactor *= currentScaleFactor;
        currentScaleFactor = 1;
      }
      d->scaleFactor = totalScaleFactor;
      scaleImage(currentScaleFactor * totalScaleFactor);
      // setTransform(QTransform().scale(totalScaleFactor * currentScaleFactor,
      //                                totalScaleFactor * currentScaleFactor));
    }
    return true;
  }
  case QEvent::Gesture: {
    QGestureEvent *gesture = static_cast<QGestureEvent *>(event);
    qDebug() << gesture;

    return true;
  }
  case QEvent::NativeGesture: {
    return this->handleNativeGestureEvents(
        static_cast<QNativeGestureEvent *>(event));
  }
  case QEvent::Wheel: {
    auto wheel = static_cast<QWheelEvent *>(event);
    return handleWheelEvents(wheel);
  }
  default:
    break;
  }
  return QWidget::event(event);
}

double QImageWidget::scaleFactor() const { return d->scaleFactor; }

double QImageWidget::rotationAngle() const { return d->rotationAngle; }

void QImageWidget::setScaleFactor(double scaleFactor) {
  if (qFuzzyCompare(d->scaleFactor, scaleFactor))
    return;

  d->scaleFactor = scaleFactor;
  emit scaleFactorChanged(d->scaleFactor);
  update();
}

void QImageWidget::setRotationAngle(double rotationAngle) {
  if (qFuzzyCompare(d->rotationAngle, rotationAngle))
    return;

  d->rotationAngle = rotationAngle;
  emit rotationAngleChanged(d->rotationAngle);
  this->update();
}

void QImageWidget::paintEvent(QPaintEvent *event) {
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);
  QImage currentImage = d->image;
  const qreal iw = currentImage.width();
  const qreal ih = currentImage.height();
  const qreal wh = height();
  const qreal ww = width();

  p.translate(ww / 2, wh / 2);
  p.translate(d->horizontalOffset, d->verticalOffset);
  p.rotate(d->rotationAngle);
  p.scale(d->currentStepScaleFactor * d->scaleFactor,
          d->currentStepScaleFactor * d->scaleFactor);
  p.translate(-iw / 2, -ih / 2);
  p.drawImage(0, 0, currentImage);
}

void QImageWidget::mouseDoubleClickEvent(QMouseEvent *event) { reset(true); }
