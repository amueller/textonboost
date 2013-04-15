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

#include "texton.h"
#include "config.h"
#ifdef USE_TBB
#include <tbb/task_scheduler_init.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_reduce.h>
#include <tbb/blocked_range.h>
#endif
#include <QMap>
#include <Eigen/SVD>
#include <QSet>

Texton::Texton( QSharedPointer<Feature> feature, int n_textons ) :feature_(feature), N_(n_textons) {
}

#ifdef USE_TBB
class TBBComputeFeatures{
	const QSharedPointer<Feature> & feature;
	const QVector< Image< float > >& lab_images;
	const QVector< QString >& names;
	int n_samples;
	const int feature_size;
public:
	QVector<float> features;
	VectorXd mean;
	MatrixXd covariance;
	double count;
	
	TBBComputeFeatures( const QSharedPointer<Feature> & feature, const QVector< Image< float > >& lab_images, const QVector< QString >& names, int n_samples ): feature(feature),lab_images(lab_images),names(names),n_samples(n_samples),feature_size(feature->size()){
		mean = VectorXd::Zero( feature_size );
		covariance = MatrixXd::Zero( feature_size, feature_size );
		count = 0;
	}
	TBBComputeFeatures( const TBBComputeFeatures & o, tbb::split ):feature(o.feature),lab_images(o.lab_images),names(o.names),n_samples(o.n_samples),feature_size(o.feature_size){
		mean = VectorXd::Zero( feature_size );
		covariance = MatrixXd::Zero( feature_size, feature_size );
		count = 0;
	}
	void join( const TBBComputeFeatures & o ){
		// Merge the sample
		covariance = covariance + o.covariance + (o.mean - mean) * (o.mean - mean).transpose()*(1.0*count*o.count/(count+o.count));
		mean = (count*mean + o.count*o.mean) / (count + o.count);
		count = o.count + count;
		features += o.features;
	}
	void operator()( tbb::blocked_range<int> rng ){
		double samples_per_image = 1.0*n_samples / lab_images.count();
	
		// Collect some statistics on mean and variance of each feature
		for( int i=rng.begin(), n_features=rng.begin()*samples_per_image; i<rng.end(); i++ ){
			Image< float > feature_response = feature->evaluate( lab_images[i], names[i] );
			for( int j=0; j<feature_response.height(); j++ )
				for( int i=0; i<feature_response.width(); i++ ){
					VectorXd x( feature_size );
					for( int k=0; k<feature_size; k++ )
						x[k] = feature_response(i,j,k);
					
					count += 1;
					VectorXd delta = x - mean;
					mean += delta / count;
					covariance += delta * (x-mean).transpose();
				}
			
			for(;n_features < (i+1)*samples_per_image; n_features++){
				int x = random() % feature_response.width();
				int y = random() % feature_response.height();
				for( int i=0; i<feature_size; i++ )
					features.append( feature_response( x, y, i ) );
			}
		}
	}
};
void computeFeatures( QVector<float> & features, VectorXd & mean, MatrixXd & covariance, const QSharedPointer<Feature> & feature, const QVector< Image< float > >& lab_images, const QVector< QString >& names, int n_samples ){
	TBBComputeFeatures tbbf( feature, lab_images, names, n_samples );
	tbb::parallel_reduce( tbb::blocked_range<int>(0, lab_images.count(), 4), tbbf );
	
	// Compute the final covariance
	mean = tbbf.mean;
	covariance = tbbf.covariance / tbbf.count;
	features = tbbf.features;
}
#else
void computeFeatures( QVector<float> & features, VectorXd & mean, MatrixXd & covariance, const QSharedPointer<Feature> & feature, const QVector< Image< float > >& lab_images, const QVector< QString >& names, int n_samples ){
	int feature_size = feature->size();
	int n_features = 0;
	double samples_per_image = 1.0*n_samples / lab_images.count();
	
	// Do propper whitening: http://en.wikipedia.org/wiki/White_noise#Whitening_a_random_vector
	
	// Collect some statistics on mean and variance of each feature
	mean = VectorXd::Zero( feature_size );
	covariance = MatrixXd::Zero( feature_size, feature_size );
	double count = 0;
	for( int i=0; i<lab_images.count(); i++ ){
		Image< float > feature_response = feature->evaluate( lab_images[i], names[i] );
		for( int j=0; j<feature_response.height(); j++ )
			for( int i=0; i<feature_response.width(); i++ ){
				VectorXd x( feature_size );
				for( int k=0; k<feature_size; k++ )
					x[k] = feature_response(i,j,k);
				
				count += 1;
				VectorXd delta = x - mean;
				mean += delta / count;
				covariance += delta * (x-mean).transpose();
			}
		
		for(;n_features < (i+1)*samples_per_image; n_features++){
			int x = random() % feature_response.width();
			int y = random() % feature_response.height();
			for( int i=0; i<feature_size; i++ )
				features.append( feature_response( x, y, i ) );
		}
	}
	// Compute the final covariance
	covariance = covariance / count;
}
#endif
void Texton::train(const QVector< Image< float > >& lab_images, const QVector<QString> & names, int n_samples ) {
	QVector< float > features;
	int feature_size = feature_->size();
	MatrixXd covariance;
	computeFeatures( features, mean_, covariance, feature_, lab_images, names, n_samples );
	
	int n_features = features.count() / feature_size;
	
	// Do propper whitening: http://en.wikipedia.org/wiki/White_noise#Whitening_a_random_vector
// 	// No Whitening
// 	transformation_ = MatrixXd::Identity( feature_size, feature_size );
// 	// Simple Whitening
// 	transformation_ = covariance.diagonal().array().inverse().sqrt().matrix().asDiagonal();
	// True Whitening
	JacobiSVD< MatrixXd > svd( covariance, ComputeThinU | ComputeThinV );
	transformation_ = svd.singularValues().array().inverse().sqrt().matrix().asDiagonal() * svd.matrixV().transpose();
	
	// Whitening (zero mean, 1 stddev)
	for( int i=0, k=0; i<n_features; i++ ){
		VectorXd x( feature_size );
		for( int j=0,kk=k; j<feature_size; j++, kk++ )
			x[j] = features[kk];
		x = transformation_*(x-mean_);
		for( int j=0; j<feature_size; j++, k++ )
			features[k] = x[j];
	}
	
	// Train the texton directory
	kmeans_.train( features.data(), feature_size, n_features, N_ );
}
Image< short > Texton::textonize(const Image< float >& lab_image, const QString & name ) const{
	Image< float > feature_response = feature_->evaluate( lab_image, name );
	// Whitening (zero mean, 1 stddev)
	for( int j=0; j<feature_response.height(); j++ )
		for( int i=0; i<feature_response.width(); i++ ){
			VectorXd x( feature_response.depth() );
			for( int k=0; k<feature_response.depth(); k++ )
				x[k] = feature_response(i,j,k);
			x = transformation_*(x - mean_);
			for( int k=0; k<feature_response.depth(); k++ )
				feature_response(i,j,k) = x[k];
		}
	return kmeans_.evaluate( feature_response );
}
#ifdef USE_TBB
class TBBTextonize{
	QVector< Image< short > > &r;
	const QVector< Image< float > >& lab_images;
	const QVector< QString >& names;
	const Texton & texton;
public:
	TBBTextonize(QVector< Image< short > > &r, const QVector< Image< float > >& lab_images, const QVector< QString >& names, const Texton & texton):r(r),lab_images(lab_images),names(names),texton(texton){
	}
	void operator()( tbb::blocked_range<int> rng ) const{
		for( int i=rng.begin(); i<rng.end(); i++ )
			r[i] = texton.textonize( lab_images[i], names[i] );
	}
};
QVector< Image< short > > Texton::textonize(const QVector< Image< float > >& lab_images, const QVector< QString >& names) const {
	QVector< Image< short > > r( lab_images.count() );
	tbb::parallel_for(tbb::blocked_range<int>(0, lab_images.count(), 1), TBBTextonize(r, lab_images, names, *this));
	return r;
}
#else
QVector< Image< short > > Texton::textonize(const QVector< Image< float > >& lab_images, const QVector< QString >& names) const {
	QVector< Image< short > > r;
	for( int i=0; i<lab_images.count(); i++ )
		r.append( textonize( lab_images[i], names[i] ) );
	return r;
}
#endif
void saveTextons(const QString& filename, const QVector< Image< short > >& textons, const QVector< QString >& names) {
	QFile file( filename );
	if (!file.open(QFile::WriteOnly))
		qFatal( "Failed to save textons to '%s'", qPrintable( filename ) );
	QDataStream s( &file );
	for( int i=0; i<textons.count(); i++ )
		s << names[i] << textons[i];
	file.close();
}
QVector< Image< short > > loadTextons(const QString& filename, const QVector< QString >& names) {
	QSet< QString > snames = QSet< QString >::fromList( names.toList() );
	QFile file( filename );
	if (!file.open(QFile::ReadOnly))
		qFatal( "Failed to load textons to '%s'", qPrintable( filename ) );
	QDataStream s( &file );
	QMap< QString, Image< short > > texton_map;
	while(!s.atEnd()){
		QString name;
		Image< short > textons;
		s >> name >> textons;
		if (snames.contains( name ))
			texton_map[ name ] = textons;
	}
	file.close();
	
	QVector< Image< short > > r;
	foreach( QString name, names )
		r.append( texton_map[name] );
	return r;
}
