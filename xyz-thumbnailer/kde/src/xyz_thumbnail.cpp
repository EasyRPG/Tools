/*
 * This file is part of kde-xyz-thumbnailer. Copyright (c) 2020 kde-xyz-thumbnailer authors.
 * https://github.com/EasyRPG/Tools - https://easyrpg.org
 *
 * kde-xyz-thumbnailer is Free/Libre Open Source Software, released under the MIT License.
 * For the full copyright and license information, please view the COPYING
 * file that was distributed with this source code.
 */

#include "xyz_thumbnail.h"
#include "xyz.h"

#include <QString>
#include <QImage>
#include <zlib.h>

#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(XyzThumbnailCreator, "xyz_thumbnail.json")

XyzThumbnailCreator::XyzThumbnailCreator(QObject *parent, const QVariantList &args)
    : KIO::ThumbnailCreator(parent, args)
{
}

XyzThumbnailCreator::~XyzThumbnailCreator()
{
}

KIO::ThumbnailResult XyzThumbnailCreator::create(const KIO::ThumbnailRequest &request) {
	FILE* f = fopen(request.url().toLocalFile().toUtf8().data(), "rb");
	if (!f) {
		KIO::ThumbnailResult::fail();
	}

	fseek(f, 0, SEEK_END);
	size_t size = ftell(f);
	fseek(f, 0, SEEK_SET);

	if (size <= 8 || size > 1024*1024*1024) {
		fclose(f);
		KIO::ThumbnailResult::fail();
	}

	char* data = (char*)malloc(size);
	if (!data) {
		fclose(f);
		KIO::ThumbnailResult::fail();
	}

	size_t res = fread(data, size, 1, f);
	if (res != 1) {
		KIO::ThumbnailResult::fail();
	}
	fclose(f);

	QImage img;
	XyzImage::toImage(data, size, img);
	return KIO::ThumbnailResult::pass(img);
}

#include "xyz_thumbnail.moc"
