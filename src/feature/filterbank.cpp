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

#include "filterbank.h"
#include "util/image.h"
#include <QVector>
#include <cmath>

#define PI 3.14159265358979323846

/**** Convolutions ****/
static Image<float> convolveX( const Image<float> & image, const QVector<float> & kernel ){
	Image<float> res( image.width(), image.height(), image.depth() );
	res.fill( 0 );
	
	int radius = kernel.size()/2;
	for( int j=0; j<image.height(); j++ )
		for( int i=0; i<image.width(); i++ ){
			// Convolve the current element
			for( int k=0; k<kernel.count(); k++ ){
				int x = i+k-radius;
				// Boundary condition is mirror
				if (x<0) x = -x;
				if (x>=image.width()) x = 2*(image.width()-2)-x;
				
				for( int l=0; l<image.depth(); l++ )
					res(i,j,l) += kernel[k]*image(x,j,l);
			}
		}
	return res;
}
static Image<float> convolveY( const Image<float> & image, const QVector<float> & kernel ){
	Image<float> res( image.width(), image.height(), image.depth() );
	res.fill( 0 );
	
	int radius = kernel.size()/2;
	for( int j=0; j<image.height(); j++ )
		// Convolve the current row
		for( int k=0; k<kernel.count(); k++ ){
			int y = j+k-radius;
			// Boundary condition is mirror
			if (y<0) y = -y;
			if (y>=image.height()) y = 2*(image.height()-2)-y;
			for( int i=0; i<image.width(); i++ )
				for( int l=0; l<image.depth(); l++ )
					res(i,j,l) += kernel[k]*image(i,y,l);
		}
	return res;
}
static Image<float> convolveXY( const Image<float> & image, const QVector<float> & kernelX, const QVector<float> & kernelY ){
	return convolveX( convolveY( image, kernelY ), kernelX );
}
static Image<float> convolveLoG( const Image<float> & image, const QVector<float> & kernel1, const QVector<float> & kernel2 ){
	Image<float> r1 = convolveX( convolveY( image, kernel1 ), kernel2 );
	Image<float> r2 = convolveX( convolveY( image, kernel2 ), kernel1 );
	for( int i=0; i<r1.width()*r1.height()*r1.depth(); i++ )
		r1[i] += r2[i];
	return r1;
}
static QVector<float> gaussianKernel( float sigma ){
	int halfSize = ceil(3.0*sigma);
	QVector<float> kernel( 2*halfSize+1 );
	double s2 = sigma * sigma;
	double f = 1.0 / sqrt(2.0 * PI * s2);
	double w2 = 1.0 / (2.0 * s2);
	for(int i = 0; i < kernel.size(); i++)
	{
		int p = i - halfSize;
		kernel[i] = f * exp(-(p * p) * w2);
	}
	return kernel;
}
static QVector<float> gaussianDerivativeKernel( float sigma ){
	int halfSize = ceil(3.0*sigma);
	QVector<float> kernel( 2*halfSize+1 );
	double s2 = sigma * sigma;
	double f = 1.0 / sqrt(2.0 * PI * s2);
	double w = 1.0 / (s2);
	double w2 = 1.0 / (2.0 * s2);
	for(int i = 0; i < kernel.size(); i++)
	{
		int p = i - halfSize;
		kernel[i] = - p * w * f * exp(-(p * p) * w2);
	}
	return kernel;
}
static QVector<float> gaussian2ndDerivativeKernel( float sigma ){
	int halfSize = ceil(3.0*sigma);
	QVector<float> kernel( 2*halfSize+1 );
	double s2 = sigma * sigma;
	double f = 1.0 / sqrt(2.0 * PI * s2);
	double w = 1.0 / (s2);
	double w2 = 1.0 / (2.0 * s2);
	for(int i = 0; i < kernel.size(); i++)
	{
		int p = i - halfSize;
		kernel[i] = ( p * p * w * w - w ) * f * exp(-(p * p) * w2);
	}
	return kernel;
}

static Image<float> channel( const Image<float> & image, int c ){
	Image<float> r( image.width(), image.height() );
	for( int j=0; j<image.height(); j++ )
		for( int i=0; i<image.width(); i++ )
			r(i,j) = image(i,j,c);
	return r;
}
static void setChannel( Image<float> & r, const Image<float> & image, int c ){
	for( int j=0; j<image.height(); j++ )
		for( int i=0; i<image.width(); i++ )
			r(i,j,c) = image(i,j);
}

FilterBank::FilterBank(float kappa, int filters):filters_(filters) {
	// Create the kernels
	g1_ = gaussianKernel( 1*kappa );
	g2_ = gaussianKernel( 2*kappa );
	g4_ = gaussianKernel( 3*kappa );
	g8_ = gaussianKernel( 4*kappa );
	dg2_ = gaussianDerivativeKernel( 2*kappa );
	dg4_ = gaussianDerivativeKernel( 4*kappa );
	lg1_ = gaussian2ndDerivativeKernel( 1*kappa );
	lg2_ = gaussian2ndDerivativeKernel( 2*kappa );
	lg4_ = gaussian2ndDerivativeKernel( 4*kappa );
	lg8_ = gaussian2ndDerivativeKernel( 8*kappa );
}
Image< float > FilterBank::evaluate(const Image< float > lab_image, const QString& name) {
	// Get the channels
	Image<float> L = channel( lab_image, 0 );
	Image<float> a = channel( lab_image, 1 );
	Image<float> b = channel( lab_image, 2 );
	
	Image<float> res( lab_image.width(), lab_image.height(), size() );
	
	// Do the gaussian kernels
	int k=0;
	if (filters_ & GAUSSIAN){
		setChannel( res, convolveXY( L, g1_, g1_ ), k++ );
		setChannel( res, convolveXY( L, g2_, g2_ ), k++ );
		setChannel( res, convolveXY( L, g4_, g4_ ), k++ );
		setChannel( res, convolveXY( a, g1_, g1_ ), k++ );
		setChannel( res, convolveXY( a, g2_, g2_ ), k++ );
		setChannel( res, convolveXY( a, g4_, g4_ ), k++ );
		setChannel( res, convolveXY( b, g1_, g1_ ), k++ );
		setChannel( res, convolveXY( b, g2_, g2_ ), k++ );
		setChannel( res, convolveXY( b, g4_, g4_ ), k++ );
	}
	if (filters_ & DGAUSSIAN){
		setChannel( res, convolveXY( L, dg2_, g2_ ), k++ );
		setChannel( res, convolveXY( L, dg4_, g4_ ), k++ );
		setChannel( res, convolveXY( L, g2_, dg2_ ), k++ );
		setChannel( res, convolveXY( L, g4_, dg4_ ), k++ );
	}
	if (filters_ & LGAUSSIAN){
		setChannel( res, convolveLoG( L, lg1_, g1_ ), k++ );
		setChannel( res, convolveLoG( L, lg2_, g2_ ), k++ );
		setChannel( res, convolveLoG( L, lg4_, g4_ ), k++ );
		setChannel( res, convolveLoG( L, lg8_, g8_ ), k++ );
	}
	return res;
}
int FilterBank::size() const {
	return (filters_ & GAUSSIAN ? 9 : 0) + (filters_ & DGAUSSIAN ? 4 : 0) + (filters_ & LGAUSSIAN ? 4 : 0);
}
