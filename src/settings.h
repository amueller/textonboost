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
// TODO: Make sure to adjust the parameters in this file
#define USE_MSRC

// Dataset parameters
static const char MSRC_DIRECTORY  [] = "/home/local/datasets/MSRC_ObjCategImageDatabase_v2/Images";
static const char MSRC_TRAIN_FILE [] = "/home/local/datasets/MSRC_ObjCategImageDatabase_v2/Train.txt";
static const char MSRC_VALID_FILE [] = "/home/local/datasets/MSRC_ObjCategImageDatabase_v2/Validation.txt";
static const char MSRC_TEST_FILE  [] = "/home/local/datasets/MSRC_ObjCategImageDatabase_v2/Test.txt";

// // Exact annotations
// static const char MSRC_DIRECTORY  [] = "../data/msrc/";
// static const char MSRC_TRAIN_FILE [] = "../data/msrc/newgt/Train.txt";
// static const char MSRC_VALID_FILE [] = "../data/msrc/newgt/Validation.txt";
// static const char MSRC_TEST_FILE  [] = "../data/msrc/newgt/Test.txt";

// Dataset parameters
static const char VOC2010_DIRECTORY  [] = "../data/voc/VOC2010/";
static const char VOC2010_TRAIN_FILE [] = "../data/voc/VOC2010/Train.txt";
static const char VOC2010_VALID_FILE [] = "../data/voc/VOC2010/Validation.txt";
static const char VOC2010_TEST_FILE  [] = "../data/voc/VOC2010/Test.txt";

// VOC Cache
static const char VOC2010_BBOX_DIRECTORY [] = "data/VOC2010_BBox/";

// Texton parameters
static const int N_TEXTONS = 400;
static const float FILTER_BANK_SIZE = 1.0;

// Boosting parameters
static const int N_BOOSTING_ROUNDS  = 10000; // Number of boosting rounds
static const int N_CLASSIFIERS      = 200; // Number of random classifiers to test [per round]
static const int N_THRESHOLDS       = 100; // Number of thresholds to test [per round]
// static const int N_BOOSTING_ROUNDS  = 10000; // Number of boosting rounds
// static const int N_CLASSIFIERS      = 750; // Number of random classifiers to test [per round]
// static const int N_THRESHOLDS       = 150; // Number of thresholds to test [per round]
#ifdef USE_MSRC
static const int BOOSTING_SUBSAMPLE = 5  ; // Subsampling factor (tradeoff between memory/computation and accuracy)
#else
static const int BOOSTING_SUBSAMPLE = 7  ; // Subsampling factor (tradeoff between memory/computation and accuracy)
#endif
static const int MIN_RECT_SIZE      = BOOSTING_SUBSAMPLE; // Minimum size of texton rectangle
static const int MAX_RECT_SIZE      = 200; // Maximum size of texton rectangle

// Other parameters
// #define AREA_SAMPLING   // Sample the rect size proportional to the area of the rectangle (uniform in w*h instead of uniform in w and h)
#define GAUSSIAN_OFFSET // Use Sample the offset from a gaussian centered at 0

// Shall we return the raw boosting results H or P = 1/Z * exp(-H)
#define RAW_BOOSTING_OUTPUT
