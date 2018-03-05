/*
 * thread1.c
 *
 *  Created on: 19 Feb 2018
 *      Author: dpatoukas
 */
#include "ink.h"
#include "app.h"
#include "timer.h"
//bitcount specific definitions 

#define SEED 4L
#define ITER 100
#define CHAR_BIT 8


__nv static char bits[256] =
{
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,  /* 0   - 15  */
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,  /* 16  - 31  */
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,  /* 32  - 47  */
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,  /* 48  - 63  */
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,  /* 64  - 79  */
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,  /* 80  - 95  */
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,  /* 96  - 111 */
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,  /* 112 - 127 */
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,  /* 128 - 143 */
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,  /* 144 - 159 */
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,  /* 160 - 175 */
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,  /* 176 - 191 */
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,  /* 192 - 207 */
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,  /* 208 - 223 */
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,  /* 224 - 239 */
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8   /* 240 - 255 */
};

// define task-shared persistent variables.
__shared(

    uint8_t pinCont;
    unsigned n_0;
    unsigned n_1;
    unsigned n_2;
    unsigned n_3;
    unsigned n_4;
    unsigned n_5;
    unsigned n_6;
    unsigned function;
    unsigned iteration;
    uint32_t seed;
)

TASK(t_init);
TASK(t_select_func);
TASK(t_bit_count);
TASK(t_bitcount);
TASK(t_ntbl_bitcnt);
TASK(t_ntbl_bitcount);
TASK(t_BW_btbl_bitcount);
TASK(t_AR_btbl_bitcount);
TASK(t_bit_shifter);
TASK(t_end);

// called at the very first boot
void thread3_init(){
    // create a thread with priority 15 and entry task t_init
    __CREATE(THREAD3,t_init);
#ifdef WKUP_TIMER
    set_wkup_timer(THREAD3,60000);
#endif
}


TASK(t_init){
    


    __SET(pinCont,1);
    __SET(function,0);
    __SET(n_0,0);
    __SET(n_1,0);
    __SET(n_2,0);
    __SET(n_3,0);
    __SET(n_4,0);
    __SET(n_5,0);
    __SET(n_6,0);

    // next task is t_select_func
    return t_select_func;
}

TASK(t_select_func){

    /* ------ local variables for persistents ------*/
    uint32_t seed;
    unsigned function;
    unsigned iteration;

    #ifdef RANDOM   
        seed = rand();   
    #else 
        seed = SEED;
    #endif

    __SET(iteration, 0);
    __SET(seed,seed);
    
    function = __GET(function);

    if (function == 0)
    {
        __SET(function,++function);
        return t_bit_count;
    }
    else if(function == 1)
    {
        __SET(function,++function);
        return t_bitcount;
    }
    else if(function == 2)
    {
        __SET(function,++function);
        return t_ntbl_bitcnt;          
    }
    else if(function == 3)
    {
        __SET(function,++function);
        return t_ntbl_bitcount;
    }
    else if(function == 4)
    {
        __SET(function,++function);
        return t_BW_btbl_bitcount;         
    }
    else if(function == 5)
    {
        __SET(function,++function);
        return t_AR_btbl_bitcount;
    }
    else if(function == 6)
    {
        __SET(function,++function);
        return t_bit_shifter;          
    }
    else 
    {
        __SET(function,++function);
        return t_end;
    }

}

TASK(t_bit_count){
        
        uint32_t seed = __GET(seed);
        unsigned n_0 = __GET(n_0);
        unsigned iteration = __GET(iteration);

        uint32_t tmp_seed = seed;
        seed = tmp_seed + 13;
        __SET(seed,seed);

        unsigned temp = 0;
        if(tmp_seed) do
            temp++;
        while (0 != (tmp_seed = tmp_seed&(tmp_seed-1)));
        
        __SET(n_0, n_0 += temp);
        __SET(iteration, ++iteration);

        if(iteration < ITER){
            return t_bit_count;
        }
        else{
            return t_select_func;
        }    

}

TASK(t_bitcount){

    uint32_t seed = __GET(seed);
    unsigned n_1 = __GET(n_1);
    unsigned iteration = __GET(iteration);

    uint32_t tmp_seed = seed;
    seed = tmp_seed + 13;
    __SET(seed,seed);

    tmp_seed = ((tmp_seed & 0xAAAAAAAAL) >>  1) + (tmp_seed & 0x55555555L);
    tmp_seed = ((tmp_seed & 0xCCCCCCCCL) >>  2) + (tmp_seed & 0x33333333L);
    tmp_seed = ((tmp_seed & 0xF0F0F0F0L) >>  4) + (tmp_seed & 0x0F0F0F0FL);
    tmp_seed = ((tmp_seed & 0xFF00FF00L) >>  8) + (tmp_seed & 0x00FF00FFL);
    tmp_seed = ((tmp_seed & 0xFFFF0000L) >> 16) + (tmp_seed & 0x0000FFFFL);
    
    __SET(n_1,n_1 += (int)tmp_seed);
    __SET(iteration,++iteration);

    if(iteration < ITER){
        return t_bitcount;
    }
    else{
        return t_select_func;
    }

}
//Helper functions

