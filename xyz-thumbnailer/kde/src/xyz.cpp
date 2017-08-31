/*
 * This file is part of kde-xyz-thumbnailer. Copyright (c) 2017 kde-xyz-thumbnailer authors.
 * https://github.com/EasyRPG/Tools - https://easyrpg.org
 *
 * kde-xyz-thumbnailer is Free/Libre Open Source Software, released under the MIT License.
 * For the full copyright and license information, please view the COPYING
 * file that was distributed with this source code.
 */

#include "xyz.h"

#include <QString>
#include <QImage>
#include <zlib.h>

extern "C" {
	Q_DECL_EXPORT ThumbCreator* new_creator() {
		return new XyzThumbnailCreator();
	}
}

bool XyzThumbnailCreator::create(const QString &path, int /*width*/, int /*height*/, QImage &img) {
	FILE* f = fopen(path.toUtf8().data(), "rb");
	if (!f) {
		return false;
	}

	fseek(f, 0, SEEK_END);
	size_t size = ftell(f);
	fseek(f, 0, SEEK_SET);

	if (size <= 8 || size > 1024*1024*1024) {
		fclose(f);
		return false;
	}

	char* data = (char*)malloc(size);
	if (!data) {
		fclose(f);
		return false;
	}

	size_t res = fread(data, size, 1, f);
	if (res != 1) {
		return false;
	}
	fclose(f);

	return XyzImage::toImage(data, size, img);
}

ThumbCreator::Flags XyzThumbnailCreator::flags() const {
	return None;
}

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
		return 0;
	}

	if (!format.isNull() && format.toLower() != "xyz") {
		return 0;
	}

	if (device && !XyzImageIOHandler::canRead(device)) {
		return 0;
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

bool XyzImage::toImage(char* data, size_t size, QImage &img) {
	if (strncmp((char *) data, "XYZ1", 4) != 0) {
		return false;
	}

	unsigned short w;
	memcpy(&w, &data[4], 2);
	unsigned short h;
	memcpy(&h, &data[6], 2);

	uLongf src_size = (uLongf)(size - 8);
	Bytef* src_buffer = (Bytef*)&data[8];
	uLongf dst_size = 768 + (w * h);
	std::vector<Bytef> dst_buffer(dst_size);

	int status = uncompress(&dst_buffer.front(), &dst_size, src_buffer, src_size);

	free(data);

	if (status != Z_OK) {
		return false;
	}
	const uint8_t (*palette)[3] = (const uint8_t(*)[3]) &dst_buffer.front();

	void* pixels = malloc(w * h * 4);

	if (!pixels) {
		return false;
	}

	uint8_t* dst = (uint8_t*) pixels;
	const uint8_t* src = (const uint8_t*) &dst_buffer[768];
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			uint8_t pix = *src++;
			const uint8_t* color = palette[pix];
			*dst++ = color[2];
			*dst++ = color[1];
			*dst++ = color[0];
			*dst++ = 255;
		}
	}

	QImage q((uchar*)pixels, w, h, QImage::Format_ARGB32);
	img = q;

	// pixels owned by QImage

	return !img.isNull();
}
