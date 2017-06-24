#pragma once
#include <QWidget>

class QImageWidgetPrivate;
class QImage;
class QScrollBar;
class QNativeGestureEvent;
class QImageWidget : public QWidget {

  Q_OBJECT

public:
  explicit QImageWidget(QWidget *parent = 0);

  Q_PROPERTY(double scaleFactor READ scaleFactor WRITE setScaleFactor NOTIFY
                 scaleFactorChanged)
  Q_PROPERTY(double rotationAngle READ rotationAngle WRITE setRotationAngle
                 NOTIFY rotationAngleChanged)

  bool fitToWidget() const;
  void setFitToWidget(bool on);

  void zoom(double factor, bool animated = false);
  void reset(bool animated = false);

  void setImage(const QImage &image);
  QImage image() const;

public:
  // QWidget interface
  double scaleFactor() const;

  double rotationAngle() const;

public slots:
  void setScaleFactor(double scaleFactor);

  void setRotationAngle(double rotationAngle);

signals:
  void scaleFactorChanged(double scaleFactor);

  void rotationAngleChanged(double rotationAngle);

protected:
  void paintEvent(QPaintEvent *event);
  void mouseDoubleClickEvent(QMouseEvent *event);
  void resizeEvent(QResizeEvent *event);
  bool handleNativeGestureEvents(QNativeGestureEvent *gesture);
  bool handleWheelEvents(QWheelEvent *wheel);
  bool event(QEvent *event);

private:
  // void adjustScrollBar(QScrollBar *scrollBar, double factor);
  void scaleImage(double factor, bool animated = false);
  QImageWidgetPrivate *d;
};
