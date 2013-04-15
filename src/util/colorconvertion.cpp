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

#include "colorconvertion.h"
#include "image.h"
#include "colorimage.h"
#include <QVector>
#include <cmath>

Image< float > RGBtoLab ( const ColorImage& cim )
{
	Image<float> r( cim.width(), cim.height(), 3 );
	// See http://en.wikipedia.org/wiki/Lab_color_space
	const double Xn = 0.950456, Yn = 1.000, Zn = 1.088854;
	const double t=0.008856;
	for( int j=0; j<cim.height(); j++ )
		for( int i=0; i<cim.width(); i++ ){
			// Load RGB
			double R = cim(i,j).r / 255.0;
			double G = cim(i,j).g / 255.0;
			double B = cim(i,j).b / 255.0;
			
			// Convert to XYZ
			double X =  0.412453 * R + 0.357580 * G + 0.180423 * B;
			double Y =  0.212671 * R + 0.715160 * G + 0.072169 * B;
			double Z =  0.019334 * R + 0.119193 * G + 0.950227 * B;
			
			// Conver to Lab
			double L,a,b;
			double x = X / Xn, y = Y / Yn, z = Z / Zn;
			if ( y > t )
				L = 116 * pow( y, 1.0 / 3.0 ) - 16.0;
			else
				L = 903.3 * y;
			
			// Compute f(t)
			if ( x > t )
				x = pow( x, 1.0 / 3.0 );
			else
				x = 7.787037 * x - 0.137931;
			
			if ( y > t )
				y = pow( y, 1.0 / 3.0 );
			else
				y = 7.787037 * y - 0.137931;
			
			if ( z > t )
				z = pow( z, 1.0 / 3.0 );
			else
				z = 7.787037 * z - 0.137931;
			
			a = 500.0*( x - y );
			b = 200.0*( y - z );
			
			// Store the value
			r(i,j,0) = L;
			r(i,j,1) = a;
			r(i,j,2) = b;
		}
	return r;
}
QVector< Image< float > > RGBtoLab ( const QVector< ColorImage >& cim )
{
	QVector< Image< float > > r;
	for( int i=0; i<cim.size(); i++ )
		r.append( RGBtoLab( cim[i] ) );
	return r;
}

Image< float > RGBtoLuv ( const ColorImage& cim )
{
	Image<float> r( cim.width(), cim.height(), 3 );
	// See http://en.wikipedia.org/wiki/Luv_color_space
	const double Yn = 1.000, ur = 0.19783303699678276, vr = 0.46833047435252234;
	const double t=0.008856;
	for( int j=0; j<cim.height(); j++ )
		for( int i=0; i<cim.width(); i++ ){
			// Load RGB
			double R = cim(i,j).r / 255.0;
			double G = cim(i,j).g / 255.0;
			double B = cim(i,j).b / 255.0;
			
			// Convert to XYZ
			double X =  0.412453 * R + 0.357580 * G + 0.180423 * B;
			double Y =  0.212671 * R + 0.715160 * G + 0.072169 * B;
			double Z =  0.019334 * R + 0.119193 * G + 0.950227 * B;
			
			// Conver to Luv
			double L;
			double uu = 4*X / (X+15*Y+3*Z), vv = 9*Y / (X+15*Y+3*Z), y = Y / Yn;
			if ( y > t )
				L = 116 * pow( y, 1.0 / 3.0 ) - 16.0;
			else
				L = 903.3 * y;
			
			// Store the value
			r(i,j,0) = L;
			r(i,j,1) = 13*L*(uu-ur);
			r(i,j,2) = 13*L*(vv-vr);
		}
	return r;
}
QVector< Image< float > > RGBtoLuv ( const QVector< ColorImage >& cim )
{
	QVector< Image< float > > r;
	for( int i=0; i<cim.size(); i++ )
		r.append( RGBtoLuv( cim[i] ) );
	return r;
}
