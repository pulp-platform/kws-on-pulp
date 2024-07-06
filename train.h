#ifndef __train_H__
#define __train_H__

#include "globals.h"

#define TRAIN_EPS 1

static float ce_loss_pre;
static float ce_loss_post;
static float ce_loss_pre_val;
static float ce_loss_post_val;
static float correct_pre_val;
static float correct_post_val;


char yes[N_MFCC_MELS * N_MFCC_WINS];
char no[N_MFCC_MELS * N_MFCC_WINS]; 
char up[N_MFCC_MELS * N_MFCC_WINS];
char down[N_MFCC_MELS * N_MFCC_WINS];
char left[N_MFCC_MELS * N_MFCC_WINS];
char right[N_MFCC_MELS * N_MFCC_WINS];
char on[N_MFCC_MELS * N_MFCC_WINS];
char off[N_MFCC_MELS * N_MFCC_WINS];
char stop[N_MFCC_MELS * N_MFCC_WINS];
char go[N_MFCC_MELS * N_MFCC_WINS];

void train();
void evaluate_largetest(int was_trained);
void evaluate_tinytest(int was_trained);
void evaluate_online(int was_trained);

#endif /* __train_H__ */
