/*
 * This file is part of kde-xyz-thumbnailer. Copyright (c) 2020 kde-xyz-thumbnailer authors.
 * https://github.com/EasyRPG/Tools - https://easyrpg.org
 *
 * kde-xyz-thumbnailer is Free/Libre Open Source Software, released under the MIT License.
 * For the full copyright and license information, please view the COPYING
 * file that was distributed with this source code.
 */

#include "xyz_imageio.h"
#include "xyz.h"

#include <QString>
#include <QImage>
#include <zlib.h>

QImageIOHandler* XyzImageIOPlugin::create(QIODevice *device, const QByteArray &format) const {
	if (format.isNull() || format.toLower() == "xyz") {
		XyzImageIOHandler* handler = new XyzImageIOHandler();
		handler->setDevice(device);
		handler->setFormat("xyz");
		return handler;
	}

	return nullptr;
}


QImageIOPlugin::Capabilities XyzImageIOPlugin::capabilities(QIODevice *device, const QByteArray &format) const {
	if (format.isNull() && !device) {
		return {};
	}

	if (!format.isNull() && format.toLower() != "xyz") {
		return {};
	}

	if (device && !XyzImageIOHandler::canRead(device)) {
		return {};
	}

	return CanRead;
}

bool XyzImageIOHandler::canRead() const {
	return canRead(device());
}

bool XyzImageIOHandler::canRead(QIODevice* device) {
	if (!device) {
		qWarning("XyzImageIOHandler::canRead() called with 0 pointer");
		return false;
	}

	QByteArray magic = device->peek(4);
	return magic.size() == 4 && memcmp(magic.data(), "XYZ1", 4) == 0;
}

bool XyzImageIOHandler::read(QImage* image) {
	if (!image) {
		 qWarning("XyzImageIOHandler::read() called with 0 pointer");
		 return false;
	}

	qint64 fsize = device()->size();

	if (fsize <= 8 || fsize > 1024*1024*1024) {
		return false;
	}

	char* data = (char*)malloc(fsize);
	if (!data) {
		return false;
	}

	qint64 res = device()->read(data, fsize);
	if (res != fsize) {
		return false;
	}

	return XyzImage::toImage(data, fsize, *image);
}
