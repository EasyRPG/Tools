/*
 * This file is part of kde-xyz-thumbnailer. Copyright (c) 2017 kde-xyz-thumbnailer authors.
 * https://github.com/EasyRPG/Tools - https://easyrpg.org
 *
 * kde-xyz-thumbnailer is Free/Libre Open Source Software, released under the MIT License.
 * For the full copyright and license information, please view the COPYING
 * file that was distributed with this source code.
 */

#ifndef XYZTHUMBNAIL_H
#define XYZTHUMBNAIL_H

#include <kio/thumbcreator.h>

class XyzImageCreator : public ThumbCreator
{
	public:
		virtual bool create(const QString &path, int width, int height, QImage &img);
		virtual Flags flags() const;
};

#endif // XYZTHUMBNAIL_H
