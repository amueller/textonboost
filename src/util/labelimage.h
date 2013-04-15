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

#pragma once
#include "image.h"

enum LabelType{
	MSRC=0,
	VOC2007,
	VOC2010,
};

template<class Key, class T >
class QMap;
class QImage;
class ColorImage;
class LabelImage: public Image< signed char >
{
	void init( const QImage& qim, const QMap< unsigned int, signed char >& map );
	void init( const QImage& qim, LabelType type );
public:
	explicit LabelImage(int w = 0, int h = 0);
	LabelImage( const QImage & qim, LabelType type );
	LabelImage( const QImage& qim, const QMap< unsigned int, signed char >& map );
	LabelImage( const QImage & qim );
	operator QImage() const;
	static LabelImage fromMap( const Image<float> & pmf );
	ColorImage colorize( LabelType t=MSRC );
	virtual void load( const char * s, LabelType type );
	virtual void load( const QString & s, LabelType type );
	virtual void save( const QString & s ) const;
};

QDataStream & operator<<( QDataStream & s, const LabelImage & im );
QDataStream & operator>>( QDataStream & s, LabelImage & im );
