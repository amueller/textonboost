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
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <QDataStream>
#include <QFile>
#include <QString>
#include <QImage>
#include <QtGlobal>

#ifdef __SSE__
#include <xmmintrin.h>
#endif

template< typename T >
class Image{
template< typename TT >
	friend QDataStream & operator<<( QDataStream & s, const Image<TT> & i );
template< typename TT >
	friend QDataStream & operator>>( QDataStream & s, Image<TT> & i );
protected:
	T * data_;
	int width_, height_, depth_;
	virtual void init( int W, int H, int D=1 ){
		if (W != width_ || H != height_ || D != depth_){
			width_ = W;
			height_ = H;
			depth_ = D;
#ifdef __SSE__
			if (data_)
				_mm_free( data_ );
			data_ = (T*) _mm_malloc( width_*height_*depth_*sizeof(T)+16, 16 );
#else
			if (data_)
				delete[] data_;
			data_ = new T[ width_*height_*depth_ ];
#endif
		}
	}
public:
	explicit Image( int w=0, int h=0, int d=1 ):width_(w),height_(h),depth_(d){
		if (width_*height_*depth_ > 0){
#ifdef __SSE__
			data_ = (T*) _mm_malloc( width_*height_*depth_*sizeof(T)+16, 16 );
#else
			data_ = new T[ width_*height_*depth_ ];
#endif
			bzero( data_, sizeof( T )*width_*height_*depth_ );
		}
		else
			data_ = NULL;
	}
	Image( const Image<T> & o ):width_(o.width_),height_(o.height_),depth_(o.depth_){
		if (width_*height_*depth_ > 0){
#ifdef __SSE__
			data_ = (T*) _mm_malloc( width_*height_*depth_*sizeof(T)+16, 16 );
			__m128i * a = (__m128i*) data_, *b = (__m128i*) o.data_;
			for( int i=0; i<width_*height_*depth_; i+=sizeof(__m128i)/sizeof(T), a++, b++ )
				*a = *b;
#else
			data_ = new T[ width_*height_*depth_ ];
			std::copy( o.data_, o.data_+width_*height_*depth_, data_ );
#endif
		}
		else
			data_ = NULL;
	}
	virtual ~Image(){
		if (data_)
#ifdef __SSE__
			_mm_free( data_);
#else
			delete [] data_;
#endif
	}
	Image & operator=( const Image & o ){
		if (width_ != o.width_ || height_ != o.height_ || depth_ != o.depth_) {
			width_  = o.width_;
			height_ = o.height_;
			depth_ = o.depth_;
			if (data_)
#ifdef __SSE__
				_mm_free( data_);
#else
				delete [] data_;
#endif
			
			if (width_*height_*depth_ > 0)
#ifdef __SSE__
				data_ = (T*) _mm_malloc( width_*height_*depth_*sizeof(T)+16, 16 );
#else
				data_ = new T[ width_*height_*depth_ ];
#endif
			else
				data_ = NULL;
		}
		if (width_*height_*depth_ > 0){
#ifdef __SSE__
			__m128i * a = (__m128i*) data_, *b = (__m128i*) o.data_;
			for( int i=0; i<width_*height_*depth_; i+=sizeof(__m128i)/sizeof(T), a++, b++ )
				*a = *b;
#else
			std::copy( o.data_, o.data_+width_*height_*depth_, data_ );
#endif
		}
		return *this;
	}
public: // data access
	int width() const{
		return width_;
	}
	int height() const{
		return height_;
	}
	int depth() const{
		return depth_;
	}
	T * data(){
		return data_;
	}
	const T * data() const{
		return data_;
	}
	T & operator[]( int i ){
		return data_[i];
	}
	const T & operator[]( int i ) const{
		return data_[i];
	}
	T & operator()( int i, int j, int k=0 ){
		Q_ASSERT( 0 <= i && i < width_ );
		Q_ASSERT( 0 <= j && j < height_ );
		Q_ASSERT( 0 <= k && k < depth_ );
		return data_[depth_*(width_*j+i)+k];
	}
	const T & operator()( int i, int j, int k=0 ) const{
		Q_ASSERT( 0 <= i && i < width_ );
		Q_ASSERT( 0 <= j && j < height_ );
		Q_ASSERT( 0 <= k && k < depth_ );
		return data_[depth_*(width_*j+i)+k];
	}
	virtual void save( const QString & name ) const{
		QFile file( name );
		if (file.open( QFile::WriteOnly )){
			QDataStream ds( &file );
			ds << *this;
		}
	}
	virtual void save( const char * name ) const{
		save( QString( name ) );
	}
	virtual void load( const QString & name ){
		QFile file( name );
		if (file.open( QFile::ReadOnly )){
			QDataStream ds( &file );
			ds >> *this;
		}
	}
	virtual void load( const char * name ){
		load( QString( name ) );
	}
	virtual void fill( const T& v ){
#ifdef __SSE__
		__m128 s = _mm_set1_ps( 0.0f ); // Ugly version of _mm_set1_ps for arbitrary types
		for( unsigned int i=0; i<sizeof(__m128)/sizeof(T); i++ )
			((T*)(&s))[i] = v;
		__m128 * a = (__m128*) data_;
		for( int i=0; i<width_*height_*depth_; i+=sizeof(__m128)/sizeof(T), a++ )
			*a = s;
#else
		for( int i=0; i<width_*height_*depth_; i++ )
			data_[ i ] = v;
#endif
	}
};

