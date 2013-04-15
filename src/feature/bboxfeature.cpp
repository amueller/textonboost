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

#include "bboxfeature.h"
#include <util/image.h>
#include <settings.h>
const QString cls_names[] = {"aeroplane","bicycle","bird","boat","bottle","bus","car","cat",
    "chair","cow","diningtable","dog","horse","motorbike","person","pottedplant",
    "sheep","sofa","train","tvmonitor"};

int BBoxFeature::size() const
{
	return 20;
}

Image< float > BBoxFeature::evaluate(const Image< float > lab_image, const QString& image_name)
{
	Image< float > r(lab_image.width(), lab_image.height(), size() );
	r.fill(0);
	for( int i=0; i<size(); i++){
		// Load all bboxes
		QString filename = VOC2010_BBOX_DIRECTORY+cls_names[i]+"_"+image_name;
		FILE * fp = fopen( qPrintable( filename ), "r" );
		if(!fp)
			qFatal( "BBoxFeature::evaluate : Oops BBox not found!" );
		int n;
		int t1 = fread( &n, sizeof(int), 1, fp );
		QVector<double> boxes(5*n);
		for( int j=0; j<5*n; j++ )
			t1 = fread( &boxes[j], sizeof(double), 1, fp );
		fclose( fp );
		
		// Create the features
		for (int bi = 0; bi < n; bi++) {
			int x1 = boxes[bi*5];
			int y1 = boxes[bi*5+1];
			int x2 = boxes[bi*5+2];
			int y2 = boxes[bi*5+3];
			
			for (int x = x1; x <= x2; x++)
				for (int y = y1; y <= y2; y++)
					r(x,y,i) = 1;
		}
	}
// 	qFatal( "Good bye %s", qPrintable( image_name ) );
	
	return r;
}

