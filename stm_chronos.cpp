#include <stm_chronos.hpp>
#include <fstream>
/*************** Debug start *******************/
#include "/usr/local/rstm_r5/stm/support/atomic_ops.h"
/*************** Debug end *******************/
#include <sys/types.h>
#include <sys/syscall.h>

#define gettid() syscall(SYS_gettid)

unsigned long mask1 = 1;
unsigned long mask2=2;
unsigned long mask3=3;
cpu_set_t gen_mask;
unsigned int len=sizeof(gen_mask);
double final_stm_slope=0.612601;
double final_lock_free_slope=0.612601;
vector<vector<double> > gen_obj_tasks;

vector<double> sh_obj;


void* warmup(void* s)
{
    //This function is the same as task1 in the program determining the stm slope
    int thread_id=gettid();
    int count=100;     //Be sure that the calculated slope is the most right one
    bool cond=false;  // check whether two consecuitive measures for stm are not highly distinguished
    unsigned long diff;
    struct timespec start1, end1;
    int num_loop=0;
    double est_stm_slope=0;     //Holds the current estimate for stm_slope
    double prev_stm_slope=0;       //Holds the previous estimate for stm_slope
    double thr_stm_slope=1;    //threshold between two consecuitive measures for stm_slope
    struct sched_param param;
    thread_args_m* args    = (thread_args_m*)s;
    args->id=thread_id;
    CounterBench* b = args->b;
    pthread_detach(pthread_self());
    param.sched_priority = RUN_PRIO;
    sched_setaffinity(0, len, (cpu_set_t *) &(args->in_mask));
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
    begin_rtseg_selfbasic(RUN_PRIO, NULL, NULL);
    stm::init("Polka","invis-eager",false);
    do{
        num_loop+=10;
        prev_stm_slope=est_stm_slope;
        clock_gettime(CLOCK_REALTIME, &start1);
        /**********************************************************************/
	int cur_th_ptr = gettid();
        struct task_in_param tt;
        tt.time_param=&start1;
	//void* tt = &start1;
        b->multi_reset(num_loop,(void*)(&tt),sh_obj,cur_th_ptr,1);
        /**********************************************************************/
        clock_gettime(CLOCK_REALTIME, &end1);
        diff=subtract_ts_mo(&start1,&end1);
        est_stm_slope=(double)diff/num_loop;
        if(!(cond=abs(est_stm_slope-prev_stm_slope)>thr_stm_slope))
            count--;
        //cout<<"start:"<<start1.tv_sec<<" sec, "<<start1.tv_nsec<<" nsec, end:"<<end1.tv_sec<<" sec, "<<end1.tv_nsec<<" nsec"<<", diff:"<<diff<<", num_loop:"<<num_loop<<", stm_slope:"<<est_stm_slope<<", count:"<<count<<endl;
    }while(cond||count);
    final_stm_slope=(double)(est_stm_slope+prev_stm_slope)/2000;//Take the mean of the last two values
                                                                //then divide by 1000 to change from nsec to usec
    //cout<<"Final stm slope:"<<final_stm_slope<<endl;
    stm::shutdown_nodeb(thread_id);    
    end_rtseg_self(END_PRIO);
    pthread_exit(NULL);
}

void* warmup_lock_free(void* s)
{
    //This function is the same as task1 in the program determining the stm slope    
    int thread_id=gettid();
    int count=100;     //Be sure that the calculated slope is the most right one
    bool cond=false;  // check whether two consecuitive measures for stm are not highly distinguished
    unsigned long diff;
    struct timespec start1, end1;
    int num_loop=0;
    double est_lock_free_slope=0;     //Holds the current estimate for stm_slope
    double prev_lock_free_slope=0;       //Holds the previous estimate for stm_slope
    double thr_lock_free_slope=1;    //threshold between two consecuitive measures for stm_slope
    struct sched_param param;
    unsigned long long dum;     //dummy variable to be passed as a pointer to multi_reset_lock_free
    thread_args_m* args    = (thread_args_m*)s;
    args->id=thread_id;
    CounterBench* b = args->b;
    pthread_detach(pthread_self());
    param.sched_priority = RUN_PRIO;
    sched_setaffinity(0, len, (cpu_set_t *) &(args->in_mask));
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
    begin_rtseg_selfbasic(RUN_PRIO, NULL, NULL);
    stm::init("Polka", "invis-eager",false);
    do{
        num_loop+=10;
        prev_lock_free_slope=est_lock_free_slope;
        clock_gettime(CLOCK_REALTIME, &start1);
        b->multi_reset_lock_free(num_loop,0,sh_obj);
        clock_gettime(CLOCK_REALTIME, &end1);
        diff=subtract_ts_mo(&start1,&end1);
        est_lock_free_slope=(double)diff/num_loop;
        if(!(cond=abs(est_lock_free_slope-prev_lock_free_slope)>thr_lock_free_slope))
            count--;
        //cout<<"start:"<<start1.tv_sec<<" sec, "<<start1.tv_nsec<<" nsec, end:"<<end1.tv_sec<<" sec, "<<end1.tv_nsec<<" nsec"<<", diff:"<<diff<<", num_loop:"<<num_loop<<", stm_slope:"<<est_stm_slope<<", count:"<<count<<endl;
    }while(cond||count);
    final_lock_free_slope=(double)(est_lock_free_slope+prev_lock_free_slope)/2000;//Take the mean of the last two values
                                                                //then divide by 1000 to change from nsec to usec
    end_rtseg_self(END_PRIO);
    pthread_exit(NULL);
}

