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

#include "util.h"
#include "settings.h"
#include "colorimage.h"
#include "labelimage.h"
#include <QVector>
#include <QString>
#include <QFileInfo>

static QVector< QString > listMSRC( int type ){
	QString base_dir = MSRC_DIRECTORY;
	QVector< QString > names;
	int types[] = {TRAIN, VALID, TEST};
	const char * files[] = {MSRC_TRAIN_FILE, MSRC_VALID_FILE, MSRC_TEST_FILE};
	for( int i=0; i<3; i++ )
		if (type & types[i]){
			QFile f( files[i] );
			if (!f.open(QFile::ReadOnly))
				qFatal( "Failed to open file '%s'!", files[i] );
			while(!f.atEnd()){
				QByteArray name = f.readLine().trimmed();
				if (name.length() > 0){
					QString filename = base_dir+"/"+name;
					if(!QFile::exists( filename ))
						qWarning( "File not found '%s'", qPrintable( filename ) );
					names.append( filename );
				}
			}
		}
	return names;
}

void loadMSRC(QVector< ColorImage >& images, QVector< LabelImage >& annotations, QVector< QString > & names, int type) {
	QVector< QString > filenames = listMSRC( type );
	images.clear();
	annotations.clear();
	names.clear();
	foreach (QString name, filenames ){
		QString gtname = name;
		gtname.replace(".bmp", "_GT.bmp");
		ColorImage im;
		LabelImage gt;
		im.load( name );
		gt.load( gtname, MSRC );
		images.append( im );
		annotations.append( gt );
		names.append( QFileInfo( name ).baseName() );
	}
}

static QVector< QString > listVOC2010( int type ){
	QString base_dir = VOC2010_DIRECTORY;
	QVector< QString > names;
	int types[] = {TRAIN, VALID, TEST};
	const char * files[] = {VOC2010_TRAIN_FILE, VOC2010_VALID_FILE, VOC2010_TEST_FILE};
	for( int i=0; i<3; i++ )
		if (type & types[i]){
			QFile f( files[i] );
			if (!f.open(QFile::ReadOnly))
				qFatal( "Failed to open file '%s'!", files[i] );
			while(!f.atEnd()){
				QByteArray name = f.readLine().trimmed();
				if (name.length() > 0){
					QString filename = base_dir+"/PNGImages/"+name;
					if(!QFile::exists( filename+".png" ))
						qWarning( "File not found '%s'", qPrintable( filename ) );
					names.append( filename );
				}
			}
		}
	return names;
}

void loadVOC2010(QVector< ColorImage >& images, QVector< LabelImage >& annotations, QVector< QString > & names, int type) {
	QVector< QString > filenames = listVOC2010( type );
	images.clear();
	annotations.clear();
	names.clear();
	foreach (QString name, filenames ){
		QString pngname = name;
		QString gtname = name;
		pngname+=".png";
		gtname.replace("/PNGImages/", "/SegmentationClass/");
		gtname+=".png";
		ColorImage im;
		LabelImage gt;
		im.load( pngname );
		gt.load( gtname, VOC2010 );
		images.append( im );
		annotations.append( gt );
		names.append( QFileInfo( name ).baseName() );
	}
}

void loadImages(QVector< ColorImage >& images, QVector< LabelImage >& annotations, QVector< QString > & names, int type) {
#ifdef USE_MSRC
	loadMSRC(images, annotations, names, type);
#else
	loadVOC2010(images, annotations, names, type);    
#endif
}