/* 
 * File:   stm_chronos.hpp
 * Author: root
 *
 * Created on July 26, 2011, 4:30 PM
 */

#ifndef STM_CHRONOS_HPP
#define	STM_CHRONOS_HPP

#include "Counter.hpp"
#include "/usr/local/rstm_r5/stm/cm/ContentionManager.hpp"
#include <iostream>
#include <chronos/chronos.h>
#include <chronos/chronos_utils.h>
#include <vector>
#include <rstm_hlp.hpp>
#include "/usr/local/rstm_r5/stm/support/atomic_ops.h"

#define RUN_PRIO 40
#define END_PRIO 45
#define MAIN_PRIO 99
/*********************** PNF START ******************************/
#define PNF_M_PRIO 46   //Used with PNF CM for executing and initially checking transactions
#define PNF_N_PRIO 39   //Used with PNF CM for retrying transactions
/*********************** PNF END ******************************/
#define MILLION 1000000
#define BILLION 1000000000



using namespace std;
using namespace bench;

extern unsigned long mask1;
extern unsigned long mask2;
extern unsigned long mask3;
extern unsigned int len;
extern double final_stm_slope;        //Default value on my PC which is changed by running the warmup
                                        //Default should be changed according to each PC
extern double final_lock_free_slope;        //Default value on my PC which is changed by running the warmup
                                        //Default should be changed according to each PC
extern vector<vector<double> > gen_obj_tasks;

struct thread_args_m{
    int id;                                     // Thread id
    unsigned long exec;                         //execution time in micro second
    CounterBench* b;
    struct timespec start;
    struct timespec *time_param;                //Holds time parameter (deadline or period) of current CM
    char* cm;                                   // Used contention manager
    void* cm_args;                              // Passed arguments to contention manager
    double psy;                                 //Used in case of LCM             
    unsigned long in_mask;                      // If required, to determine which processor to use
                                                // Useful for the warmup function to run it on all processors one by one
    
};




//void burn_cpu_stm(unsigned long, double,char*,void* cm_args,int,void*);
//void burn_cpu_stm(long long, char*,void* cm_args,int,void*);
//void burn_cpu_stm_deb(long long, char*,void*,int,void*);
extern void* warmup(void*);
extern void main_warmup(int);
extern void* warmup_lock_free(void*);
extern void main_warmup_lock_free(int);
extern unsigned long long subtract_ts_mo(struct timespec *first, struct timespec *last);	//subtracts first from last and returns the difference in unsigned long long form
/************************* Debug start ***************************/
extern void main_warmup_lock_free_test(int no_proc);
/************************* Debug end ***************************/

extern struct timespec add_ts(struct timespec* first, struct timespec* second);
extern vector<double> genObj_checkpoint(int num_obj,double min_obj_no,int num_tasks,int task_id,double sh_obj_per=-1);

/*
void burn_cpu_stm(unsigned long exec_usec, double exec_slope,char* in_cm,void* cm_args,int id,void* b) {
    try{
        long long i;
	long long j;
	long long num_loop;
        CounterBench* c=(CounterBench*)b;
        
        char* cm="Polka";       //Default cm
        
        if(in_cm!=NULL)     // If cm is not specified at input, use default
            cm=in_cm;

	num_loop = (long long)(exec_usec/(exec_slope));           
	for(i = 0; i<num_loop; i++){
            c->reset(cm_args);
        }
    }catch(exception& e){
        cout<<"Exception in burn_cpu_stm:"<<e.what()<<endl;
    }
}

void burn_cpu_stm(long long num_loop1, char* in_cm,void* cm_args,int id,void* b) {
    try{
        CounterBench* c=(CounterBench*)b;
        
        char* cm="Polka";       //Default cm
        
        if(in_cm!=NULL)     // If cm is not specified at input, use default
            cm=in_cm;
	for(int i = 0; i<num_loop1; i++){
            c->reset(cm_args);
        }
    }catch(exception& e){
        cout<<"Exception in burn_cpu_stm:"<<e.what()<<endl;
    }
}

void burn_cpu_stm_deb(long long num_loop1, char* in_cm,void* cm_args,int id,void* b) {
    try{
        CounterBench* c=(CounterBench*)b;
        
        char* cm="Polka";       //Default cm
        
        if(in_cm!=NULL)     // If cm is not specified at input, use default
            cm=in_cm;
	for(int i = 0; i<num_loop1; i++){
            c->reset(cm_args);
        }
    }catch(exception& e){
        cout<<"Exception in burn_cpu_stm:"<<e.what()<<endl;
    }
}
*/

#endif	/* STM_CHRONOS_HPP */

