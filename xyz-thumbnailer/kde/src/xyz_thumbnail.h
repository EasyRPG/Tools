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

#include <KIO/ThumbnailCreator>

// Required by KDE ThumbCreator
class XyzThumbnailCreator : public KIO::ThumbnailCreator {
public:
	XyzThumbnailCreator(QObject *parent, const QVariantList &args);
	~XyzThumbnailCreator() override;

	KIO::ThumbnailResult create(const KIO::ThumbnailRequest &request) override;
};

#endif // XYZTHUMBNAIL_H
