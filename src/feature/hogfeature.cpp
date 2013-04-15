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

#include "hogfeature.h"
#include "util/image.h"
#include <cmath>

const int nAngleBins = 9;
const int cellSize = 6;
// Total number of cells is nCells * nCells
const int nCells = 3;

int HogFeature::size() const {
	return nAngleBins*nCells*nCells;
}

Image< float > HogFeature::evaluate(const Image< float > lab_image, const QString& name) {
	Image< float > og( lab_image.width(), lab_image.height(), nAngleBins );
	og.fill( 0.0 );
	int c = 0;
	if (type_ == A) c = 1;
	if (type_ == B) c = 2;
	// Compute the per ppixel gradient and store it's origentation and magnitude
	for( int j=0; j<lab_image.height(); j++ ){
		int j1 = j+1<lab_image.height() ? j+1 : j;
		int j2 = j>0 ? j-1 : j;
		for( int i=0; i<lab_image.width(); i++ ){
			int i1 = i+1<lab_image.width() ? i+1 : i;
			int i2 = i>0 ? i-1 : i;
			float dx = (lab_image(i1, j, c) - lab_image(i2, j, c)) / (i1-i2);
			float dy = (lab_image(i, j1, c) - lab_image(i, j2, c)) / (j1-j2);
			float a = atan2( dx, dy );
			float m = sqrt( dx*dx + dy*dy );
			// Signed binning
			if (a < 0){
				a += M_PI;
				m = -m;
			}
			a = nAngleBins*a / M_PI;
			if (a >= nAngleBins)
				a = nAngleBins-1;
			// Linear interpolation
			int a1 = a, a2 = a+1;
			float w1 = a2-a, w2 = a-a1;
			if (a2 >= nAngleBins)
				a2 = 0;
			og(i,j,a1) += w1*m;
			og(i,j,a2) += w2*m;
		}
	}
	
	// Sum upto cellSize (it's numerically more stable than an integral image)
	Image< float > tmp( lab_image.width(), lab_image.height(), nAngleBins );
	// Sum along x
	tmp.fill( 0.0 );
	const int halfCellSize = cellSize/2;
	for( int j=0; j<lab_image.height(); j++ ){
		// Current running sum
		float cnt = 0;
		float sum[nAngleBins];
		for( int k=0; k<nAngleBins; k++ )
			sum[k] = 0;
		
		for( int i=-halfCellSize; i<lab_image.width(); i++ ){
			if (i+halfCellSize < lab_image.width()){
				for( int k=0; k<nAngleBins; k++ )
					sum[k] += og(i+halfCellSize,j,k);
				cnt+= 1;
			}
				
			if (i >= 0)
				for( int k=0; k<nAngleBins; k++ )
					tmp(i,j,k) = sum[k] / cnt;
			
			if (i-halfCellSize >= 0){
				for( int k=0; k<nAngleBins; k++ )
					sum[k] -= og(i-halfCellSize,j,k);
				cnt-= 1;
			}
		}
	}
	// Sum along y [not very cache efficient, but who cares]
	og.fill( 0.0 );
	for( int i=0; i<lab_image.width(); i++ ){
		// Current running sum
		float cnt = 0;
		float sum[nAngleBins];
		for( int k=0; k<nAngleBins; k++ )
			sum[k] = 0;
		
		for( int j=-halfCellSize; j<lab_image.height(); j++ ){
			if (j+halfCellSize < lab_image.height()){
				for( int k=0; k<nAngleBins; k++ )
					sum[k] += tmp(i,j+halfCellSize,k);
				cnt+= 1;
			}
				
			if (j >= 0)
				for( int k=0; k<nAngleBins; k++ )
					og(i,j,k) = sum[k] / cnt;
			
			if (j-halfCellSize >= 0){
				for( int k=0; k<nAngleBins; k++ )
					sum[k] -= tmp(i,j-halfCellSize,k);
				cnt-= 1;
			}
		}
	}
	
	// Normalize each cell [Option 1: by it's size, Option 2: by it's norm]
// 	for( int j=0; j<lab_image.height(); j++ )
// 		for( int i=0; i<lab_image.width(); i++ ){
// 			float l2 = 0;
// 			for( int k=0; k<nAngleBins; k++ )
// 				l2 += og(i,j,k)*og(i,j,k);
// 			l2 = 1.0f / (sqrt(l2) + 0.00000001);
// 			for( int k=0; k<nAngleBins; k++ )
// 				og(i,j,k) *= l2;
// 			// Alternative normalize by 1/((min(i,width-i-1,halfCellSize)+halfCellSize)*(min(j,height-j-1,halfCellSize)+halfCellSize))
// 		}
	
	// Build the final descriptor
	Image< float > res( lab_image.width(), lab_image.height(), size() );
	const int blockCenterSize = (nCells-1)*cellSize;
	const int halfBlockCenterSize = blockCenterSize/2;
	for( int j=0; j<lab_image.height(); j++ )
		for( int i=0; i<lab_image.width(); i++ ){
			for( int jj=0; jj<nCells; jj++ )
				for( int ii=0; ii<nCells; ii++ ){
					const int cid = jj*nCells + ii;
					int x = i+ii*cellSize - halfBlockCenterSize;
					int y = j+jj*cellSize - halfBlockCenterSize;
					if (x<0) x=0; else if (x >= lab_image.width())  x=lab_image.width()-1;
					if (y<0) y=0; else if (y >= lab_image.height()) y=lab_image.height()-1;
					
					for( int k=0; k<nAngleBins; k++ )
						res(i,j,cid*nAngleBins+k) = og(x,y,k);
				}
			
			// Normalize the HoG histogram
			float l2 = 0;
			for( int k=0; k<res.depth(); k++ )
				l2 += res(i,j,k)*res(i,j,k);
			l2 = 1.0f / (sqrt(l2) + 0.00000001);
			for( int k=0; k<res.depth(); k++ )
				res(i,j,k) *= l2;
		}
	return res;
}
HogFeature::HogFeature(HogFeature::HogFeatureType type) :type_(type) {
}

