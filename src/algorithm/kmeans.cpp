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

#include "kmeans.h"
#include "config.h"
#ifdef USE_TBB
#include <tbb/task_scheduler_init.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#endif
#include <util/image.h>

#define MAX_IT 1000

void KMeans::train(const float * features, int feature_size, int M, int N) {
	n_center_ = N;
	feature_size_ = feature_size;
	center_.fill( 0.0f, N*feature_size );
	
	// Use K-Means to find the centers [initial seeds]
	for( int i=0; i<N; i++ )
		memcpy( center_.data() + feature_size*i, features + (random() % M)*feature_size, feature_size*sizeof(float) );
	
	// Happily iterate
	QVector<short> belongs_to( M, 0 );
	QVector<int> count( N, 0 );
	for( int it=0; it<MAX_IT; it++ ){
		int change = 0;
		// Assignment step
		initAnn();
		
		QVector<short> old_belongs_to = belongs_to;
		evaluateMany( features, feature_size, M, belongs_to.data() );
		for( int i=0; i<M; i++ ){
			if (belongs_to[i]!=old_belongs_to[i])
				change++;
		}
		if (!change)
			break;
		
		// Update step
		count.fill( 0 );
		center_.fill( 0 );
		for( int i=0; i<M; i++ ){
			for( int j=0; j<feature_size; j++ )
				center_[ belongs_to[i]*feature_size+j ] += features[i*feature_size+j];
			count[ belongs_to[i] ]++;
		}
		
		for( int i=0; i<N; i++ )
			if (count[i] == 0)
				// If the current cluster dies, choose a random feature
				memcpy( center_.data() + feature_size*i, features + (random() % M)*feature_size, feature_size*sizeof(float) );
			else
				// otherwise normalize
				for( int j=0; j<feature_size; j++ )
					center_[ i*feature_size+j ] *= 1.0 / count[i];
	}
}
KMeans::KMeans(const KMeans& o) :center_(o.center_), feature_size_(o.feature_size_), n_center_(o.n_center_), kd_tree_(NULL), ann_points_(NULL) {
}
KMeans::KMeans() :feature_size_(0), n_center_(0), kd_tree_(NULL), ann_points_(NULL) {
}
KMeans::~KMeans() {
    if (kd_tree_)
        delete kd_tree_;
	if (ann_points_)
		annDeallocPts( ann_points_ );
}
void KMeans::initAnn() const {
	// This is threadsafe now
	mutex_.lock();
	{
		if (kd_tree_)
			delete kd_tree_;
		if (ann_points_)
			annDeallocPts( ann_points_ );
		
		ann_points_ = annAllocPts( n_center_, feature_size_ );
		
		for( int i=0, k=0; i<n_center_; i++ )
			for( int j=0; j<feature_size_; j++, k++ )
				ann_points_[i][j] = center_[k];
		kd_tree_ = new ANNkd_tree( ann_points_, n_center_, feature_size_ );
	}
	mutex_.unlock();
}
#ifdef USE_TBB
class TBBEvaluateMany{
	const float * features;
	int feature_size, N;
	short * res;
	ANNkd_tree * kd_tree;
public:
	TBBEvaluateMany( const float * features, int feature_size, int N, short * res, ANNkd_tree * kd_tree ):features(features),feature_size(feature_size),N(N),res(res),kd_tree(kd_tree){
	}
	void operator()( tbb::blocked_range<int> rng ) const{
		ANNidx id;
		ANNdist d;
		ANNpoint pt = annAllocPt( feature_size );
		for( int i=rng.begin(); i<rng.end(); i++ ){
			for( int j=0; j<feature_size; j++ )
				pt[j] = features[i*feature_size+j];
			kd_tree->annkSearch(pt, 1, &id, &d );
			res[i] = id;
		}
		annDeallocPt( pt );
	}
};
void KMeans::evaluateMany( const float * features, int feature_size, int N, short * res ) const{
	if (!kd_tree_)
		initAnn();
	// Our patched version of ANN works with TBB
	tbb::parallel_for(tbb::blocked_range<int>(0, N, 100), TBBEvaluateMany(features, feature_size, N, res, kd_tree_));
}
#else
void KMeans::evaluateMany( const float * features, int feature_size, int N, short * res ) const{
	if (!kd_tree_)
		initAnn();
	// Our patched version of ANN works with TBB
	ANNidx id;
	ANNdist d;
	ANNpoint pt = annAllocPt( feature_size );
	for( int i=0; i<N; i++ ){
		for( int j=0; j<feature_size; j++ )
			pt[j] = features[i*feature_size+j];
		kd_tree_->annkSearch(pt, 1, &id, &d );
		res[i] = id;
	}
	annDeallocPt( pt );
}
#endif
short KMeans::evaluate(const float * feature, int feature_size) const {
	if (!kd_tree_)
		initAnn();
	ANNidx id;
	ANNdist d;
	ANNpoint pt = annAllocPt( feature_size );
	for( int i=0; i<feature_size; i++ )
		pt[i] = feature[i];
	
	kd_tree_->annkSearch( pt, 1, &id, &d );
	
	annDeallocPt( pt );
	return id;
}
Image<short> KMeans::evaluate(const Image<float>& fim ) const{
	Image<short> r( fim.width(), fim.height() );
	evaluateMany( fim.data(), fim.depth(), fim.width()*fim.height(), r.data() );
	return r;
}
void KMeans::save(const QString& s) const {
	QFile file( s );
	if (!file.open(QFile::WriteOnly))
		qWarning( "Failed to write file '%s'", qPrintable( s ) );
	else {
		QDataStream s(&file);
		s << center_ << feature_size_ << n_center_;
		file.close();
	}
}
void KMeans::load(const QString& s) {
	QFile file( s );
	if (!file.open(QFile::ReadOnly))
		qWarning( "Failed to load file '%s'", qPrintable( s ) );
	else {
		QDataStream s(&file);
		s >> center_ >> feature_size_ >> n_center_;
		file.close();
		initAnn();
	}
}