int recursive_cnt(uint32_t x){
    int cnt = bits[(int)(x & 0x0000000FL)];

    if (0L != (x >>= 4))
        cnt += recursive_cnt(x);

    return cnt;
}

int non_recursive_cnt(uint32_t x){
    int cnt = bits[(int)(x & 0x0000000FL)];

    while (0L != (x >>= 4)) {
        cnt += bits[(int)(x & 0x0000000FL)];
    }

    return cnt;
}

TASK(t_ntbl_bitcnt){

    uint32_t seed = __GET(seed);
    unsigned n_2 = __GET(n_2);
    unsigned iteration = __GET(iteration);

    uint32_t tmp_seed = seed;
    n_2 += non_recursive_cnt(tmp_seed);
    seed = tmp_seed + 13;
    iteration++;

    __SET(n_2,n_2);
    __SET(seed, seed);
    __SET(iteration,iteration);

    if(iteration < ITER){
        return t_ntbl_bitcnt;
    }
    else{
        return t_select_func;
    }
}

TASK(t_ntbl_bitcount){
    /* ------ local variables for persistents ------*/
    uint32_t seed = __GET(seed);
    unsigned n_3 = __GET(n_3);
    unsigned iteration = __GET(iteration);

    uint16_t __cry = seed;
    n_3 += bits[ (int) (__cry & 0x0000000FUL)       ] +
        bits[ (int)((__cry & 0x000000F0UL) >> 4) ] +
        bits[ (int)((__cry & 0x00000F00UL) >> 8) ] +
        bits[ (int)((__cry & 0x0000F000UL) >> 12)] +
        bits[ (int)((__cry & 0x000F0000UL) >> 16)] +
        bits[ (int)((__cry & 0x00F00000UL) >> 20)] +
        bits[ (int)((__cry & 0x0F000000UL) >> 24)] +
        bits[ (int)((__cry & 0xF0000000UL) >> 28)];

    uint32_t tmp_seed = seed;
    seed = tmp_seed + 13;
    iteration++;

    __SET(n_3,n_3);
    __SET(seed, seed);
    __SET(iteration,iteration);

    if(iteration < ITER){
        return t_ntbl_bitcount;
    }
    else{
        return t_select_func;
    }
}

TASK(t_BW_btbl_bitcount){

    /* ------ local variables for persistents ------*/
    uint32_t seed = __GET(seed);
    unsigned n_4 = __GET(n_4);
    unsigned iteration = __GET(iteration);

    union
    {
        unsigned char ch[4];
        long y;
    } U;

    U.y = seed;

    n_4 += bits[ U.ch[0] ] + bits[ U.ch[1] ] +
        bits[ U.ch[3] ] + bits[ U.ch[2] ];
    uint32_t tmp_seed = seed;
    seed = tmp_seed + 13;
    iteration++;

    __SET(n_4,n_4);
    __SET(seed, seed);
    __SET(iteration,iteration);

    if(iteration < ITER){
        return t_BW_btbl_bitcount;
    }
    else{
        return t_select_func;
    }

}

TASK(t_AR_btbl_bitcount) {

    /* ------ local variables for persistents ------*/
    uint32_t seed = __GET(seed);
    unsigned n_5 = __GET(n_5);
    unsigned iteration = __GET(iteration);

    unsigned char * Ptr = (unsigned char *) &seed ;
    int Accu ;

    Accu  = bits[ *Ptr++ ];
    Accu += bits[ *Ptr++ ];
    Accu += bits[ *Ptr++ ];
    Accu += bits[ *Ptr ];
    n_5+= Accu;
    uint32_t tmp_seed = seed;
    seed = tmp_seed + 13;
    iteration++;

    __SET(n_5,n_5);
    __SET(seed, seed);
    __SET(iteration,iteration);

    if(iteration < ITER){
        return t_AR_btbl_bitcount;
    }
    else{
        return t_select_func;
    }
}

TASK(t_bit_shifter){
        /* ------ local variables for persistents ------*/
    uint32_t seed = __GET(seed);
    unsigned n_6 = __GET(n_6);
    unsigned iteration = __GET(iteration);

    unsigned i, nn;
    uint32_t tmp_seed = seed;
    for (i = nn = 0; tmp_seed && (i < (sizeof(long) * CHAR_BIT)); ++i, tmp_seed >>= 1)
        nn += (unsigned)(tmp_seed & 1L);
    n_6 += nn;
    tmp_seed = seed;
    tmp_seed += 13;
    seed = tmp_seed;

    iteration++;

    __SET(n_6,n_6);
    __SET(seed, seed);
    __SET(iteration,iteration);

    if(iteration < ITER){
        return t_bit_shifter;
    }
    else{
        return t_select_func;
    }
}

TASK(t_end){
        /* ------ local variables for persistents ------*/
    uint8_t pinCont = __GET(pinCont);
    /* ------------------------------------- */

    if (pinCont){
        P3OUT |=BIT5;
        P3OUT &=~BIT5;
    }
#ifdef RAISE_PIN
        __port_on(3, 4);
        __port_off(3, 4);
#endif


    __SET(pinCont,0);

    return t_init;
}
