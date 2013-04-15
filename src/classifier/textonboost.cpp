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

#include "textonboost.h"
#include "settings.h"
#include <util/labelimage.h>

/**** Data ****/
TextonData::TextonData(const Image< float >* int_image, int x, int y) :int_image_(int_image), x_(x), y_(y) {
}
double TextonData::value(int x1, int y1, int x2, int y2, int t) const {
    x1 += x_;
    x2 += x_;
    y1 += y_;
    y2 += y_;
	if (x1 >= int_image_->width() || x2 <= 0 || y1 >= int_image_->height() || y2 <= 0)
		return 0;
    
	// Make the coords fit
	if (x1 < 0) x1 = 0;
	if (y1 < 0) y1 = 0;
	if (x2 > int_image_->width() ) x2 = int_image_->width();
	if (y2 > int_image_->height()) y2 = int_image_->height();
	
	// Sum up the rect
	double r = (*int_image_)(x2-1,y2-1,t);
	if (x1>0)         r -= (*int_image_)(x1-1,y2-1,t);
	if (y1>0)         r -= (*int_image_)(x2-1,y1-1,t);
	if (x1>0 && y1>0) r += (*int_image_)(x1-1,y1-1,t);
	return r / ((x2-x1)*(y2-y1));
}


static double gauss( double stddev ){
	// Marsaglia polar method
	double u, v, w;
	while( 1 ){
		u = 2.0 * random() / RAND_MAX - 1.0;
		v = 2.0 * random() / RAND_MAX - 1.0;
		w = u*u + v*v;
		if (w < 1) break;
	}
	w = sqrt( -2.0 * log( w ) / w );
	double x = u*w;
	return x * stddev;
}
static double gaussRange( double stddev2 ){
	// Marsaglia polar method
	double stddev = stddev2 / 2.0;
	return stddev + gauss(stddev);
}

/**** Weak Classifier ****/
int TextonClassifier::sub_sample_factor_ = 1;
QVector< int > TextonClassifier::texton_offset_ = QVector< int >()<<400;
int TextonClassifier::min_rect_size_ = 5;
int TextonClassifier::max_rect_size_ = 100;
TextonClassifier TextonClassifier::random() {
    TextonClassifier r;
	// Randomly pick the rectangle
#ifdef AREA_SAMPLING
	// Rect size sampling proportional to the area of the final rectangle
	double area = min_rect_size_*min_rect_size_ + (max_rect_size_*max_rect_size_ - min_rect_size_*min_rect_size_) * 1.0 * ::random() / RAND_MAX;
	int mnw = ceil( qMax( (double)min_rect_size_, area / max_rect_size_ ) );
	int mxw = floor( qMin( (double)max_rect_size_, area / min_rect_size_ ) );
	int w = mnw + ::random()%(mxw-mnw+1);
    int h = round( area / w );
	if (::random()&1)
		qSwap( w, h );
#else
	// Uniform sampling for rect size
    int w = min_rect_size_ + (::random() % (max_rect_size_-min_rect_size_+1));
    int h = min_rect_size_ + (::random() % (max_rect_size_-min_rect_size_+1));
#endif
#ifdef GAUSSIAN_OFFSET
	// Gaussian position sampling for rect
    int x = gaussRange(max_rect_size_-w);
    int y = gaussRange(max_rect_size_-h);
#else
	// Unary position sampling for rect
    int x = ::random() % (max_rect_size_-w+1);
    int y = ::random() % (max_rect_size_-h+1);
#endif
    r.x1_ = x - max_rect_size_/2;
    r.y1_ = y - max_rect_size_/2;
    r.x2_ = r.x1_+w;
    r.y2_ = r.y1_+h;
	
	// Pick a random channel
	int c = ::random() % (texton_offset_.count()-1);
	
	int mn = texton_offset_[c];
	int mx = texton_offset_[c+1];
	// Randomly pick the texton
	r.t_ = mn + (::random()%(mx-mn));
	
	return r;
}
double TextonClassifier::value(const TextonData& data) const {
	// Dont forget to correct for the smaller area in the normalization
    return data.value( x1_, y1_, x2_, y2_, t_ ) / (sub_sample_factor_*sub_sample_factor_);
}
Image<float> TextonClassifier::value(const Image<float>& im) const {
	// Dont forget to correct for the smaller area in the normalization
	Image<float> r( im.width(), im.height() );
	for( int j=0; j<im.height(); j++ )
		for( int i=0; i<im.width(); i++ )
			r(i,j) = TextonData( &im, i, j ).value( x1_, y1_, x2_, y2_, t_ ) / (sub_sample_factor_*sub_sample_factor_);
	return r;
}
bool TextonClassifier::classify(const TextonData& data) const {
    return value(data) > threshold_;
}
Image<bool> TextonClassifier::classify(const Image<float>& im) const {
	Image<bool> r( im.width(), im.height() );
	bool * rdata = r.data();
	for( int j=0; j<im.height(); j++ )
		for( int i=0; i<im.width(); i++, rdata++ )
			*rdata = TextonData( &im, i, j ).value( x1_, y1_, x2_, y2_, t_ ) > threshold_*(sub_sample_factor_*sub_sample_factor_);
	return r;
}
void TextonClassifier::fast_classify(const Image<float>& im, Image<bool> & r) const {
	bool * rdata = r.data();
	for( int j=0; j<im.height(); j++ )
		for( int i=0; i<im.width(); i++, rdata++ )
			*rdata = TextonData( &im, i, j ).value( x1_, y1_, x2_, y2_, t_ ) > threshold_*(sub_sample_factor_*sub_sample_factor_);
}
void TextonClassifier::setThreshold(float t) {
    threshold_ = t;
}
void TextonClassifier::finalize() {
    x1_ *= sub_sample_factor_;
    x2_ *= sub_sample_factor_;
    y1_ *= sub_sample_factor_;
    y2_ *= sub_sample_factor_;
}
QDataStream& operator<<(QDataStream& s, const TextonClassifier& c) {
    return s << c.x1_ << c.y1_ << c.x2_ << c.y2_ << c.t_ << c.threshold_;
}
QDataStream& operator>>(QDataStream& s, TextonClassifier& c) {
    return s >> c.x1_ >> c.y1_ >> c.x2_ >> c.y2_ >> c.t_ >> c.threshold_;
}


