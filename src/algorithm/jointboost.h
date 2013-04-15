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
#include "util/image.h"
#include "config.h"
#include "settings.h"
#include <QVector>
#include <cmath>
#include <QTime>

#ifdef USE_TBB
#include <tbb/task_scheduler_init.h>
#include <tbb/parallel_reduce.h>
#include <tbb/blocked_range.h>
#endif

// Optimize a weak classifier and return it's error
double optimizeWeak( const QVector< double > & wi, const QVector< double > & wizi, int NT, const QVector<double> & kc, const QVector<double> & kc_num, const QVector<double> & kc_den, unsigned long long sharing, int * thres_id = NULL, double * r_a = NULL, double * r_b = NULL );

// Training a weak learner
template<typename W>
struct BoostRound{
	double a, b;
	unsigned long long sharing_set;
	W weak;
	double error;
};

// Train a single random weak classifier
template<typename W, typename D>
BoostRound<W> trainSingle( const QVector<D> & data, const QVector< signed char > & gt, int n_classes, int n_thresholds, const QVector<double> & class_weight, const QVector<double> & kc, const QVector<double> & kc_num, const QVector<double> & kc_den ){
	BoostRound<W> r;
	r.error = 1e100;
	r.a = r.b = 0;
	// Generate a new weak classifier
	r.weak = W::random();

	// Compute all values
	QVector< double > values( data.count() );
	for( int i=0; i<data.count(); i++ )
		values[i] = r.weak.value( data[i] );

	// Compute min and max values
	double min = values.first(), max = values.first();
	for( int i=1; i<values.count(); i++ )
		if (values[i] < min)
			min = values[i];
		else if (values[i] > max)
			max = values[i];
	if (min >= max)
		return r;
	
	const double exp_growth_factor = 1.1;
	QVector< double > thresholds;
	for( int i=1; i<n_thresholds; i++ ) // Uniform Thresholds
		thresholds.append( min + (max - min)*i/n_thresholds );
	for( int i=0; i<n_thresholds; i++ ) // Sample Thresholds
		thresholds.append( values[ random()%values.count() ] );
	double tot_exp = 0;
	for( double i=0, f=1; i<n_thresholds; i++, f*=exp_growth_factor ) // Exponentially growing thresholds
		tot_exp += f;
	double step_size = (max - min) / tot_exp;
	for( double i=0, f=1, p=1; i<n_thresholds; i++, f*=exp_growth_factor, p+=f ) // Exponentially growing thresholds
		thresholds.append( min + step_size * p );
	
	qSort( thresholds );
	
	// Build a histogram where each bin is a block:  value \in [i,i+1] * (max-min) / n_thresholds + min
	QVector< double > wi( (thresholds.count()+1)*n_classes, 0.0 ), wizi( (thresholds.count()+1)*n_classes, 0.0 );
	const signed char * tgt = gt.data();
	const double *tcw = class_weight.data();
	for( int i=0; i<values.count(); i++, tgt++ ){
		// Use lower bound because we compare (f_i <= t)
// 		int t = qLowerBound( thresholds, values[i] ) - thresholds.begin();
		int t = qUpperBound( thresholds, values[i] ) - thresholds.begin();
		double * twi = wi.data()+t*n_classes, * twizi = wizi.data()+t*n_classes;
		for( int c=0; c<n_classes; c++, twi++, twizi++, tcw++ ){
			*twi += *tcw;
			*twizi += *tgt==c ? *tcw : -*tcw;
		}
	}
	// Greedily find a better sharing set
	unsigned long long sharing_set = 0;
	for( int n_bits = 0; n_bits < n_classes; n_bits++ ){
		double lbest = 1e100;
		unsigned long long lbest_sharing = sharing_set;
		
		// For each bit that's not in the sharing set see if adding it will improve our score
		for( int bit = 0; bit < n_classes; bit++ )
			if (!(sharing_set & (1ll << bit)) ){
				unsigned long long new_sharing_set = sharing_set | (1ll << bit);
				int tid;
				double a, b;
				double score = optimizeWeak( wi, wizi, thresholds.count()+1, kc, kc_num, kc_den, new_sharing_set, &tid, &a, &b );
				if (score < lbest){
					lbest = score;
					lbest_sharing = new_sharing_set;
					// If we found a new global optimum set it
					if (score < r.error){
						r.error = score;
						r.sharing_set = new_sharing_set;
						if (tid>0)
							r.weak.setThreshold( thresholds[tid-1] );
						else
							r.weak.setThreshold( thresholds[0] );
						r.a = a;
						r.b = b;
					}
				}
			}
		sharing_set = lbest_sharing;
	}
	return r;
}
#ifdef USE_TBB

