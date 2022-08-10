#pragma once

#include "../mischelpers_global.h"

#include <QGraphicsColorizeEffect>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QPainter>
#include "Common.h"

class MISCHELPERS_EXPORT CNeonEffect : public QGraphicsColorizeEffect
{
	Q_OBJECT
public:
	CNeonEffect(float blurRadius = 5.0,	int glow = 2, QObject* parent = nullptr) : QGraphicsColorizeEffect(parent) 
	{
		_blurRadius = blurRadius;
		_glow = glow;
		_hue = 0;
	}
	~CNeonEffect() {}

	int glow() const { return _glow; }
	float blurRadius() const { return _blurRadius; }
	int hue() const { return _hue; }
	QColor glowColor() const { return _color; }

public slots:
	void setGlow(int glow) { 
		if (_glow == glow)
			return;
		_glow = std::max(1, std::min(glow, 10)); 
		update();
	}
	void setBlurRadius(float radius) { 
		if (_blurRadius == radius)
			return;
		_blurRadius = std::max(1.0F, radius);
		update();
	}
	void setHue(int hue) { 
		if (_hue == hue)
			return;
		_hue = hue; 
		update();
	}
	void setGlowColor(const QColor& color) { 
		if (_color == color)
			return;
		_color = color; 
		update();
	}

protected:
	virtual void draw(QPainter* painter)
	{
		QPoint offset;
		auto pm = sourcePixmap(Qt::LogicalCoordinates, &offset, PadToEffectiveBoundingRect);
		if (pm.isNull())
			return;

		if (!_color.isValid() && _hue != 0) {
			QImage img = pm.toImage();
			for (QRgb* c = (QRgb*)img.bits(); c != (QRgb*)(img.bits() + img.sizeInBytes()); c++) {
				*c = (*c & 0xFF000000) | (change_hsv_c(*c & 0x00FFFFFF, float(_hue), 2, 1) & 0x00FFFFFF);
			}
			pm = QPixmap::fromImage(img);
		}


        // use a double sized image to increase the blur factor
		auto scaledSize = QSize(pm.width() * 2, pm.height() * 2);
		auto blurImage = QImage(scaledSize, QImage::Format_ARGB32_Premultiplied);
		blurImage.fill(0);
		QPainter blurPainter; 
		blurPainter.begin(&blurImage);
		blurPainter.drawPixmap(0, 0, pm.scaled(scaledSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
		blurPainter.end();

        // apply the blurred effect on the image
		QGraphicsBlurEffect* blur = new QGraphicsBlurEffect();
		blur->setBlurRadius(_blurRadius);
		//QGraphicsDropShadowEffect *blur = new QGraphicsDropShadowEffect;
		//blur->setColor(QColor(40,40,40,245));
		//blur->setOffset(0,10);
		//blur->setBlurRadius(_blurRadius);
		blurImage = applyEffectToImage(blurImage, blur);

		if (_color.isValid()) {
			// start the painter that will use the previous image as alpha
			QPainter tmpPainter;
			tmpPainter.begin(&blurImage);
			// using SourceIn composition mode we use the existing alpha values
			// to paint over
			tmpPainter.setCompositionMode(tmpPainter.CompositionMode_SourceIn);
			//auto color = this->color();
			QColor color = _color;
			color.setAlpha(color.alpha() * strength());
			// fill using the color
			tmpPainter.fillRect(QRect(pm.rect().left() * 2, pm.rect().top() * 2, pm.rect().width() * 2, pm.rect().height() * 2), color);
			tmpPainter.end();
		}
		

        // repeat the effect which will make it more "glowing"
		for (int g = 0; g < _glow; g++)
			painter->drawImage(0, 0, blurImage.scaled(pm.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

		QGraphicsColorizeEffect::draw(painter);
	}

	QImage applyEffectToImage(QImage src, QGraphicsEffect* effect, int extent = 0)
	{
		if (src.isNull()) return QImage();
		if (!effect) return src;
		QGraphicsScene scene;
		QGraphicsPixmapItem item;
		item.setPixmap(QPixmap::fromImage(src));
		item.setGraphicsEffect(effect);
		scene.addItem(&item);
		QImage res(src.size() + QSize(extent * 2, extent * 2), QImage::Format_ARGB32);
		res.fill(Qt::transparent);
		QPainter ptr(&res);
		scene.render(&ptr, QRectF(), QRectF(-extent, -extent, src.width() + extent * 2, src.height() + extent * 2));
		return res;
	}

	float _blurRadius;
	int _glow;
	int _hue;
	QColor _color;
};
