#include "scrolltext.h"

ScrollText::ScrollText(QWidget *parent)
    : QWidget(parent)
    , scrollPos(0)
{
  staticText.setTextFormat(Qt::PlainText);

  //    setFixedHeight(fontMetrics().height()*2); //The theme sets this
  leftMargin = height() / 3;

  setSeparator("   ---   ");

  connect(&timer, &QTimer::timeout, this, &ScrollText::timer_timeout);
  timer.setInterval(50);
}

QString ScrollText::text() const
{
  return m_text;
}

void ScrollText::setText(QString text)
{
  m_text = text;
  updateText();
  update();
}

QString ScrollText::separator() const
{
  return _separator;
}

void ScrollText::setSeparator(QString separator)
{
  _separator = separator;
  updateText();
  update();
}

void ScrollText::updateText()
{
  timer.stop();

  singleTextWidth = fontMetrics().horizontalAdvance(m_text);
  scrollEnabled = (singleTextWidth > width() - leftMargin * 2);

  if (scrollEnabled)
  {
    scrollPos = -64;
    staticText.setText(m_text + _separator);
    timer.start();
  }
  else
  {
    staticText.setText(m_text);
  }

  staticText.prepare(QTransform(), font());
  wholeTextSize = QSize(fontMetrics().horizontalAdvance(staticText.text()), fontMetrics().height());
}

void ScrollText::paintEvent(QPaintEvent *)
{
  QPainter p(this);

  if (scrollEnabled)
  {
    buffer.fill(qRgba(0, 0, 0, 0));
    QPainter pb(&buffer);
    pb.setPen(p.pen());
    pb.setFont(p.font());

    int x = qMin(-scrollPos, 0) + leftMargin;
    while (x < width())
    {
      pb.drawStaticText(QPointF(x, (height() - wholeTextSize.height()) / 2), staticText);
      x += wholeTextSize.width();
    }

    // Apply Alpha Channel
    pb.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    pb.setClipRect(width() - 15, 0, 15, height());
    pb.drawImage(0, 0, alphaChannel);
    pb.setClipRect(0, 0, 15, height());
    // initial situation: don't apply alpha channel in the left half of the
    // image at all; apply it more and more until scrollPos gets positive
    if (scrollPos < 0)
    {
      pb.setOpacity(static_cast<qreal>((qMax(-8, scrollPos) + 8) / 8.0));
    }
    pb.drawImage(0, 0, alphaChannel);

    // pb.end();
    p.drawImage(0, 0, buffer);
  }
  else
  {
    p.drawStaticText(QPointF(leftMargin, (height() - wholeTextSize.height()) / 2), staticText);
  }
}

void ScrollText::resizeEvent(QResizeEvent *)
{
  // When the widget is resized, we need to update the alpha channel.

  alphaChannel = QImage(size(), QImage::Format_ARGB32_Premultiplied);
  buffer = QImage(size(), QImage::Format_ARGB32_Premultiplied);

  // Create Alpha Channel:
  if (width() > 64)
  {
    // create first scanline
    QRgb *scanline1 = reinterpret_cast<QRgb *>(alphaChannel.scanLine(0));
    for (int x = 1; x < 16; ++x)
    {
      scanline1[x - 1] = scanline1[width() - x] = qRgba(0, 0, 0, x << 4);
    }
    for (int x = 15; x < width() - 15; ++x)
    {
      scanline1[x] = qRgb(0, 0, 0);
    }
    // copy scanline to the other ones
    for (int y = 1; y < height(); ++y)
    {
      memcpy(alphaChannel.scanLine(y), scanline1, static_cast<uint>(width() * 4));
    }
  }
  else
  {
    alphaChannel.fill(qRgb(0, 0, 0));
  }

  // Update scrolling state
  bool newScrollEnabled = (singleTextWidth > width() - leftMargin);
  if (newScrollEnabled != scrollEnabled)
  {
    updateText();
  }
}

void ScrollText::timer_timeout()
{
  scrollPos = (scrollPos + 2) % wholeTextSize.width();
  update();
}
