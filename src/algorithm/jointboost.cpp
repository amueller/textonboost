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

#include "jointboost.h"

double optimizeWeak( const QVector< double > & wi, const QVector< double > & wizi, int NT, const QVector<double> & kc, const QVector<double> & kc_num, const QVector<double> & kc_den, unsigned long long sharing, int * thres_id, double * r_a, double * r_b ) {
    // Precomputations
    double sum_wi = 0;
    double sum_wizi = 0;
    for ( int c=0; c<kc.count(); c++ )
        if ((1ll<<c) & sharing) {
            sum_wi += kc_den[c];
            sum_wizi += kc_num[c];
        }

    double best = 1e100;
    if (thres_id) *thres_id = 0;
    if (r_a) *r_a = sum_wizi / sum_wi;
    if (r_b) *r_b = 0;
    // Store sum wi*zi for vi > theta
    QVector< double > rev_rsum_wizi = kc_num;
    // Store sum wi for vi > theta
    QVector< double > rev_rsum_wi = kc_den;
    double b_num = 0, b_den = 0;
    const double * twizi = wizi.data();
    const double * twi = wi.data();
    for ( int t=0; t<NT-1; t++ ) {
        for ( int c=0; c<kc.count(); c++, twizi++, twi++ ) {
            // Update the wizi and wi running sums
            rev_rsum_wizi[c] -= *twizi;
            rev_rsum_wi[c] -= *twi;

            // Update the numerator and denominator of a and b
            if ((1ll<<c) & sharing) {
                b_num += *twizi;
                b_den += *twi;
            }
        }
        if (sum_wi - b_den <= 1e-10 || b_den <= 1e-10)
			continue;
        // Compute a and b
        double b = b_num / b_den;
        double a = (sum_wizi - b_num) / (sum_wi - b_den) - b;

        double error = 0;
        for ( int c=0; c<kc.count(); c++ ) {
            if ((1ll<<c) & sharing)
                // z_i*z_i = 1
                //       sum_i w_i + sum_i w_i z_i h_i(c)                   + sum_i w_i h_i(c)*h_i(c)
                error += kc_den[c] - 2.*a*rev_rsum_wizi[c] - 2.*b*kc_num[c] + (a*(a+2*b))*rev_rsum_wi[c] + b*b*kc_den[c];
            else
                error += kc_den[c] - 2.*kc[c]*kc_num[c] + kc[c]*kc[c]*kc_den[c];
        }
        if (error < best) {
            best = error;
            if (thres_id) *thres_id = t+1;
            if (r_a) *r_a = a;
            if (r_b) *r_b = b;
        }
    }
    return best;
}
