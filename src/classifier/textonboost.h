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

#pragma once

#include "algorithm/jointboost.h"

class TextonData{
protected:
	friend class TextonLearner;
	const Image< float > * int_image_;
	int x_, y_;
public:
	TextonData( const Image<float> * int_image = NULL, int x=0, int y=0 );
	double value( int x1, int y1, int x2, int y2, int t ) const;
};

class TextonClassifier{
public:
	friend QDataStream& operator<<( QDataStream & s, const TextonClassifier & c );
	friend QDataStream& operator>>( QDataStream & s, TextonClassifier & c );
	int x1_, y1_, x2_, y2_;
	int t_;
	float threshold_;
protected: // Static settings
	friend class TextonBoost;
	static QVector< int > texton_offset_;
	static int sub_sample_factor_;
	static int min_rect_size_;
	static int max_rect_size_;
public:
	static TextonClassifier random();
	double value( const TextonData & data ) const;
	Image<float> value(const Image<float>& im) const;
	bool classify( const TextonData & data ) const;
	Image<bool> classify( const Image<float> & im ) const;
	void fast_classify( const Image<float> & im, Image<bool> & res ) const;
	void setThreshold( float t );
	void finalize();
};
QDataStream& operator<<( QDataStream & s, const TextonClassifier & c );
QDataStream& operator>>( QDataStream & s, TextonClassifier & c );


class LabelImage;
class TextonBoost: protected JointBoost<TextonClassifier>
{
protected:
	friend QDataStream& operator<<( QDataStream & s, const TextonBoost & b );
	friend QDataStream& operator>>( QDataStream & s, TextonBoost & b );
	QVector< int > texton_offset_;
	Image<float> integrate( const Image< short int >& texton, const QVector< int >& n_textons, int subsample ) const;
public:
	// train will clear all textons (so save memory)
	void train( QVector< Image< short > >& textons, const QVector< LabelImage >& gt, int n_rounds, int n_classifiers, int n_thresholds, int subsample, int min_rect_size, int max_rect_size );
	Image<float> evaluate( const Image< short >& textons ) const;
	void save( const QString & s );
	void load( const QString& name );
};

QDataStream& operator<<( QDataStream & s, const TextonBoost & b );
QDataStream& operator>>( QDataStream & s, TextonBoost & b );
	