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
#include "settings.h"
#include <QVector>
#include <QString>
#include "classifier/textonboost.h"


int main( int argc, char * argv[]){
	/**** Read the IO ****/
	if (argc<3){
		qWarning( "Usage: %s classifier_file texton_file [texton_file ...]", argv[0] );
		return 1;
	}
	QString save_filename = argv[1];
	int n_rounds = N_BOOSTING_ROUNDS;
	int n_classifiers = N_CLASSIFIERS;
	int n_thresholds = N_THRESHOLDS;
	int subsample = BOOSTING_SUBSAMPLE;
	int min_rect_size = MIN_RECT_SIZE;
	int max_rect_size = MAX_RECT_SIZE;
	
	// Declare all variables we need for both training and evaluation
	QVector< ColorImage > images;
	QVector< LabelImage > labels;
	QVector< Image<short> > textons;
	QVector< QString > names;
	
	/**** Training ****/
	qDebug("(train) Loading the database");
	loadImages( images, labels, names, TRAIN );
	images.clear();
	// Color Conversion
	qDebug("(train) Loading textons");
	
	int nTextons = argc-2;
	for( int i=2; i<argc; i++ ){
		QVector< Image<short> > tmp = loadTextons( argv[i], names );
		for( int j=0; j<tmp.size(); j++ ){
			if (j >= textons.count())
				textons.append( Image<short>(tmp[j].width(), tmp[j].height(), nTextons) );
			for( int k=0; k<tmp[j].width()*tmp[j].height(); k++ )
				textons[j][k*nTextons+i-2] = tmp[j][k];
		}
	}
	
	// Training
	qDebug("(train) Boosting");
	TextonBoost booster;
	booster.train( textons, labels, n_rounds, n_classifiers, n_thresholds, subsample, min_rect_size, max_rect_size );
	booster.save( save_filename );
}
