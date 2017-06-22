#include <QApplication>
#include <QImageWidget/qimagewidget.h>

int main(int argc, char **argv) {

  QApplication app(argc, argv);

  QImageWidget w;
  w.resize(500, 400);
  w.setFitToWidget(true);
  w.show();

  auto p = QPixmap(QString::fromUtf8("/Users/rasmus/Desktop/test.png"));
  w.setImage(p.toImage());
  w.reset();
  return app.exec();
}
