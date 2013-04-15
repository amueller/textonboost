/*
    Copyright (c) 2011, Philipp Krähenbühl
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
        * Neither the name of the Stanford University nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY Philipp Krähenbühl ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL Philipp Krähenbühl BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "labelimage.h"
#include <QImage>
#include <QMap>
#include "colorimage.h"
static QMap< unsigned int, signed char > init_msrc(){
	QMap< unsigned int, signed char > color_to_id;
	color_to_id[ qRgb(0,0,0) ] = -1;
	int i = 0;
	color_to_id[ qRgb(128,0,0) ] = i++;
	color_to_id[ qRgb(0,128,0) ] = i++;
	color_to_id[ qRgb(128,128,0) ] = i++;
	color_to_id[ qRgb(0,0,128) ] = i++;
	color_to_id[ qRgb(128,0,128) ] = -1;
	color_to_id[ qRgb(0,128,128) ] = i++;
	color_to_id[ qRgb(128,128,128) ] = i++;
	color_to_id[ qRgb(64,0,0) ] = -1;
	color_to_id[ qRgb(192,0,0) ] = i++;
	color_to_id[ qRgb(64,128,0) ] = i++;
	color_to_id[ qRgb(192,128,0) ] = i++;
	color_to_id[ qRgb(64,0,128) ] = i++;
	color_to_id[ qRgb(192,0,128) ] = i++;
	color_to_id[ qRgb(64,128,128) ] = i++;
	color_to_id[ qRgb(192,128,128) ] = i++;
	color_to_id[ qRgb(0,64,0) ] = i++;
	color_to_id[ qRgb(128,64,0) ] = i++;
	color_to_id[ qRgb(0,192,0) ] = i++;
	color_to_id[ qRgb(128,64,128) ] = i++;
	color_to_id[ qRgb(0,192,128) ] = i++;
	color_to_id[ qRgb(128,192,128) ] = i++;
	color_to_id[ qRgb(64,64,0) ] = i++;
	color_to_id[ qRgb(192,64,0) ] = i++;
	return color_to_id;
}
static QMap< unsigned int, signed char > init_voc2007(){
	QMap< unsigned int, signed char > color_to_id;
	for( int i=0; i<21; i++ )
		color_to_id[i] = i;
	color_to_id[0xff] = -1;
	return color_to_id;
}

static QMap< unsigned int, signed char > init_voc2010(){
	QMap< unsigned int, signed char > color_to_id;
	for( int i=0; i<21; i++ )
		color_to_id[i] = i;
	color_to_id[0xff] = -1;
	return color_to_id;
}

static QMap< signed char, unsigned int > init_msrc_colors(){
	QMap< signed char, unsigned int > color_to_id;
	color_to_id[ -1 ] = qRgb(0,0,0);
	int i = 0;
	color_to_id[ i++ ] = qRgb(128,0,0);
	color_to_id[ i++ ] = qRgb(0,128,0);
	color_to_id[ i++ ] = qRgb(128,128,0);
	color_to_id[ i++ ] = qRgb(0,0,128);
	color_to_id[ i++ ] = qRgb(0,128,128);
	color_to_id[ i++ ] = qRgb(128,128,128);
	color_to_id[ i++ ] = qRgb(192,0,0);
	color_to_id[ i++ ] = qRgb(64,128,0);
	color_to_id[ i++ ] = qRgb(192,128,0);
	color_to_id[ i++ ] = qRgb(64,0,128);
	color_to_id[ i++ ] = qRgb(192,0,128);
	color_to_id[ i++ ] = qRgb(64,128,128);
	color_to_id[ i++ ] = qRgb(192,128,128);
	color_to_id[ i++ ] = qRgb(0,64,0);
	color_to_id[ i++ ] = qRgb(128,64,0);
	color_to_id[ i++ ] = qRgb(0,192,0);
	color_to_id[ i++ ] = qRgb(128,64,128);
	color_to_id[ i++ ] = qRgb(0,192,128);
	color_to_id[ i++ ] = qRgb(128,192,128);
	color_to_id[ i++ ] = qRgb(64,64,0);
	color_to_id[ i++ ] = qRgb(192,64,0);
	return color_to_id;
}
static QMap< signed char, unsigned int > init_voc2007_colors(){
	// TODO: Complete
	return init_msrc_colors();
}

static QMap< signed char, unsigned int > init_voc2010_colors(){
	// TODO: Complete
	return init_msrc_colors();
}

static QMap< unsigned int, signed char > MSRC_MAP = init_msrc();
static QMap< unsigned int, signed char > VOC2007_MAP = init_voc2007();
static QMap< unsigned int, signed char > VOC2010_MAP = init_voc2010();
static QMap< signed char, unsigned int > MSRC_COLORS = init_msrc_colors();
static QMap< signed char, unsigned int > VOC2007_COLORS = init_voc2007_colors();
static QMap< signed char, unsigned int > VOC2010_COLORS = init_voc2010_colors();

void LabelImage::init(const QImage& qim, LabelType type) {
	if( type == MSRC )
		init( qim, MSRC_MAP );
	else if( type == VOC2007 )
		init( qim, VOC2007_MAP );
	else if( type == VOC2010 )
		init( qim, VOC2010_MAP );
	else
		qWarning( "Unsupported Label type" );

}
void LabelImage::init(const QImage& qim, const QMap< unsigned int, signed char >& map) {
	if (qim.width() != width_ || qim.height() != height_)
		Image< signed char >::init( qim.width(), qim.height() );
	bool use_index = qim.depth() < 24;
	for( int j=0; j<qim.height(); j++ )
		for( int i=0; i<qim.width(); i++ ){
			unsigned int p = use_index ? qim.pixelIndex(i, j) : qim.pixel( i, j );
			if ( map.contains( p ) )
				(*this)(i,j) = map[ p ];
			else
				if (use_index)
					qWarning( "Color not found in map: %d !", p );
				else
					qWarning( "Color not found in map: %x !", p );
		}
}
LabelImage::LabelImage(int w, int h) : Image< signed char >( w, h ) {
}
LabelImage::LabelImage(const QImage& qim, LabelType type): Image<signed char>( qim.width(), qim.height() ) {
	init( qim, type );
}
LabelImage::LabelImage(const QImage& qim, const QMap< unsigned int, signed char >& map): Image<signed char>( qim.width(), qim.height() ) {
	init( qim, map );
}
LabelImage::LabelImage(const QImage& qim): Image<signed char>( qim.width(), qim.height() ) {
	if (qim.depth() > 8)
		qWarning( "Image format not supported! Expected 8 bit image!" );
	for( int j=0; j<height(); j++ )
		for( int i=0; i<width(); i++ )
			(*this)(i,j) = qim.pixel( i, j );
}
void LabelImage::load(const QString& s, LabelType type) {
	QImage im;
	im.load( s );
	init( im, type );
}
void LabelImage::load(const char* s, LabelType type) {
    load( QString( s ), type );
}
void LabelImage::save(const QString& s) const {
	// TODO:
	qWarning( "Not implemented!" );
}
QDataStream& operator<<(QDataStream& s, const LabelImage& im) {
	// TODO:
	qWarning( "Not implemented!" );
	return s;
}
QDataStream& operator>>(QDataStream& s, LabelImage& im) {
	// TODO:
	qWarning( "Not implemented!" );
	return s;
}
LabelImage::operator QImage() const {
	// TODO:
	qWarning( "Not implemented!" );
	return QImage();
}
LabelImage LabelImage::fromMap(const Image< float >& pmf) {
	LabelImage r(pmf.width(), pmf.height());
	for ( int j=0; j<pmf.height(); j++ )
		for ( int i=0; i<pmf.width(); i++ ) {
			int b = 0;
			for ( int k=1; k<pmf.depth(); k++ )
				if (pmf(i,j,k) > pmf(i,j,b))
					b = k;
			r(i,j) = b;
		}
	return r;
}
ColorImage LabelImage::colorize(LabelType t) {
	ColorImage r( width_, height_ );
	for( int i=0; i<width_*height_; i++ )
		if (t==MSRC)
			r[i] = MSRC_COLORS[ operator[](i) ];
		else if (t==VOC2007)
			r[i] = VOC2007_COLORS[ operator[](i) ];
		else
			r[i] = VOC2010_COLORS[ operator[](i) ];
	return r;
}
