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

class QImage;
struct ColorF;
struct Color{
	// Little Swig workaround. Swig doesn't like the union and struct
#ifndef SWIG
	union{
		unsigned char c[4];
		struct{
#endif
			unsigned char r, g, b, a;
#ifndef SWIG
		};
		unsigned int color_;
	};
#endif
	explicit Color( unsigned char r=0, unsigned char g=0, unsigned char b=0, unsigned char a=255 );
	operator unsigned int() const;
	Color& operator=( unsigned int o );
	Color& operator=( const Color& o );
	ColorF operator*( float v ) const;
	ColorF & operator*=( float v );
};
ColorF operator*( float v, const Color & c );
ColorF operator*( double v, const Color & c );
ColorF operator*( const Color & c, double v );
QDataStream & operator<<( QDataStream & s, const Color & c );
QDataStream & operator>>( QDataStream & s, Color & c );

struct ColorF{
	explicit ColorF(float r=0, float g=0, float b=0, float a=255);
	float r, g, b, a;
	ColorF( const Color & c );
	operator Color() const;
	ColorF operator+( const ColorF & o ) const;
	ColorF & operator+=( const ColorF & o );
	ColorF operator-( const ColorF & o ) const;
	ColorF & operator-=( const ColorF & o );
	ColorF operator*( float v ) const;
	ColorF & operator*=( float v );
	float squaredLength() const;
	float length() const;
};
ColorF operator*( float v, const ColorF & c );
ColorF operator*( double v, const ColorF & c );
ColorF operator*( const ColorF & c, double v );


class ColorImage : public InterpolableImage< Color > {
	friend QDataStream & operator<<( QDataStream & s, const ColorImage & im );
	friend QDataStream & operator>>( QDataStream & s, ColorImage & im );
public:
	explicit ColorImage( int w=0, int h=0 );
	ColorImage( const QImage & qim );
	ColorImage& operator=( const QImage & qim );
	operator QImage() const;
	virtual void load( const QString & s );
	virtual void save( const QString & s ) const;
};

QDataStream & operator<<( QDataStream & s, const ColorImage & im );
QDataStream & operator>>( QDataStream & s, ColorImage & im );