template< typename T >
class InterpolableImage: public Image< T >{
public:
	explicit InterpolableImage( int w=0, int h=0 ):Image<T>( w, h ){
	}
	using Image< T >::operator();
	virtual T operator()( float i, float j, float k=0 ) const{
		int I = i, J = j, K = k;
		int II = i+1, JJ = j+1, KK = k+1;
		float wi = i-I, wj = j-J, wk = k-K;
		if (i >= Image<T>::width_)  i = Image<T>::width_-1;
		if (j >= Image<T>::height_) j = Image<T>::height_-1;
		if (k >= Image<T>::depth_)  k = Image<T>::depth_-1;
		if (I >= Image<T>::width_)  I = Image<T>::width_-1;
		if (J >= Image<T>::height_) J = Image<T>::height_-1;
		if (K >= Image<T>::depth_)  K = Image<T>::depth_-1;
		return  (1.0-wi)*(1.0-wj)*(1.0-wk)*Image<T>::operator()(I ,J ,K )+
				(1.0-wi)*(    wj)*(1.0-wk)*Image<T>::operator()(I ,JJ,K )+
				(1.0-wi)*(1.0-wj)*(    wk)*Image<T>::operator()(I ,J ,KK)+
				(1.0-wi)*(    wj)*(    wk)*Image<T>::operator()(I ,JJ,KK)+
				(    wi)*(1.0-wj)*(1.0-wk)*Image<T>::operator()(II,J ,K )+
				(    wi)*(    wj)*(1.0-wk)*Image<T>::operator()(II,JJ,K )+
				(    wi)*(1.0-wj)*(    wk)*Image<T>::operator()(II,J ,KK)+
				(    wi)*(    wj)*(    wk)*Image<T>::operator()(II,JJ,KK);
	}
};

template< typename T >
QDataStream & operator<<( QDataStream & s, const Image<T> & im ){
	s << im.width_ << im.height_ << im.depth_;
	for( int i=0; i<im.width_*im.height_*im.depth_; i++ )
		s << im.data_[i];
	return s;
}
template< typename T >
QDataStream & operator>>( QDataStream & s, Image<T> & im ){
	int W, H, D;
	s >> W >> H >> D;
	im.init( W, H, D );
	for( int i=0; i<im.width_*im.height_*im.depth_; i++ )
		s >> im.data_[i];
	return s;
}
template<>
QDataStream & operator<<( QDataStream & s, const Image<short> & im );
template<>
QDataStream & operator>>( QDataStream & s, Image<short> & im );
template<>
QDataStream & operator<<( QDataStream & s, const Image<char> & im );
template<>
QDataStream & operator>>( QDataStream & s, Image<char> & im );