/**** TextonBoost ****/
Image< float > TextonBoost::integrate(const Image< short int >& texton, const QVector< int >& n_textons, int subsample) const {
	int nw = (texton.width()-1)/subsample + 1;
	int nh = (texton.height()-1)/subsample + 1;
	Image< float > r( nw, nh, texton_offset_.last() );
	r.fill(0);
	// Count
	for( int j=0; j<texton.height(); j++ )
		for( int i=0; i<texton.width(); i++ )
			for( int k=0; k<texton.depth(); k++ )
				r( i/subsample, j/subsample, texton_offset_[k] + texton(i,j,k) )+=1;
	// and Integrate
	for( int j=0; j<nh; j++ )
		for( int i=0; i<nw; i++ )
			for( int k=0; k<texton_offset_.last(); k++ ){
				if ( i      ) r(i,j,k) += r(i-1,j,k);
				if ( j      ) r(i,j,k) += r(i,j-1,k);
				if ( i && j ) r(i,j,k) -= r(i-1,j-1,k);
			}
	return r;
}
// NOTE: train will clear all textons (so save memory)
void TextonBoost::train( QVector< Image< short > >& textons, const QVector< LabelImage >& gt, int n_rounds, int n_classifiers, int n_thresholds, int subsample, int min_rect_size, int max_rect_size ) {
	texton_offset_.fill( 0, textons.first().depth()+1 );
	for( int k=0; k<textons.count(); k++ )
		for( int i=0; i<textons[k].width()*textons[k].height(); i++ )
			for( int j=0; j<textons[k].depth(); j++ )
				if ( texton_offset_[j+1] <= textons[k][i*textons[k].depth()+j] )
					texton_offset_[j+1] = textons[k][i*textons[k].depth()+j]+1;
	for( int i=1; i<texton_offset_.size(); i++ )
		texton_offset_[i] += texton_offset_[i-1];

	// Setup the weak classifier
	TextonClassifier::sub_sample_factor_ = subsample;
	TextonClassifier::texton_offset_ = texton_offset_;
	TextonClassifier::min_rect_size_ = min_rect_size / subsample;
	TextonClassifier::max_rect_size_ = max_rect_size / subsample;
	
	// Compute the subsampled integral images
	QVector< Image<float> > int_images;
	for( int i=0; i<textons.count(); i++ ){
		int_images.append( integrate( textons[i], texton_offset_, subsample ) );
		textons[i] = Image<short>();
	}
	// Create the data and groundtruth
	int n_classes = 0;
	QVector< TextonData > data;
	QVector< signed char > groundtruth;
	for( int k=0; k<int_images.count(); k++ ){
		for( int j=0; j<int_images[k].height(); j++ )
			for( int i=0; i<int_images[k].width(); i++ ){
				signed char g = gt[k](i*subsample, j*subsample);
				for( int jj=j*subsample; g>=0 && jj<(j+1)*subsample && jj<gt[k].height(); jj++ )
					for( int ii=i*subsample; g>=0 && ii<(i+1)*subsample && ii<gt[k].width(); ii++ )
						if (gt[k](ii,jj) != g)
							g = -1;
				// Only count the sample if we are absolutely sure
				if (g>=0){
					data.append( TextonData( &int_images[k], i, j ) );
					groundtruth.append( g );
					if (g >= n_classes)
						n_classes = g+1;
				}
			}
	}
	
	JointBoost<TextonClassifier>::train( data, groundtruth, n_classes, n_rounds, n_classifiers, n_thresholds );
}
Image< float > TextonBoost::evaluate(const Image< short >& textons) const {
	TextonClassifier::sub_sample_factor_ = 1;
	
	// Integrate
	Image<float> integral = integrate( textons, texton_offset_, TextonClassifier::sub_sample_factor_ );
	
	// Classify the whole image
	return classify( integral );
}
QDataStream& operator<<(QDataStream& s, const TextonBoost& b) {
    s << b.texton_offset_;
    return operator<<( s, (const JointBoost<TextonClassifier>&) b );
}
QDataStream& operator>>(QDataStream& s, TextonBoost& b) {
	s >> b.texton_offset_;
	return operator>>( s, (JointBoost<TextonClassifier>&) b );
}
void TextonBoost::save(const QString& name) {
	QFile file(name);
	if (!file.open( QFile::WriteOnly ))
		qWarning("Failed to save TextonBoost to '%s'", qPrintable( name ) );
	QDataStream s( &file );
	s << *this;
	file.close();
}
void TextonBoost::load(const QString& name) {
	QFile file(name);
	if (!file.open( QFile::ReadOnly ))
		qWarning("Failed to load TextonBoost from '%s'", qPrintable( name ) );
	QDataStream s( &file );
	s >> *this;
	file.close();
}