//void main_warmup(vector<unsigned long> no_proc){
void main_warmup(int no_proc){
    //no_proc is the number of processors in the system
    // This function is similar to the main function in the program determinig the stm slope
    /********************* new 1 start ***********************/
    ofstream stm_slope_file("/usr/local/stm_slope_file");
    if(!stm_slope_file.is_open()){
        cout<<"Cannot open stm_slope file"<<endl;
        exit(0);
    }
    /********************* new 1 end ***********************/
    stm::init("Polka", "invis-eager",false);
    pthread_t t1;
    thread_args_m args1;
    CounterBench c;
    c.setMaxObjNo(1);           //Set only one shared object
    if(sh_obj.empty()){         //Initialize the shared object
        sh_obj.push_back(0);
    }
    struct sched_param param;
    param.sched_priority = MAIN_PRIO;
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
    args1.b=&c;
    /******************* Debug 1 start ********************/
    //cout<<"number of processors is "<<no_proc<<endl;
    /******************* Debug 1 end ********************/
    for(int i=0;i<no_proc;i++){
        CPU_ZERO(&gen_mask);
        CPU_SET(i,&gen_mask);
        sched_setaffinity(0, len, &gen_mask);
        pthread_create(&t1, NULL, warmup, (void*)&args1);
        pthread_join(t1, NULL);
        /********************* new 2 start ********************/
        stm_slope_file<<final_stm_slope<<endl;
        /********************* new 2 end ********************/
    }
    /********************* new 4 start **********************/
    stm_slope_file.close();
    /********************* new 4 end **********************/
    stm::shutdown_nodeb(0);
}

void main_warmup_lock_free(int no_proc){
    //no_proc is the number of processors in the system
    // This function is similar to the main function in the program determinig the stm slope
    /************************ new 3 start *************************/
    ofstream lock_free_slope_file("/usr/local/lock_free_slope");
    if(!lock_free_slope_file.is_open()){
        cout<<"Cannot open lock_free_slope_file"<<endl;
        exit(0);
    }
    /************************ new 3 end *************************/
    stm::init("Polka", "invis-eager",false);
    pthread_t t1;
    thread_args_m args1;
    CounterBench c;
    c.setMaxObjNo(1);           //Set only one shared object
    if(sh_obj.empty()){         //Initialize the shared object
        sh_obj.push_back(0);
    }
    struct sched_param param;
    param.sched_priority = MAIN_PRIO;
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
    args1.b=&c;
    /******************* Debug 1 start ********************/
    //cout<<"number of processors is "<<no_proc<<endl;
    /******************* Debug 1 end ********************/
    for(int i=0;i<no_proc;i++){
        CPU_ZERO(&gen_mask);
        CPU_SET(i,&gen_mask);
        sched_setaffinity(0, len, &gen_mask);
        pthread_create(&t1, NULL, warmup_lock_free, (void*)&args1);
        pthread_join(t1, NULL);
        /********************** new 5 start ******************************/
        lock_free_slope_file<<final_lock_free_slope<<endl;
        /********************** new 5 end ******************************/
    }
    /********************** new 6 start ****************************/
    lock_free_slope_file.close();
    /********************** new 6 end ****************************/
}

