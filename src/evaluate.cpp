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
#include "config.h"
#include <QVector>
#include <QString>
#include <QDir>
#include "classifier/textonboost.h"

#ifdef USE_TBB
#include <tbb/task_scheduler_init.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#endif

void evaluate( const TextonBoost & booster, const Image<short> & texton, const QString & save_file ){
	Image<float> r = booster.evaluate( texton );
	
	// Save the result
	QFile file( save_file );
	if (file.open( QFile::WriteOnly ) ){
		QDataStream stream( &file );
		
		// We want to write floats [saves 2x space]
		stream.setVersion( QDataStream::Qt_4_7 );
		stream.setFloatingPointPrecision( QDataStream::SinglePrecision );
		
		stream << r;
	}
	file.close();
	
}

#ifdef USE_TBB
class TBBEvaluate{
	const TextonBoost & booster;
	const QVector< Image<short> > & textons;
	const QVector<QString> & names;
	const QString & save_dir;
public:
	TBBEvaluate( const TextonBoost & booster, const QVector< Image<short> > & textons, const QVector<QString> & names, const QString & save_dir ):booster(booster), textons(textons), names(names), save_dir(save_dir){}
	void operator()( tbb::blocked_range<int> rng ) const{
		for( int i=rng.begin(); i<rng.end(); i++ ){
			qDebug("Doing Image %d", i );
			evaluate( booster, textons[i], save_dir + "/" + names[i] + ".unary" );
		}
	}
};
void evaluate_all( const TextonBoost & booster, const QVector< Image<short> > & textons, const QVector<QString> & names, const QString & save_dir ){
	tbb::parallel_for(tbb::blocked_range<int>(0, textons.size(), 1), TBBEvaluate(booster, textons, names, save_dir));
}
#else
void evaluate_all( const TextonBoost & booster, const QVector< Image<short> > & textons, const QVector<QString> & names, const QString & save_dir ){
	for( int i=0; i<textons.count(); i++ ){
		qDebug("Doing Image %d", i );
		evaluate( booster, textons[i], save_dir + "/" + names[i] + ".unary" );
	}
}
#endif
int main( int argc, char * argv[]){
	/**** Read the IO ****/
	if (argc<4){
		qWarning( "Usage: %s classifier_file texton_file save_dir", argv[0] );
		return 1;
	}
	QString boost_file = argv[1];
	QString save_dir = argv[argc-1];
	
	// Declare all variables we need for both training and evaluation
	QVector< ColorImage > images;
	QVector< LabelImage > labels;
	QVector< Image<short> > textons;
	QVector< QString > names;
	
	/**** Training ****/
	qDebug("(test) Loading the database");
	loadImages( images, labels, names, ALL );
	images.clear();
	labels.clear();
	
	// Saving memory [not having vwrender crash]
	for( int n=0; n<names.count(); n+=100 ){
		QVector< QString > cur_names;
		for( int i=n; i<n+100 && i<names.count(); i++ )
			cur_names.append( names[i] );
		
		
		qDebug("(test) Loading textons");
		int nTextons = argc-3;
		QVector< Image<short> > textons;
		for( int i=2; i<argc-1; i++ ){
			QVector< Image<short> > tmp = loadTextons( argv[i], cur_names );
			for( int j=0; j<tmp.size(); j++ ){
				if (j >= textons.count())
					textons.append( Image<short>(tmp[j].width(), tmp[j].height(), nTextons) );
				for( int k=0; k<tmp[j].width()*tmp[j].height(); k++ )
					textons[j][k*nTextons+i-2] = tmp[j][k];
			}
		}
		
		// Training
		qDebug("(test) Evaluating");
		TextonBoost booster;
		booster.load( boost_file );
		
		// Create the output directory
		QDir dir( save_dir );
		if (!dir.exists())
			dir.mkpath( dir.absolutePath() );
		
		// Do the hard work
		evaluate_all( booster, textons, cur_names, save_dir );
	}
}
