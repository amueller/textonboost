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

#include "colorimage.h"
#include <QImage>
#include <cmath>

/**************************/
/***       Color        ***/
/**************************/
Color::Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a) :r(r),g(g),b(b),a(a) {
}
Color::operator unsigned int() const{
	return color_;
}
Color& Color::operator=( unsigned int o ){
	color_ = o;
	return *this;
}
Color& Color::operator=( const Color& o ){
	color_ = o.color_;
	return *this;
}
ColorF Color::operator* ( float v ) const
{
	ColorF r = *this;
	return r*v;
}
ColorF& Color::operator*= ( float v )
{
	ColorF r = *this;
	return r*=v;
}
ColorF operator*( float v, const Color & c ){
	return c * v;
}
ColorF operator*( double v, const Color & c ){
	return c * (float)v;
}
ColorF operator*( const Color & c, double v ){
	return c * (float)v;
}
QDataStream & operator<<( QDataStream & s, const Color & c ){
	return s << c.color_;
}
QDataStream & operator>>( QDataStream & s, Color & c ){
	return s >> c.color_;
}

/**************************/
/***       ColorF       ***/
/**************************/
ColorF::ColorF ( float r, float g, float b, float a ) :r ( r ),g ( g ),b ( b ),a ( a )
{
}
ColorF::ColorF ( const Color& c ) :r ( c.r ),g ( c.g ),b ( c.b ),a ( c.a )
{
}
ColorF ColorF::operator+ ( const ColorF& o ) const
{
	ColorF r = *this;
	return r+=o;
}
ColorF& ColorF::operator+= ( const ColorF& o )
{
	a += o.a;
	r += o.r;
	g += o.g;
	b += o.b;
	return *this;
}
ColorF ColorF::operator- ( const ColorF& o ) const
{
	ColorF r = *this;
	return r-=o;
}
ColorF& ColorF::operator-= ( const ColorF& o )
{
	a -= o.a;
	r -= o.r;
	g -= o.g;
	b -= o.b;
	return *this;
}
ColorF ColorF::operator* ( float v ) const
{
	ColorF r = *this;
	return r*=v;
}
ColorF& ColorF::operator*= ( float v )
{
	a *= v;
	r *= v;
	g *= v;
	b *= v;
	return *this;
}
ColorF operator* ( float v, const ColorF& c )
{
	return c * v;
}
ColorF operator*( double v, const ColorF & c ){
	return c * (float)v;
}
ColorF operator*( const ColorF & c, double v ){
	return c * (float)v;
}
ColorF::operator Color() const
{
	Color r;
	r.a = round ( a );
	r.r = round ( r );
	r.g = round ( g );
	r.b = round ( b );
	return r;
}
float ColorF::squaredLength() const {
    return r*r+g*g+b*b+a*a;
}
float ColorF::length() const {
    return sqrt( squaredLength() );
}


/**************************/
/***     ColorImage     ***/
/**************************/
ColorImage::ColorImage( int w, int h ): InterpolableImage<Color>(w, h){
}
ColorImage::ColorImage(const QImage& qim) : InterpolableImage< Color >( qim.width(), qim.height() ) {
	for( int j=0; j<height_; j++ )
		for( int i=0; i<width_; i++ )
			operator()(i,j) = qim.pixel( i, j );
}
ColorImage& ColorImage::operator=(const QImage& qim) {
	QImage im = qim.rgbSwapped();
	init( im.width(), im.height() );
	for( int j=0; j<height_; j++ )
		for( int i=0; i<width_; i++ )
			operator()(i,j) = im.pixel( i, j );
	return *this;
}
ColorImage::operator QImage() const {
	QImage r( width_, height_, QImage::Format_ARGB32 );
	for( int j=0; j<height_; j++ )
		for( int i=0; i<width_; i++ )
			r.setPixel( i, j, operator()(i,j) );
	return r.rgbSwapped();
}

QDataStream & operator<<( QDataStream & s, const ColorImage & im ){
	QImage qim = im;
	qim.rgbSwapped().save( s.device(), "PNG" );
	return s;
}

QDataStream & operator>>( QDataStream & s, ColorImage & im ){
	QImage qim;
	qim.load( s.device(), "PNG" );
	im = qim.rgbSwapped();
	return s;
}
void ColorImage::load(const QString& s) {
	QImage qim;
	if (!qim.load( s ))
		qWarning("Failed to load image '%s'", qPrintable( s ) );
	*this = qim;
}
void ColorImage::save(const QString& s) const {
	QImage qim = *this;
	qim.save( s );
}