void* warmup_lock_free_test(void* s)
{
    //This function is the same as task1 in the program determining the stm slope
    int thread_id=gettid();
    int count=100;     //Be sure that the calculated slope is the most right one
    bool cond=false;  // check whether two consecuitive measures for stm are not highly distinguished
    unsigned long diff;
    struct timespec start1, end1;
    int num_loop=0;
    double est_lock_free_slope=0;     //Holds the current estimate for stm_slope
    double prev_lock_free_slope=0;       //Holds the previous estimate for stm_slope
    double thr_lock_free_slope=1;    //threshold between two consecuitive measures for stm_slope
    unsigned long dum;     //dummy variable to be used by multi_reset_lock_free
    struct sched_param param;
    thread_args_m* args    = (thread_args_m*)s;
    args->id=thread_id;
    pthread_detach(pthread_self());
    param.sched_priority = RUN_PRIO;
    sched_setaffinity(0, len, (cpu_set_t *) &(args->in_mask));
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
    begin_rtseg_selfbasic(RUN_PRIO, NULL, NULL);
    do{
        num_loop+=10;
        prev_lock_free_slope=est_lock_free_slope;
        clock_gettime(CLOCK_REALTIME, &start1);
        for(int i=0;i<num_loop;i++){
			fas(&dum,0);
	}
        clock_gettime(CLOCK_REALTIME, &end1);
        diff=subtract_ts_mo(&start1,&end1);
        est_lock_free_slope=(double)diff/num_loop;
        if(!(cond=abs(est_lock_free_slope-prev_lock_free_slope)>thr_lock_free_slope))
            count--;
        //cout<<"start:"<<start1.tv_sec<<" sec, "<<start1.tv_nsec<<" nsec, end:"<<end1.tv_sec<<" sec, "<<end1.tv_nsec<<" nsec"<<", diff:"<<diff<<", num_loop:"<<num_loop<<", stm_slope:"<<est_stm_slope<<", count:"<<count<<endl;
    }while(cond||count);
    final_lock_free_slope=(double)(est_lock_free_slope+prev_lock_free_slope)/2000;//Take the mean of the last two values
                                                                //then divide by 1000 to change from nsec to usec
    /********************** Debug-start-1 ****************************/
    cout<<"Final Lock-free_test slope:"<<final_lock_free_slope<<endl;
    /********************** Debug-end-1 ****************************/
    end_rtseg_self(END_PRIO);
    pthread_exit(NULL);
}

void main_warmup_lock_free_test(int no_proc){
    //no_proc is the number of processors in the system
    // This function is similar to the main function in the program determinig the stm slope
    pthread_t t1;
    thread_args_m args1;
    struct sched_param param;
    param.sched_priority = MAIN_PRIO;
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
    /******************* Debug 1 start ********************/
    //cout<<"number of processors is "<<no_proc<<endl;
    /******************* Debug 1 end ********************/
    for(int i=0;i<no_proc;i++){
        CPU_ZERO(&gen_mask);
        CPU_SET(i,&gen_mask);
        sched_setaffinity(0, len, &gen_mask);
        pthread_create(&t1, NULL, warmup_lock_free_test, (void*)&args1);
        pthread_join(t1, NULL);
    }
}

unsigned long long subtract_ts_mo(struct timespec *first, struct timespec *last) {
	/*
	 * subtracts first from last and returns the difference in unsigned long long form
	 */
	signed long nsec;
	unsigned long long int time;

	nsec = last->tv_nsec - first->tv_nsec;
	if(nsec < 0) {
		time = BILLION + nsec;
		time += ((unsigned long long)(last->tv_sec - first->tv_sec - 1))*BILLION;
	} else {
		time = nsec + ((unsigned long long)(last->tv_sec - first->tv_sec))*BILLION;
	}

	return time;
}

struct timespec add_ts(struct timespec* first, struct timespec* second){
    unsigned long long nsec,sec;
    struct timespec result;
    nsec=(unsigned long long)(first->tv_nsec+ second->tv_nsec);
    sec=(unsigned long long)(first->tv_sec+ second->tv_sec);
    while(nsec>=BILLION){
        nsec-=BILLION;
        sec++;
    }
    result.tv_nsec=nsec;
    result.tv_sec=sec;
    return result;
}