template<typename W, typename D>
struct TBBTrainRound{
	BoostRound<W> best;
	const QVector<D> & data;
	const QVector< signed char > & gt;
	int n_classes;
	int n_thresholds;
	const QVector<double> & class_weight;
	const QVector<double> & kc;
	const QVector<double> & kc_num;
	const QVector<double> & kc_den;
	TBBTrainRound( const TBBTrainRound & o, tbb::split ):data(o.data),gt(o.gt),n_classes(o.n_classes),n_thresholds(o.n_thresholds),class_weight(o.class_weight),kc(o.kc),kc_num(o.kc_num),kc_den(o.kc_den){
		best.error = 1e100;
	}
	TBBTrainRound( const QVector<D> & data, const QVector< signed char > & gt, int n_classes, int n_thresholds, const QVector<double> & class_weight, const QVector<double> & kc, const QVector<double> & kc_num, const QVector<double> & kc_den ):data(data),gt(gt),n_classes(n_classes),n_thresholds(n_thresholds),class_weight(class_weight),kc(kc),kc_num(kc_num),kc_den(kc_den){
		best.error = 1e100;
	}
	void join( const TBBTrainRound & o ){
		if (o.best.error < best.error)
			best = o.best;
	}
	void operator()( tbb::blocked_range<int> rng ){
		// Text a number of weak classifiers
		best.error = 1e100;
		for( int i=rng.begin(); i<rng.end(); i++ ){
			BoostRound<W> r = trainSingle<W,D>( data, gt, n_classes, n_thresholds, class_weight, kc, kc_num, kc_den );
			if (r.error < best.error)
				best = r;
		}
	}
};

