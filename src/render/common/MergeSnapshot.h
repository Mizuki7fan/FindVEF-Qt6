#pragma once

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

#include <QImage>
#include <algorithm> // for std::copy

void copyImage(const QImage &srcImg, QImage &destImg, int destX, int destY) {
  int width = srcImg.width();
  int height = srcImg.height();
  for (int y = 0; y < height; ++y) {
    const QRgb *srcLine =
        reinterpret_cast<const QRgb *>(srcImg.constScanLine(y));
    QRgb *destLine =
        reinterpret_cast<QRgb *>(destImg.scanLine(destY + y)) + destX;
    std::copy(srcLine, srcLine + width, destLine);
  }
}

QImage MergeLayout1x2(const QImage &img00, const QImage &img01) {
  int width = std::max(img00.width(), img01.width());
  int height = std::max(img00.height(), img01.height());

  QImage result(2 * width, height, QImage::Format_ARGB32);
  result.fill(Qt::transparent);

  copyImage(img00, result, 0 * width, 0);
  copyImage(img01, result, 1 * width, 0);

  return result;
}

QImage MergeLayout1x3(const QImage &img00, const QImage &img01,
                      const QImage &img02) {
  int width = std::max({img00.width(), img01.width(), img02.width()});
  int height = std::max({img00.height(), img01.height(), img02.height()});

  QImage result(3 * width, height, QImage::Format_ARGB32);
  result.fill(Qt::transparent);

  copyImage(img00, result, 0 * width, 0);
  copyImage(img01, result, 1 * width, 0);
  copyImage(img02, result, 2 * width, 0);

  return result;
}

QImage MergeLayout1x4(const QImage &img00, const QImage &img01,
                      const QImage &img02, const QImage &img03) {
  int width =
      std::max({img00.width(), img01.width(), img02.width(), img03.width()});
  int height = std::max(
      {img00.height(), img01.height(), img02.height(), img03.height()});

  QImage result(4 * width, height, QImage::Format_ARGB32);
  result.fill(Qt::transparent);

  copyImage(img00, result, 0 * width, 0);
  copyImage(img01, result, 1 * width, 0);
  copyImage(img02, result, 2 * width, 0);
  copyImage(img03, result, 3 * width, 0);

  return result;
}

QImage MergeLayout2x1(const QImage &img00, const QImage &img10) {
  int width = std::max(img00.width(), img10.width());
  int height = std::max(img00.height(), img10.height());

  QImage result(width, 2 * height, QImage::Format_ARGB32);
  result.fill(Qt::transparent);

  copyImage(img00, result, 0, 0 * height);
  copyImage(img10, result, 0, 1 * height);

  return result;
}

QImage MergeLayout2x2(const QImage &img00, const QImage &img01,
                      const QImage &img10, const QImage &img11) {
  int width =
      std::max({img00.width(), img01.width(), img10.width(), img11.width()});
  int height = std::max(
      {img00.height(), img01.height(), img10.height(), img11.height()});

  QImage result(2 * width, 2 * height, QImage::Format_ARGB32);
  result.fill(Qt::transparent);

  copyImage(img00, result, 0 * width, 0 * height);
  copyImage(img01, result, 1 * width, 0 * height);
  copyImage(img10, result, 0 * width, 1 * height);
  copyImage(img11, result, 1 * width, 1 * height);

  return result;
}

QImage MergeLayout2x4(const QImage &img00, const QImage &img01,
                      const QImage &img02, const QImage &img03,
                      const QImage &img10, const QImage &img11,
                      const QImage &img12, const QImage &img13) {
  int width =
      std::max({img00.width(), img01.width(), img02.width(), img03.width(),
                img10.width(), img11.width(), img12.width(), img13.width()});
  int height = std::max({img00.height(), img01.height(), img02.height(),
                         img03.height(), img10.height(), img11.height(),
                         img12.height(), img13.height()});

  QImage result(4 * width, 2 * height, QImage::Format_ARGB32);
  result.fill(Qt::transparent);

  copyImage(img00, result, 0 * width, 0 * height);
  copyImage(img01, result, 1 * width, 0 * height);
  copyImage(img02, result, 2 * width, 0 * height);
  copyImage(img03, result, 3 * width, 0 * height);
  copyImage(img10, result, 0 * width, 1 * height);
  copyImage(img11, result, 1 * width, 1 * height);
  copyImage(img12, result, 2 * width, 1 * height);
  copyImage(img13, result, 3 * width, 1 * height);

  return result;
}