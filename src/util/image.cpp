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

#include "image.h"
template<>
QDataStream& operator<< ( QDataStream& s, const Image< short int >& im )
{
    s << im.width_ << im.height_ << im.depth_;
    QImage sim ( im.width_*im.depth_, im.height_, QImage::Format_ARGB32 );
    for ( int j=0,k=0; j<im.height_; j++ )
        for ( int i=0; i<im.width_*im.depth_; i++, k++ )
            sim.setPixel ( i, j, im.data_[k] );
    sim.save ( s.device(), "PNG" );
    return s;
}
template<>
QDataStream& operator>>(QDataStream& s, Image< short int >& im) {
    int W, H, D;
    s >> W >> H >> D;
    im.init( W, H, D );

    QImage sim;
    sim.load( s.device(), "PNG" );

    for ( int j=0,k=0; j<im.height_; j++ )
        for ( int i=0; i<im.width_*im.depth_; i++, k++ )
            im.data_[k] = sim.pixel( i, j );
    return s;
}
template<>
QDataStream& operator<<(QDataStream& s, const Image< char >& im) {
    s << im.width_ << im.height_ << im.depth_;
    QImage sim( im.width_*im.depth_, im.height_, QImage::Format_ARGB32 );
    for ( int j=0,k=0; j<im.height_; j++ )
        for ( int i=0; i<im.width_*im.depth_; i++, k++ )
            sim.setPixel( i, j, im.data_[k] );
    sim.save( s.device(), "PNG" );
    return s;
}
template<>
QDataStream& operator>>(QDataStream& s, Image< char >& im) {
    int W, H, D;
    s >> W >> H >> D;
    im.init( W, H, D );

    QImage sim;
    sim.load( s.device(), "PNG" );

    for ( int j=0,k=0; j<im.height_; j++ )
        for ( int i=0; i<im.width_*im.depth_; i++, k++ )
            im.data_[k] = sim.pixel( i, j );
    return s;
}