vector<double> genObj_checkpoint(int num_obj,double min_obj_no,int num_tasks,int task_id,double sh_obj_per){
	/*
	 * Generates objects for all tasks in a pattern suitable for CHECKPOINTING. Non-shared objects
	 * are generated first, then shared objects. Percentage of shared objects relative to total number
	 * of objects per task is specified by sh_obj_per
	 */
/**************** DEBUG 1 ST *****************/
//cout<<"genObj_checkpoint for task_id "<<task_id<<endl;
/************** DEBUG 1 END *****************/
if(gen_obj_tasks.size()==0){
		if(sh_obj_per<0){
			sh_obj_per=SH_OBJ_PER;
		}
		vector<double> obj_inx;        //vector of all available objects
		vector<vector <double> > gen_obj;        //vector of distinct generated objects to be allocated for the current transaction
		int total_sh_obj=(int)floor(sh_obj_per*num_obj);	//Total number of shared objects for all tasks
		int total_nsh_obj=num_obj-total_sh_obj;	//Total number of non shared objects for all tasks
		if(sh_obj_per!=1 && total_nsh_obj<num_tasks){
			cout<<"*** THERE ARE NO ENOUGH INDEPENDNT OBJECTS FOR TASKS ***"<<endl;
			exit(0);
		}
		int max_nsh_obj_task=total_nsh_obj/num_tasks;	//Max number of non shared objects per task
		if((double)max_nsh_obj_task/total_nsh_obj<min_obj_no){
			min_obj_no=(double)max_nsh_obj_task/total_nsh_obj;
	//		cout<<"*** MIN OBJECTS REQUIREMENT WILL BE MODIFIED TO "<<min_obj_no<<" ***"<<endl;
		}
		int max_sh_obj_task=max_nsh_obj_task/(1-sh_obj_per)-max_nsh_obj_task;	//Max number of shared objects per task

		int obj_num_prev_tasks=0;	//Records total number of non shared objects allocated to previous tasks
		for(int j=0;j<num_tasks;j++){
			//Specify number of objects for current task. Also number of shared and non-shared objects
			vector<double> cur_gen_obj;	//generated objects for current task
			int min_obj_no_tmp=(int)(ceil(min_obj_no*num_obj));
			int max_obj_no=((int)(rand())%(num_obj-min_obj_no_tmp))+min_obj_no_tmp;
			max_obj_no=max_obj_no>max_nsh_obj_task+max_sh_obj_task?min_obj_no_tmp:max_obj_no;
			int cur_task_sh_obj=sh_obj_per*max_obj_no;	//number of shared objects for current task
			int cur_task_nsh_obj=max_obj_no-cur_task_sh_obj;	//number of non shared objects for current task

			/*
			 * Generate non-shared objects for current task
			 */
			for(int i=obj_num_prev_tasks;i<obj_num_prev_tasks+cur_task_nsh_obj;i++){
				cur_gen_obj.push_back(i);
			}

			/*
			 * Generate shared objects for current task
			 */
			//First, generate a vector of all shared objects, knowing that shared objects always start
			//after non-shared objects
			vector<double> all_sh_obj;
			for(int i=total_nsh_obj;i<num_obj;i++){
				all_sh_obj.push_back(i);
			}
			//Now, assign shared objects for current task
			for(int i=0;i<cur_task_sh_obj;i++){
				int tmp_sh_size=all_sh_obj.size();
				if(tmp_sh_size==0){
					//There is no more shared objects to allocate for current task
					break;
				}
				int obj_loc=rand()%tmp_sh_size;	//location of chosen shared object
				cur_gen_obj.push_back(all_sh_obj[obj_loc]);
				all_sh_obj.erase(all_sh_obj.begin()+obj_loc);
			}
			obj_num_prev_tasks+=cur_task_nsh_obj;
			gen_obj.push_back(cur_gen_obj);
			/******************* DBEUG 2 ST *****************/
/*			cout<<"Objects that will be given now are "<<endl;
			for(int k=0;k<cur_gen_obj.size();k++){
				cout<<cur_gen_obj[k]<<",";
			}
			cout<<endl;
*/
			/***************** DEBUG 2 END *******************/
		}
		/*
		 * return final object vector for all tasks
		 */
		gen_obj_tasks=gen_obj;
	}
	/************ DEBG 1 ST ****************/
/*	cout<<"objects vector size is "<<gen_obj_tasks.size()<<endl;
	if(&gen_obj_tasks[task_id]){
		cout<<"Objects for task "<<task_id<<" have been generated"<<endl;
	}
	else{
		cout<<"Objects for task "<<task_id<<" have NOT been generated"<<endl;
	}
	for(int i=0;i<gen_obj_tasks.size();i++){
		cout<<"Generated objects for task "<<i<<endl;
		for(int j=0;j<gen_obj_tasks[i].size();j++){
			cout<<gen_obj_tasks[i][j]<<",";
		}
		cout<<endl;
	}
*/
	/********** DEBUG 1 END ***************/
	return gen_obj_tasks[task_id];
}
