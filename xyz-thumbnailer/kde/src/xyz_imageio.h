/*
 * This file is part of kde-xyz-thumbnailer. Copyright (c) 2020 kde-xyz-thumbnailer authors.
 * https://github.com/EasyRPG/Tools - https://easyrpg.org
 *
 * kde-xyz-thumbnailer is Free/Libre Open Source Software, released under the MIT License.
 * For the full copyright and license information, please view the COPYING
 * file that was distributed with this source code.
 */

#ifndef XYZ_IMAGEIO_H
#define XYZ_IMAGEIO_H

#include <QImageIOPlugin>

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

#endif // XYZ_IMAGEIO_H
