/*
 * This file is part of kde-xyz-thumbnailer. Copyright (c) 2020 kde-xyz-thumbnailer authors.
 * https://github.com/EasyRPG/Tools - https://easyrpg.org
 *
 * kde-xyz-thumbnailer is Free/Libre Open Source Software, released under the MIT License.
 * For the full copyright and license information, please view the COPYING
 * file that was distributed with this source code.
 */

#ifndef XYZTHUMBNAIL_H
#define XYZTHUMBNAIL_H

#include <kio/thumbcreator.h>
#include <QImageIOPlugin>

// Required by KDE ThumbCreator
class XyzThumbnailCreator : public ThumbCreator {
public:
	virtual bool create(const QString &path, int width, int height, QImage &img) override;
	Flags flags() const override;
};

// Required by QImageIOPlugin
class XyzImageIOPlugin : public QImageIOPlugin {
	Q_OBJECT
	Q_CLASSINFO("author", "kde-xyz-thumbnailer authors")
	Q_CLASSINFO("url", "https://github.com/EasyRPG/Tools")
	Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "qxyz.json")

public:
	Capabilities capabilities(QIODevice *device, const QByteArray &format) const override;
	QImageIOHandler* create(QIODevice *device, const QByteArray &format) const override;
};

class XyzImageIOHandler : public QImageIOHandler {
public:
	bool canRead() const override;
	static bool canRead(QIODevice *device);
	bool read(QImage* image) override;
};

// Shared code for creating a XYZ QImage
namespace XyzImage {
	bool toImage(char* xyz_buf, size_t size, QImage &img);
}

#endif // XYZTHUMBNAIL_H