// Train a single random weak classifier using tbb
template<typename W, typename D>
BoostRound<W> trainRound( const QVector<D> & data, const QVector< signed char > & gt, int n_classes, int n_classifiers, int n_thresholds, const QVector<double> & class_weight, const QVector<double> & kc, const QVector<double> & kc_num, const QVector<double> & kc_den ){
	TBBTrainRound<W,D> rounds( data, gt, n_classes, n_thresholds, class_weight, kc, kc_num, kc_den );
	tbb::parallel_reduce( tbb::blocked_range<int>(0, n_classifiers, 4), rounds );
	return rounds.best;
}
#else
// Train a single random weak classifier
template<typename W, typename D>
BoostRound<W> trainRound( const QVector<D> & data, const QVector< signed char > & gt, int n_classes, int n_classifiers, int n_thresholds, const QVector<double> & class_weight, const QVector<double> & kc, const QVector<double> & kc_num, const QVector<double> & kc_den ){
	// Text a number of weak classifiers
	BoostRound<W> best;
	best.error = 1e100;
	for( int i=0; i<n_classifiers; i++ ){
		BoostRound<W> r = trainSingle<W,D>( data, gt, n_classes, n_thresholds, class_weight, kc, kc_num, kc_den );
		if (r.error < best.error)
			best = r;
	}
	return best;
}
#endif
template<typename W>
class JointBoost
{
protected:
	template <typename WW> friend QDataStream& operator<<( QDataStream & s, const JointBoost<WW> & b );
	template <typename WW> friend QDataStream& operator>>( QDataStream & s, JointBoost<WW> & b );
	int num_rounds_, num_classes_;
	QVector<double> a_, b_;
	QVector<unsigned long long> sharing_set_;
	QVector< QVector<double> > kc_;
	QVector< W > weak_learner_;
	
public:
template<typename D>
	void train( const QVector<D> & data, const QVector< signed char > & gt, int n_classes, int n_rounds, int n_classifiers, int n_thresholds ){
		a_.clear();
		b_.clear();
		sharing_set_.clear();
		kc_.clear();
		weak_learner_.clear();
		
		num_classes_ = n_classes;
		num_rounds_ = n_rounds;
		qDebug("Boosting %d", gt.size() );
		QVector< double > class_weight( data.size()*n_classes, 1 );
		// Do N rounds of boosting
		for( int t=0; t<n_rounds; t++ ){
			QTime timer;
			timer.start();
			
			qDebug("  Round %d", t);
			
			// Compute kc
			QVector<double> kc( n_classes, 0.0 );
			QVector<double> kc_num( n_classes, 0.0 );
			QVector<double> kc_den( n_classes, 0.0 );
			double * tcw = class_weight.data();
			for( int i=0; i<gt.count(); i++ )
				for( int c=0; c<n_classes; c++, tcw++ ){
					kc_num[c] += gt[i]==c ? *tcw : -*tcw;
					kc_den[c] += *tcw;
				}
			for( int c=0; c<n_classes; c++ )
				kc[c] = kc_num[c] / kc_den[c];
			
			double t1 = timer.elapsed() / 1000.0, t2=0;
			
			timer.restart();
			// Text a number of weak classifiers
			BoostRound<W> best = trainRound<W,D>( data, gt, n_classes, n_classifiers, n_thresholds, class_weight, kc, kc_num, kc_den );
			t2 = timer.elapsed() / 1000.0; timer.restart();
			
			// Let's recompute a and b, just to be sure
			double ab_num=0, ab_den=0, b_num=0, b_den=0;
			
			tcw = class_weight.data();
			for( int i=0; i<data.count(); i++ ){
				bool cls = best.weak.classify( data[i] );
                for (int c = 0; c < num_classes_; c++, tcw++)
					if(best.sharing_set & (1ll<<c)){
						double wi = (*tcw);
						double zi = gt[i] == c ? 1.0 : -1.0;
						if (cls){
							ab_num += wi*zi;
							ab_den += wi;
						}
						else {
							b_num += wi*zi;
							b_den += wi;
						}
					}
			}
			double b = b_num / b_den;
			double a = ab_num / ab_den - b;
			// Reweight
			double error = 0;
			tcw = class_weight.data();
			for( int i=0; i<data.count(); i++ ){
				double hm = best.weak.classify( data[i] ) ? (best.a+best.b) : best.b;
                for (int c = 0; c < num_classes_; c++, tcw++){
					double hc = (best.sharing_set & (1ll<<c)) ? hm : kc[c];
					double zi = gt[i] == c ? 1.0 : -1.0;
					error += (*tcw)*(zi - hc)*(zi - hc);
					*tcw *= exp(-zi * hc);
				}
			}
			
			// Add the result of the current round
			a_.append( best.a );
			b_.append( best.b );
			sharing_set_.append( best.sharing_set );
			kc_.append( kc );
			// Finalize the weak learner [upsample, ...]
			best.weak.finalize();
			weak_learner_.append( best.weak );
			qDebug("     err: %f (==%f) time: [%0.3f %0.3f %0.3f    %f]", best.error, error, t1, t2, timer.elapsed()/1000.0, t1+t2+timer.elapsed()/1000.0);
			qDebug("     sset: 0x%llx a: %f (==%f) b: %f (==%f) rect: [%d %d - %d %d] thres: %f", best.sharing_set, best.a, a, best.b, b, best.weak.x1_, best.weak.y1_, best.weak.x2_, best.weak.y2_, best.weak.threshold_ );
// 			if ((best.error-error) / (best.error+error) > 10e-5){
// 				qFatal( "Oops fucked up! %f", (best.error-error) / (best.error+error) );
// 			}
		}
	}
	
template<typename I>
	Image<float> classify( const I& int_im ) const{
		float t0 = 0, t1 = 0;
		Image<float> r( int_im.width(), int_im.height(), num_classes_ );
		r.fill( 0 );
		// Do the boosting
		QTime timer;
		Image<bool> cls( int_im.width(), int_im.height() );
		for( int k=0; k<num_rounds_; k++ ){
			timer.start();
			weak_learner_[k].fast_classify( int_im, cls );
			t0 += timer.elapsed();timer.restart();
			
			QVector<double> kc = kc_[k];
			unsigned long long sset = sharing_set_[k];
			double ab = a_[k] + b_[k], b = b_[k];
			
			float * rdata = r.data();
			bool * cdata = cls.data();
			for( int j=0; j<int_im.height(); j++ )
				for( int i=0; i<int_im.width(); i++, cdata++ ){
					double value = *cdata ? ab : b;
					for( int c=0; c<num_classes_; c++, rdata++)
						*rdata += ((1ll<<c)&sset) ? value : kc[c];
				}
			t1 += timer.elapsed();timer.restart();
		}
		qDebug("Classification time %f %f", t0, t1 );
		// Make the result something like a probability distribution
		// TODO: maybe logistic regression is the way to go [with learned parameters]
#ifndef RAW_BOOSTING_OUTPUT
		for( int j=0; j<int_im.height(); j++ )
			for( int i=0; i<int_im.width(); i++ ){
				double mx = r(i,j,0);
				for( int c=1; c<num_classes_; c++ )
					if (r(i,j,c) > mx)
						mx = r(i,j,c);
				for( int c=0; c<num_classes_; c++ )
					r(i,j,c) = exp( r(i,j,c)-mx );
				double tot = 0;
				for( int c=0; c<num_classes_; c++ )
					tot += r(i,j,c);
				for( int c=0; c<num_classes_; c++ )
					r(i,j,c) /= tot;
			}
#endif
		return r;
	}
	
};

template <typename W>
QDataStream& operator<<( QDataStream & s, const JointBoost<W> & b ){
	return s << b.num_rounds_ << b.num_classes_ << b.a_ << b.b_ << b.sharing_set_ << b.kc_ << b.weak_learner_;
}
template <typename W>
QDataStream& operator>>( QDataStream & s, JointBoost<W> & b ){
	return s >> b.num_rounds_ >> b.num_classes_ >> b.a_ >> b.b_ >> b.sharing_set_ >> b.kc_ >> b.weak_learner_;
}
