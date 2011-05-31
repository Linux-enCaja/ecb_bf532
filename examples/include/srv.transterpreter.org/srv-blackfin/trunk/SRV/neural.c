/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  neural.c - simple integer backprop neural network library based on 
 *  floating point code originally written by Baegsch of www.e-m-c.org
 *
 *       Copyright (C) 2007-2008  Surveyor Corporation
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details (www.gnu.org/licenses)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define NUM_INPUT    64
#define NUM_OUTPUT   16
#define NUM_HIDDEN   16    
#define NUM_PATTERNS 16

int learn = 300;

typedef struct pattern_st {
    int p_in[NUM_INPUT];
    int p_out[NUM_OUTPUT];
} pattern_t;

pattern_t pattern[] = {
    {  // solid ball
    {    0,    0,    0, 1024, 1024,    0,    0,    0,
         0, 1024, 1024, 1024, 1024, 1024, 1024,    0,
         0, 1024, 1024, 1024, 1024, 1024, 1024,    0,
      1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024,
      1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024,
         0, 1024, 1024, 1024, 1024, 1024, 1024,    0,
         0, 1024, 1024, 1024, 1024, 1024, 1024,    0,
         0,    0,    0, 1024, 1024,    0,    0,    0 },
    { 1024, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    }, 
    {  // solid square
    { 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024,
      1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024,
      1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024,
      1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024,
      1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024,
      1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024,
      1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024,
      1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024 },
    { 0, 1024, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    }, 
    {  // cross
    {    0,    0,    0, 1024, 1024,    0,    0,    0,
         0,    0,    0, 1024, 1024,    0,    0,    0,
         0,    0,    0, 1024, 1024,    0,    0,    0,
      1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024,
      1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024,
         0,    0,    0, 1024, 1024,    0,    0,    0,
         0,    0,    0, 1024, 1024,    0,    0,    0,
         0,    0,    0, 1024, 1024,    0,    0,    0 },
    { 0, 0, 1024, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    }, 
    {  // box
    { 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024,
      1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024,
      1024, 1024,    0,    0,    0,    0, 1024, 1024,
      1024, 1024,    0,    0,    0,    0, 1024, 1024,
      1024, 1024,    0,    0,    0,    0, 1024, 1024,
      1024, 1024,    0,    0,    0,    0, 1024, 1024,
      1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024,
      1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024 },
    { 0, 0, 0, 1024, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    }, 
    {  // circle
    {    0,    0,    0, 1024, 1024,    0,    0,    0,
         0, 1024, 1024, 1024, 1024, 1024, 1024,    0,
         0, 1024, 1024,    0,    0, 1024, 1024,    0,
      1024, 1024,    0,    0,    0,    0, 1024, 1024,
      1024, 1024,    0,    0,    0,    0, 1024, 1024,
         0, 1024, 1024,    0,    0, 1024, 1024,    0,
         0, 1024, 1024, 1024, 1024, 1024, 1024,    0,
         0,    0,    0, 1024, 1024,    0,    0,    0 },
    { 0, 0, 0, 0, 1024, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    }, 
    {  // xing
    { 1024, 1024,    0,    0,    0,    0, 1024, 1024,
      1024, 1024,    0,    0,    0,    0, 1024, 1024,
         0,    0, 1024,    0,    0, 1024,    0,    0,
         0,    0,    0, 1024, 1024,    0,    0,    0,
         0,    0,    0, 1024, 1024,    0,    0,    0,
         0,    0, 1024,    0,    0, 1024,    0,    0,
      1024, 1024,    0,    0,    0,    0, 1024, 1024,
      1024, 1024,    0,    0,    0,    0, 1024, 1024 },
    { 0, 0, 0, 0, 0, 1024, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    }, 
    {  // diamond
    {    0,    0,    0, 1024, 1024,    0,    0,    0,
         0,    0, 1024, 1024, 1024, 1024,    0,    0,
         0, 1024, 1024,    0,    0, 1024, 1024,    0,
      1024, 1024,    0,    0,    0,    0, 1024, 1024,
      1024, 1024,    0,    0,    0,    0, 1024, 1024,
         0, 1024, 1024,    0,    0, 1024, 1024,    0,
         0,    0, 1024, 1024, 1024, 1024,    0,    0,
         0,    0,    0, 1024, 1024,    0,    0,    0 },
    { 0, 0, 0, 0, 0, 0, 1024, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    }, 
    {  // horizontal line
    {    0,    0,    0,    0,    0,    0,    0,    0,
         0,    0,    0,    0,    0,    0,    0,    0,
      1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024,
      1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024,
      1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024,
      1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024,
         0,    0,    0,    0,    0,    0,    0,    0,
         0,    0,    0,    0,    0,    0,    0,    0 },
    { 0, 0, 0, 0, 0, 0, 0, 1024, 0, 0, 0, 0, 0, 0, 0, 0 }
    }, 
    {  // vertical line
    {    0,    0, 1024, 1024, 1024, 1024,    0,    0,
         0,    0, 1024, 1024, 1024, 1024,    0,    0,
         0,    0, 1024, 1024, 1024, 1024,    0,    0,
         0,    0, 1024, 1024, 1024, 1024,    0,    0,
         0,    0, 1024, 1024, 1024, 1024,    0,    0,
         0,    0, 1024, 1024, 1024, 1024,    0,    0,
         0,    0, 1024, 1024, 1024, 1024,    0,    0,
         0,    0, 1024, 1024, 1024, 1024,    0,    0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 1024, 0, 0, 0, 0, 0, 0, 0 }
    }, 
    {  // slash
    {    0,    0,    0,    0,    0,    0, 1024, 1024,
         0,    0,    0,    0,    0,    0, 1024, 1024,
         0,    0,    0,    0,    0, 1024,    0,    0,
         0,    0,    0, 1024, 1024,    0,    0,    0,
         0,    0,    0, 1024, 1024,    0,    0,    0,
         0,    0, 1024,    0,    0,    0,    0,    0,
      1024, 1024,    0,    0,    0,    0,    0,    0,
      1024, 1024,    0,    0,    0,    0,    0,    0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1024, 0, 0, 0, 0, 0, 0 }
    }, 
    {  // backslash
    { 1024, 1024,    0,    0,    0,    0,    0,    0,
      1024, 1024,    0,    0,    0,    0,    0,    0,
         0,    0, 1024,    0,    0,    0,    0,    0,
         0,    0,    0, 1024, 1024,    0,    0,    0,
         0,    0,    0, 1024, 1024,    0,    0,    0,
         0,    0,    0,    0,    0, 1024,    0,    0,
         0,    0,    0,    0,    0,    0, 1024, 1024,
         0,    0,    0,    0,    0,    0, 1024, 1024 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1024, 0, 0, 0, 0, 0 }
    }, 
    {  // up arrow
    {    0,    0,    0, 1024, 1024,    0,    0,    0,
         0,    0,    0, 1024, 1024,    0,    0,    0,
         0,    0, 1024, 1024, 1024, 1024,    0,    0,
         0,    0, 1024, 1024, 1024, 1024,    0,    0,
         0, 1024, 1024,    0,    0, 1024, 1024,    0,
         0, 1024, 1024,    0,    0, 1024, 1024,    0,
      1024, 1024,    0,    0,    0,    0, 1024, 1024,
      1024, 1024,    0,    0,    0,    0, 1024, 1024 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1024, 0, 0, 0, 0 }
    }, 
    {  // down arrow
    { 1024, 1024,    0,    0,    0,    0, 1024, 1024,
      1024, 1024,    0,    0,    0,    0, 1024, 1024,
         0, 1024, 1024,    0,    0, 1024, 1024,    0,
         0, 1024, 1024,    0,    0, 1024, 1024,    0,
         0,    0, 1024, 1024, 1024, 1024,    0,    0,
         0,    0, 1024, 1024, 1024, 1024,    0,    0,
         0,    0,    0, 1024, 1024,    0,    0,    0,
         0,    0,    0, 1024, 1024,    0,    0,    0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1024, 0, 0, 0 }
    }, 
    {  // right arrow
    { 1024, 1024,    0,    0,    0,    0,    0,    0,
      1024, 1024, 1024, 1024,    0,    0,    0,    0,
         0,    0, 1024, 1024, 1024, 1024,    0,    0,
         0,    0,    0,    0,    0, 1024, 1024, 1024,
         0,    0,    0,    0,    0, 1024, 1024, 1024,
         0,    0, 1024, 1024, 1024, 1024,    0,    0,
      1024, 1024, 1024, 1024,    0,    0,    0,    0,
      1024, 1024,    0,    0,    0,    0,    0,    0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1024, 0, 0 }
    }, 
    {  // left arrow
    {    0,    0,    0,    0,    0,    0, 1024, 1024,
         0,    0,    0,    0, 1024, 1024, 1024, 1024,
         0,    0, 1024, 1024, 1024, 1024,    0,    0,
      1024, 1024, 1024,    0,    0,    0,    0,    0,
      1024, 1024, 1024,    0,    0,    0,    0,    0,
         0,    0, 1024, 1024, 1024, 1024,    0,    0,
         0,    0,    0,    0, 1024, 1024, 1024, 1024,
         0,    0,    0,    0,    0,    0, 1024, 1024 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1024, 0 }
    }, 
    {  // blank
    {    0,    0,    0,    0,    0,    0,    0,    0,
         0,    0,    0,    0,    0,    0,    0,    0,
         0,    0,    0,    0,    0,    0,    0,    0,
         0,    0,    0,    0,    0,    0,    0,    0,
         0,    0,    0,    0,    0,    0,    0,    0,
         0,    0,    0,    0,    0,    0,    0,    0,
         0,    0,    0,    0,    0,    0,    0,    0,
         0,    0,    0,    0,    0,    0,    0,    0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1024 }
    }, 
};

/*
 * The weights are saved in this array.
 * Access the weight through the macros below
 *
 * For accessing weight from input 2 to hidden 3 use:
 *    W_IN_HIDDEN(2,3)
 */

int weights[NUM_INPUT*NUM_HIDDEN + NUM_HIDDEN*NUM_OUTPUT];

#define W_IN_HIDDEN(i, h)  weights[i*NUM_HIDDEN + h]
#define W_HIDDEN_OUT(h, o) weights[NUM_INPUT*NUM_HIDDEN + h*NUM_OUTPUT + o]

int neurons[NUM_INPUT + NUM_HIDDEN + NUM_OUTPUT];
int teach_neurons[NUM_OUTPUT];

#define N_IN(i)     neurons[i]
#define N_HIDDEN(h) neurons[NUM_INPUT + h]
#define N_OUT(o)    neurons[NUM_INPUT + NUM_HIDDEN + o]
#define N_TEACH(o)  teach_neurons[o]

int error[NUM_OUTPUT + NUM_HIDDEN];

#define E_HIDDEN(h) error[h]
#define E_OUT(o)    error[NUM_HIDDEN + o]

void
init_network(void)
{
    int mod = 32, n, h, o;
    int w, x = 32;

    for(n=0; n < sizeof(weights) / sizeof(int); n++) {
        w = rand() % mod;
        if(rand() < 0x00008000)
            w = -w;

        weights[n] = w * x;
    }

    return;
}

void
init_pattern(void)
{
    int pat = rand() % NUM_PATTERNS;
    int i, o;

    for(i=0; i < NUM_INPUT; i++)
        N_IN(i) = pattern[pat].p_in[i]; 

    for(o=0; o < NUM_OUTPUT; o++)
        N_TEACH(o) = pattern[pat].p_out[o];

    return;
}

void
set_pattern(int pat)
{
    int i, o;

    for(i=0; i < NUM_INPUT; i++)
        N_IN(i) = pattern[pat].p_in[i]; 

    for(o=0; o < NUM_OUTPUT; o++)
        N_TEACH(o) = pattern[pat].p_out[o];

    return;
}

int
f_logic(int x)
{
    /*
     * Sigmoid function approximation using piecewise linear approximation 
     * of a nonlinear function (PLAN) - proposed by Amin, Curtis & Hayes-Gill
     * IEE Proc Circuits, 1997
     */

    static int c = 1;
    int ret;
    int neg;

    neg = 0;
    if (x < 0) {
        neg = 1;
        x = -x;
    }

    if (x > 5120)
        ret = 1024;
    else if (x > 2432)
        ret = (x * 32) / 1024 + 864;
    else if (x > 1024)
        ret = (x * 128) / 1024 + 640;
    else
        ret = (x * 256) / 1024 + 512;
    
    if (neg)
        ret = 1024 - ret;

    return ret;
}


void
calculate_network(void)
{
    int i,h,o;

    for(h=0; h < NUM_HIDDEN; h++)
        N_HIDDEN(h) = 0.0;

    for(o=0; o < NUM_OUTPUT; o++)
        N_OUT(o) = 0.0;

    for(h=0; h < NUM_HIDDEN; h++) {
        for(i=0; i < NUM_INPUT; i++)
            N_HIDDEN(h) += (N_IN(i) * W_IN_HIDDEN(i, h)) / 1024;

        N_HIDDEN(h) = f_logic(N_HIDDEN(h));
    }

    for(o=0; o < NUM_OUTPUT; o++) {
        for(h=0; h < NUM_HIDDEN; h++)
            N_OUT(o) +=( N_HIDDEN(h) * W_HIDDEN_OUT(h, o)) / 1024;

        N_OUT(o) = f_logic(N_OUT(o));
    }
        
    return;
}


void
calculate_errors(void)
{
    int h, o;
    int err;

    for(o=0; o < NUM_OUTPUT; o++)
        E_OUT(o) = (N_TEACH(o) - N_OUT(o));

    for(h=0; h < NUM_HIDDEN; h++) {
        err = 0.0;
        for(o=0; o < NUM_OUTPUT; o++)
            err += (E_OUT(o) * W_HIDDEN_OUT(h, o)) / 1024;

        E_HIDDEN(h) = (((N_HIDDEN(h) * (1024 - N_HIDDEN(h))) / 1024) * err) / 1024;
    }

    return;
}

void
train_network(int num)
{
    int i, h, o;
    unsigned int n;


    for(n = 0; n < num; n++) {

        init_pattern();
        calculate_network();
        calculate_errors();
    
        for(h=0; h < NUM_HIDDEN; h++)
            for(o=0; o < NUM_OUTPUT; o++)
                W_HIDDEN_OUT(h, o) += (((learn * N_HIDDEN(h)) / 1024) * E_OUT(o)) / 1024;

        for(i=0; i < NUM_INPUT; i++)
            for(h=0; h < NUM_HIDDEN; h++)
                W_IN_HIDDEN(i, h) += (((learn * N_IN(i)) / 1024) * E_HIDDEN(h)) / 1024;

    }

    return;
}

