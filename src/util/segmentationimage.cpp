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

#include "segmentationimage.h"
#include <QSet>
#include <QVector>
#include <QImage>
#include <QBuffer>

struct Edge{
	int a_, b_;
	Edge( int a=0, int b=0 ):a_(a),b_(b){
	}
	bool operator==( const Edge & o ) const{
		return a_ == o.a_ && b_ == o.b_;
	}
};
static uint qHash( const Edge & e ){
	return (e.a_ * 13 + e.b_*7) % 100003;
}

SegmentationImage::SegmentationImage(int w, int h) : Image< unsigned short >( w, h ) {
}
QVector< QVector< int > > SegmentationImage::adjList() const {
	// Build the adjacency graph
	QSet< Edge > edges;
	int M=0;
	for( int j=0; j<height(); j++ )
		for( int i=0; i<width(); i++ ){
			if (i && operator()(i,j)<operator()(i-1,j))
				edges.insert( Edge(operator()(i,j), operator()(i-1,j)) );
			if (i && operator()(i-1,j)<operator()(i,j))
				edges.insert( Edge(operator()(i-1,j), operator()(i,j)) );
			if (j && operator()(i,j-1)<operator()(i,j))
				edges.insert( Edge(operator()(i,j-1), operator()(i,j)) );
			if (j && operator()(i,j)<operator()(i,j-1))
				edges.insert( Edge(operator()(i,j), operator()(i,j-1)) );
			if (M <= operator()(i,j))
				M = operator()(i,j)+1;
		}
	
	QVector< QVector< int > > adj_list( M );
	foreach( Edge e, edges ){
		adj_list[e.a_].append( e.b_ );
		adj_list[e.b_].append( e.a_ );
	}
	return adj_list;
}

QDataStream& operator<<(QDataStream& s, const SegmentationImage& im) {
    QImage img( im.width(), im.height(), QImage::Format_ARGB32 );
	for( int j=0; j<img.height(); j++ )
		for( int i=0; i<img.width(); i++ )
			img.setPixel(i,j,im(i,j));
	QByteArray ba;
	QBuffer buffer( &ba );
	buffer.open(QIODevice::WriteOnly);
	img.save( &buffer, "PNG" );
	return s << ba;
}
QDataStream& operator>>(QDataStream& s, SegmentationImage& im) {
	QByteArray ba;
	s >> ba;
	QBuffer buffer( &ba );
	buffer.open(QIODevice::ReadOnly);
	QImage img;
	img.load( &buffer, "PNG" );
	im.init( img.width(), img.height() );
	for( int j=0; j<img.height(); j++ )
		for( int i=0; i<img.width(); i++ )
			im(i,j) = img.pixel(i,j) & 0xffffff;
	return s;
}

