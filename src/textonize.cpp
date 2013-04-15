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

#include "util/colorimage.h"
#include "util/colorconvertion.h"
#include "util/labelimage.h"
#include "util/util.h"
#include "feature/texton.h"
#include "feature/filterbank.h"
#include "feature/colorfeature.h"
#include "settings.h"
#include <QVector>
#include <QString>
#include "feature/locationfeature.h"
#include "feature/hogfeature.h"
#include "feature/bboxfeature.h"


int main( int argc, char * argv[]){
	/**** Read the IO ****/
	if (argc<3){
		qWarning( "Usage: %s texton_file type [params ..]", argv[0] );
		qWarning( "     type :");
		qWarning( "           FilterBank [nTextons filterbank_size]" );
		qWarning( "           Color [nTextons]" );
		qWarning( "           HoG [nTextons L/A/B]" );
		qWarning( "           Location [nTextons]" );
// 		qWarning( "           BBox [nTextons]" );
		return 1;
	}
	QString save_filename = argv[1];
	QString type = argv[2];
	type = type.toLower();
	
	int n_textons = N_TEXTONS;
	if (argc>3)
		n_textons = QString( argv[3] ).toInt();
	
	QSharedPointer<Feature> filter;
	if (type == "filterbank"){
		float filterbank_size = FILTER_BANK_SIZE;
		
		if (argc>4)
			filterbank_size = QString( argv[4] ).toFloat();
		
		filter = QSharedPointer<Feature>( new FilterBank( filterbank_size ) );
	}
	else if (type == "color"){
		filter = QSharedPointer<Feature>( new ColorFeature() );
	}
	else if (type == "location"){
		filter = QSharedPointer<Feature>( new LocationFeature() );
	}
	else if (type == "hog"){
		HogFeature::HogFeatureType type = HogFeature::L;
		if (argc>4){
			QString t = QString( argv[4] ).toLower();
			if (t=="a") type = HogFeature::A;
			if (t=="b") type = HogFeature::B;
		}
		filter = QSharedPointer<Feature>( new HogFeature(type) );
	}
	else if (type == "bbox"){
		filter = QSharedPointer<Feature>( new BBoxFeature() );
	}
	else
		qFatal( "Unknown feature %s", qPrintable( type ) );
	// Declare all variables we need for both training and evaluation
	QVector< ColorImage > images;
	QVector< Image<float> > lab_images;
	QVector< LabelImage > labels;
	QVector< QString > names;
	
	/**** Training ****/
	qDebug("(train) Loading the database");
	loadImages( images, labels, names, TRAIN );
	
	// Color Conversion
	qDebug("(train) Converting to Lab");
	lab_images = RGBtoLab( images );
	
	// Training
	qDebug("(train) Training Textons");
	Texton texton( filter, n_textons );
	texton.train( lab_images, names );
	
	
	/**** Evaluation ****/
	qDebug("(test)  Loading the database");
	loadImages( images, labels, names, ALL );
	// Color Conversion
	qDebug("(test)  Converting to Lab");
	lab_images = RGBtoLab( images );
	// Evalutation
	qDebug("(test)  Textonizing");
	QVector< Image<short> > textons = texton.textonize( lab_images, names );
	
	/**** Storing textons ****/
	saveTextons( save_filename, textons, names );
}
