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
#include <QtGui>
#include "classifier/textonboost.h"


int main( int argc, char * argv[]){
	/**** Read the IO ****/
	if (argc<2){
		qWarning( "Usage: %s texton_file", argv[0] );
		return 1;
	}
	// Declare all variables we need for both training and evaluation
	QVector< ColorImage > images;
	QVector< LabelImage > labels;
	QVector< QVector< Image<short> > > textons;
	QVector< QString > names;
	
	/**** Training ****/
	qDebug("(train) Loading the database");
	loadImages( images, labels, names, TEST );
	// Color Conversion
	qDebug("(train) Loading textons");
	
	for ( int k=1; k<argc; k++ )
		textons.append( loadTextons( argv[k], names ) );
	QApplication app( argc, argv );
	QScrollArea scrollarea;
	QWidget * widget = new QWidget;
	QGridLayout * layout = new QGridLayout;
	QVector< QRgb > colors;
	for( int i=0; i<5000; i++ )
		colors.append( qRgb(random()&0xff,random()&0xff,random()&0xff) );
	for( int k=0; k<images.count() && k<10; k++ ){
		QLabel * lim = new QLabel;
		lim->setPixmap( QPixmap::fromImage(images[k]) );
		layout->addWidget(lim, k, 0 );
		
		for ( int l=0; l<textons.count(); l++ ){
			QLabel * ltx = new QLabel;
			QImage tx = images[k];
			for( int j=0; j<tx.height(); j++ )
				for( int i=0; i<tx.width(); i++ )
					tx.setPixel(i,j,colors[textons[l][k](i,j)]);
			ltx->setPixmap( QPixmap::fromImage( tx ) );
			layout->addWidget(ltx, k, l+1 );
		}
	}
	widget->setLayout( layout );
	scrollarea.setWidget( widget );
	scrollarea.show();
	return app.exec();
}
