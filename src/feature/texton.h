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
#include "feature.h"
#include "algorithm/kmeans.h"

#include <QSharedPointer>

#include <Eigen/Core>
using namespace Eigen;

class Texton
{
	QSharedPointer<Feature> feature_;
	KMeans kmeans_;
	int N_;
	VectorXd mean_;
	MatrixXd transformation_;
public:
	Texton( QSharedPointer< Feature > feature, int n_textons );
	void train( const QVector< Image< float > >& lab_images, const QVector< QString >& names, int n_samples = 100000  );
	Image<short> textonize( const Image< float >& lab_image, const QString & name ) const;
	QVector< Image<short> > textonize( const QVector< Image<float> > & lab_images, const QVector< QString >& names  ) const;
};

void saveTextons( const QString & filename , const QVector< Image<short> > & textons, const QVector< QString > & names );
QVector< Image<short> > loadTextons( const QString & filename , const QVector< QString > & names );
