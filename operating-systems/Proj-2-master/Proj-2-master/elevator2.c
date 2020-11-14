#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/random.h>
#include <linux/mutex.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/syscalls.h>

//										GLOBALS
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Adds random animals and owners to a list and uses proc to"
	"query the stats");

#define ENTRY_NAME "elevator"
#define ENTRY_SIZE 1000
#define PERMS 0644
#define PARENT NULL
#define PASSENGER_PERSON 3
#define PASSENGER_DOG 2
#define PASSENGER_CAT 1
#define PASSENGER_NOPET 0
#define NUM_PASSENGER_TYPES 3
#define MAX_WEIGHT 15
#define NUM_FLOORS 11
#define FLOOR_TOP 10
#define FLOOR_BOTTOM 1

#define KTHREAD_STRING_1 "kthread 1"

static struct file_operations fops;

static char *message;
static int read_p;

typedef enum{OFFLINE, IDLE, LOADING, UP, DOWN}ElevatorState;

static struct mutex shared_data_mutex;
static struct task_struct* kthread;

u_int8_t floors_w_req[11];

struct
{
	int num_pas,
		weight,
		pet_type,
		num_req,
		num_served;
	u_int8_t cur_flr,
		highest_flr,
		lowest_flr;
	uint16_t stop_flr;
	ElevatorState State;
	struct list_head list;
}elevator;

struct
{
	int total_cnt;
	int total_weight;
	int req_up;
	int req_dwn;
	struct list_head list;
}floor_list[11];

typedef struct
{
	int num_pets;
	int pet_type;
	int start_floor;
	int dest_floor;
	int weight;
	char owner_char;
	char pet_char;
	const char* name;
	const char* pet_name;
	struct list_head list;
}Owner;

//										FUNCTION DECLARATIONS
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/
int ElevatorSchedule(void);
int ServiceFloor(void);
int AddPas2Elv(ElevatorState);
int Add2ElvRmvFrmList(Owner*, u_int8_t);
int RmvPasFrmElv(void);
int MoveElevator(void);
int print_passenger(void);
int my_run(void*data);

//										ELEVATOR SCHEDULER
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/
int ElevatorSchedule(void)
{
	u_int8_t idx;
	int pas_addQ = 0;
	ElevatorState IS_OFFLINEQ = IDLE;

	printk(KERN_NOTICE"ElevatorSchedule");

	//no one waiting to be serviced & no passengers.
	if((elevator.num_req == 0) && (elevator.num_pas == 0))
	{
		if(elevator.State != IDLE)
		{
			elevator.State = IDLE;
			elevator.pet_type = PASSENGER_NOPET;
			//when no passengers and no request go to first floor.
			while(elevator.cur_flr > FLOOR_BOTTOM)
			{
				MoveElevator();
			}
		}
	}
	
	//elevator was stopped but has now has requests.
	if((elevator.State == IDLE) && (elevator.num_req > 0))
	{
		//assume when the elevator is IDLE it is on floor 1;
		elevator.State = UP;
		
		//find the highest floor with requests.
		for(idx = FLOOR_TOP; idx > 1; idx--)
		{
			if(floor_list[idx].total_cnt > 0)
			{
				printk(KERN_NOTICE"0updating highest floor from %d to %d", 
					elevator.highest_flr, idx);
				elevator.highest_flr = idx;
				break;
			}
		}
	}
	
	//set IS_OFFLINEQ to offline here if OFFLINE WAS CALLED.
	
	if(elevator.State != IDLE)
	{
		printk(KERN_NOTICE"state != IDLE");
		
		if(elevator.State == UP)
		{
			printk(KERN_NOTICE"state = UP");
			//drop off and add guest until highest floor is reached.
			while(elevator.cur_flr < elevator.highest_flr)
			{
				print_passenger();
				printk(KERN_NOTICE"CF: %d, NR: %d, NP: %d, HF: %d, LF: %d, W: %d\n", 
					elevator.cur_flr, elevator.num_req, elevator.num_pas, 
					elevator.highest_flr, elevator.lowest_flr, elevator.weight);
				
				ServiceFloor();
				MoveElevator();
			}
			if(elevator.cur_flr == elevator.highest_flr)
			{
				print_passenger();
				pas_addQ = ServiceFloor();
				if(pas_addQ > 0)
				{
					//do not update state.
				}
				else
				{
					elevator.State = DOWN;
					ServiceFloor();
					//do not update top & bottom floor with requests when STATE == OFFLINE.
					if(IS_OFFLINEQ != OFFLINE)
					{
						for(idx = 1; idx < elevator.cur_flr; idx++)
						{
							if(floor_list[idx].total_cnt > 0)
							{
								printk(KERN_NOTICE"0updating lowest floor from %d to %d", 
									elevator.lowest_flr, idx);
								
								elevator.lowest_flr = idx;
								break;
							}
						}
					}
					else
					{
						
					}
				}
			}
		}
		else if(elevator.State == DOWN)
		{
			printk(KERN_NOTICE"state = DOWN");
			//drop off and add guests until lowest floor is reached.
			while(elevator.cur_flr > elevator.lowest_flr)
			{
				print_passenger();
				printk(KERN_NOTICE"CF: %d, NR: %d, NP: %d, HF: %d, LF: %d, W: %d\n", 
					elevator.cur_flr, elevator.num_req, elevator.num_pas, 
					elevator.highest_flr, elevator.lowest_flr, elevator.weight);
				
				ServiceFloor();
				MoveElevator();
			}
			if(elevator.cur_flr == elevator.lowest_flr)
			{
				print_passenger();
				pas_addQ = ServiceFloor();
				
				//keep going up.
				if(pas_addQ > 0)
				{
					//do not update state.
				}
				else	//switch state to down and service again.
				{
					elevator.State = UP;
					ServiceFloor();
					//do not update top & bottom floor with requests when STATE == OFFLINE.
					if(IS_OFFLINEQ != OFFLINE)
					{
						for(idx = FLOOR_TOP; idx > elevator.cur_flr; idx--)
						{
							if(floor_list[idx].total_cnt > 0)
							{
								printk(KERN_NOTICE"1updating highest floor from %d to %d", 
									elevator.highest_flr, idx);
							
								elevator.highest_flr = idx;
								break;
							}
						}
					}
					else
					{
						
					}
				}		
			}		
		}
		else if(elevator.State == OFFLINE)
		{
			
		}
	}
	return 1;
}

//										MOVE ELEVATOR
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/
int MoveElevator(void)
{
	printk(KERN_NOTICE"entered move elevator");
	if(elevator.State == UP)
	{
		if(elevator.cur_flr < FLOOR_TOP)
		{
			elevator.cur_flr += 1;			
		}
	}
	else if((elevator.State == DOWN)  || (elevator.State == IDLE))
	{
		if(elevator.cur_flr > FLOOR_BOTTOM)
		{
			elevator.cur_flr -= 1;			
		}
	}
	return 1;
}

//										SERVICE CURRENT FLOOR
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/
int ServiceFloor(void)
{
	u_int8_t ldOrDrop = 0x00;
	int returncode = 0;
	ElevatorState save_state = elevator.State;
	const uint16_t mask = (0x0001 << elevator.cur_flr);

	printk(KERN_NOTICE"entering ServiceFloor");

	//passengers need to be picked up.
	if(floor_list[elevator.cur_flr].total_cnt > 0) 
	{
		ldOrDrop |= 0x01;
	}

	//passengers need to be dropped off.
	if((elevator.stop_flr & mask) > 0)
	{
		ldOrDrop |= 0x02;
	}
	printk(KERN_NOTICE"stop_flr = %04X", elevator.stop_flr);

	switch(ldOrDrop)
	{
		case 0x03:	//Drop off then load.
		{
			elevator.State = LOADING;
			printk(KERN_NOTICE"dropping off & picking up.");
			RmvPasFrmElv();
			returncode = AddPas2Elv(save_state);
			elevator.State = save_state;
		}
		case 0x02:	//just drop off passengers.
		{
			elevator.State = LOADING;
			printk(KERN_NOTICE"just dropping off passengers.");
			RmvPasFrmElv();
			elevator.State = save_state;
		}
		case 0x01:	//just load passengers.
		{
			elevator.State = LOADING;
			printk(KERN_NOTICE"just picking up passengers.");
			returncode = AddPas2Elv(save_state);
			elevator.State = save_state;
		}
		case 0x00:	//no passengers associated with cur_flr.
		{
			
		}
		default:
		{
		
		}
		
	}
	return returncode;
}

//										ADD PASSENGERS TO ELEVATOR
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/
int AddPas2Elv(ElevatorState save_state)
{
	u_int8_t cur_flr = elevator.cur_flr;
	int returncode = 0;
	struct list_head *tmp;
	//struct list_head *Ftmp; 
	struct list_head *dummy;
	Owner *o;
	
	//must add until the next person is seen.
	//must be all dogs or all cats.
	
	printk(KERN_NOTICE"entered AddPas2Elv");
	
	//might be better to use list_entry in a while loop.
	list_for_each_safe(tmp, dummy, &floor_list[cur_flr].list)
	{
		o = list_entry(tmp, Owner, list);
		
		printk(KERN_NOTICE"servicing passenger on floor %d", cur_flr);
		
		//pet type requirement met.
		if((elevator.pet_type == o->pet_type) 
				|| (elevator.pet_type == PASSENGER_NOPET))
		{		
			printk(KERN_NOTICE"pet requirement met");
			//weight requirement met.
			if((elevator.weight + o->weight) <= MAX_WEIGHT)
			{
				printk(KERN_NOTICE"weight requirement met");
				if(save_state == UP)
				{
					printk(KERN_NOTICE"UP, dest_floor: %d, cur_flr: %d", o->dest_floor, cur_flr);
					//pick up passengers going up.
					if(o->dest_floor > cur_flr)
					{
						printk(KERN_NOTICE"pick up passengers going up");
						floor_list[cur_flr].req_up -= 1;
						list_del(tmp);
						Add2ElvRmvFrmList(o, cur_flr);
						returncode = 1;
					}
					else	//passenger not going in the current direction.
					{
						printk(KERN_NOTICE"passenger not going in current direction");
						break;
					}
				}
				else if(save_state == DOWN)
				{
					printk(KERN_NOTICE"DOWN, dest_floor: %d, cur_flr: %d", o->dest_floor, cur_flr);
					//pick up passengers going down.
					if(o->dest_floor < cur_flr)
					{
						printk(KERN_NOTICE"pick up passengers going down");
						floor_list[cur_flr].req_dwn -= 1;
						list_del(tmp);
						Add2ElvRmvFrmList(o, cur_flr);
						returncode = 1;
					}
					else	//passenger not going in the current direction.
					{
						printk(KERN_NOTICE"passenger not going in current direction");
						break;
					}
				}
				else
				{
					printk(KERN_NOTICE"state not up or down");
				}
				
			}
			else	//weight requirement not met.
			{
				//can possibly add another passenger from the floor with less pets.
				//if(o->num_pets > 1)
				
				printk(KERN_NOTICE"breaking from add, weight requirement not met");
				break;	//from list_for_each.
			}
		}
		else	//pet_type not correct, go to next passenger and check pet_type 
		{
			printk("pet type not correct");
			break;
		}
		
	}
	
	return returncode;
}

//										ADD TO ELEVATOR REMOVE FROM FLOOR_LIST
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/

int Add2ElvRmvFrmList(Owner *o, u_int8_t cur_flr)
{
	const uint16_t mask = (0x0001 << o->dest_floor);
	
	printk(KERN_NOTICE"enterd Add2ElvRmvFrmList");
	
	//safe to add Owner and pets to the elevator.
	//insert at back of list.
	
	list_add_tail(&o->list, &elevator.list);
	
	//update elevator stats.
	if(elevator.pet_type != o->pet_type){elevator.pet_type = o->pet_type;}
	elevator.num_pas += (o->num_pets + 1);
	
	printk(KERN_NOTICE"adding passenger with weight %d", o->weight);
	
	elevator.weight += o->weight;
	
	//no longer a request. passenger been serviced.
	elevator.num_req -= 1;
	
	//update floor stats.
	floor_list[cur_flr].total_cnt -= (o->num_pets + 1);
	floor_list[cur_flr].total_weight -= o->weight;
	
	//dont update.
	if((elevator.stop_flr & mask) > 0)
	{
		printk(KERN_NOTICE"add2elvrmv not up-dating stop_flr  %04X", elevator.stop_flr);
	}
	else	//update stop_flr to say someone needs to stop on o->dest_floor.
	{
		elevator.stop_flr |= mask;
		printk(KERN_NOTICE"add2elvrmv stop_flr to %04X, dest_floor = %d", elevator.stop_flr, o->dest_floor);
	}
	
	//update minimum lowest floor visited.
	if(o->dest_floor < elevator.lowest_flr)
	{
		printk(KERN_NOTICE"0updating lowest floor from %d to %d", elevator.lowest_flr, o->dest_floor);
		elevator.lowest_flr = o->dest_floor;
	}
	//update minimum highest floor visited.
	else if(o->dest_floor > elevator.highest_flr)
	{
		printk(KERN_NOTICE"2updating highest floor from %d to %d", elevator.highest_flr, o->dest_floor);
		elevator.highest_flr = o->dest_floor;
	}
	else	//floor already in current range.
	{
		printk(KERN_NOTICE"floor already in current range");
	}
	return 1;
}

//										REMOVE PASSENGERS FROM ELEVATOR
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/
int RmvPasFrmElv(void)
{
	u_int8_t cur_flr = elevator.cur_flr, petseenQ = 0x00;
	const uint16_t mask = ~(0x0001 << cur_flr);
	struct list_head *tmp;
	struct list_head *dummy;
	Owner* o;
	
	printk(KERN_NOTICE"enterd remove passenger from elevator. CF: %d", cur_flr);
	
	list_for_each_safe(tmp, dummy, &elevator.list)
	{
		o = list_entry(tmp, Owner, list);
		
		if(o->pet_type != PASSENGER_NOPET){petseenQ = 0x01;}
		
		if(o->dest_floor == cur_flr)
		{
			elevator.num_pas -= (o->num_pets + 1);
			elevator.weight -= o->weight;
			elevator.num_served += 1;
			list_del(tmp);
			kfree(o);
		}
	}
	
	//do nothing. pet type cannot change.
	if(petseenQ == 0x01)
	{
		
	}
	else	//update elevator pet type.
	{
		elevator.pet_type = PASSENGER_NOPET;
	}
	
	elevator.stop_flr &= mask;
	return 1;
}

//										ADD PASSENGERS TO FLOOR LIST
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/

//add passenger to start floor.
int AddPas2List(int num_pets, int pet_type, int start_floor, int dest_floor)
{
	//int length;
	int weight = 0;
	char *name;
	char c_type;
	//const uint16_t mask = (0x0001 << dest_floor);
	Owner *o;
	
	if((start_floor == dest_floor) || (dest_floor > 10) || (start_floor < 1))
	{
		printk(KERN_NOTICE"passenger not added to list. SF == DF, or out of range");
		return 1;
	}
	
	if(num_pets == 0){pet_type = PASSENGER_NOPET;}
	if(pet_type == PASSENGER_NOPET){num_pets = 0;}
	
	switch (pet_type)
	{
		case PASSENGER_CAT:
		{	
			c_type = 'o';
			weight = 1;
			name = "cat";
			break;
		}
		case PASSENGER_DOG:
		{	
			c_type = 'x';
			weight = 2;
			name = "dog";
			break;
		}
		case PASSENGER_NOPET:
		{
			c_type = ' ';
			weight = 0;
			name = "n/a";
		}
		default:
		{return -1;}
	}

	o = kmalloc(sizeof(Owner) * 1, __GFP_RECLAIM);
		
	if(o == NULL){return -ENOMEM;} 
	
	o->weight = ((weight * num_pets) + 3);
	o->owner_char = '|';
	o->pet_char = c_type;
	o->num_pets = num_pets;
	o->pet_type = pet_type;
	o->pet_name = name;
	o->name = "person";
	o->start_floor = start_floor;
	o->dest_floor = dest_floor;
	
	//insert at back of list.
	list_add_tail(&o->list, &floor_list[start_floor].list); 
	floor_list[start_floor].total_cnt += (num_pets + 1);
	floor_list[start_floor].total_weight += o->weight;
	elevator.num_req += 1;
	
	if(o->start_floor < o->dest_floor)
	{
		floor_list[start_floor].req_up += 1;
	}
	else
	{
		floor_list[start_floor].req_dwn += 1;
	}

	return 0;
}

//										PRINT PASSENGERS
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/

int print_passenger(void)
{
	int i;
	int i_FlrIdx, petidx;
	char state[10], isOnFlr;
	Owner *o;
	struct list_head* temp;
	struct list_head* dummy;

	

	char *buf = kmalloc(sizeof(char) * 300, __GFP_RECLAIM);
	char bufTmp[300];
	char msgTmp[300];
	if (buf == NULL)
	{
		printk(KERN_WARNING "print_passengers");
		return -ENOMEM;
	}


	/*
	//print elevator entries.
	i = 0;
	list_for_each(temp, &elevator.list)
	{
		o = list_entry(temp, Owner, list);
		//newline after every 5 entries.
		if (i % 5 == 0 && i > 0)
			strcat(message, "\n");
		sprintf(buf, "%s ", o->name);
		strcat(message, buf);
		i++;
	}
	*/

	/*
	
	strcpy(message, "");
	
	//OFFLINE, IDLE, LOADING, UP, DOWN
	
	if(elevator.State == OFFLINE)
	{
		strcpy(state, "OFFLINE");
	}
	else if(elevator.State == IDLE)
	{
		strcpy(state, "IDLE");
	}
	else if(elevator.State == LOADING)
	{
		strcpy(state, "LOADING");
	}
	else if(elevator.State == UP)
	{
		strcpy(state, "UP");
	}
	else if(elevator.State == DOWN)
	{
		strcpy(state, "DOWN");
	}
	else
	{
		
	}
	
	
	sprintf(buf, 
		"Elevator state: %s\n"
		"Elevator animals: %d\n"
		"Current floor: %d\n"
		"Number of passengers: %d\n"
		"Current weight: %d\n"
		"Number of passengers waiting: %d\n"
		"Number of passengers serviced: %d\n"
		, state
		, elevator.pet_type
		, elevator.cur_flr
		, elevator.num_pas
		, elevator.weight
		, elevator.num_req
		, elevator.num_served
		);
		
	strcat(message, buf);
	*/

	/*
	//print passengers waiting on each floor.
	for(i_FlrIdx = 1; i_FlrIdx < 11; i_FlrIdx++)
	{
		//print entries
		i = 0;
		//list_for_each_prev(temp, &animals.list) { //backwards
		sprintf(buf, "[] floor %d: %d ", i_FlrIdx, floor_list[i_FlrIdx].total_cnt);
		strcat(message, buf);
		list_for_each(temp, &floor_list[i_FlrIdx].list)
		{ 	//forwards.
			o = list_entry(temp, Owner, list);
			//newline after every 5 entries.
			
			if (i % 5 == 0 && i > 0)
				strcat(message, "\n");
			
			sprintf(buf, "%s (%d) %s ", o->name, o->num_pets, o->pet_name);
			strcat(message, buf);
			i++;
		}
		//trailing newline to separate file from commands.
		strcat(message, "\n");
	}
	*/
	
	//print elevator entries.
	//i = 0;
	
	strcpy(msgTmp, "");
	
	printk(KERN_NOTICE"printing floor list:");
	for(i = 1; i < 11; i++)
	{
		if(elevator.cur_flr == i){isOnFlr = '*';}else{isOnFlr = ' ';}
		
		sprintf(bufTmp, "[%c] Floor %d: %d\t", isOnFlr, i, floor_list[i].total_cnt);
		strcpy(msgTmp, bufTmp);
		list_for_each_safe(temp, dummy, &floor_list[i].list)
		{
			o = list_entry(temp, Owner, list);
			
			sprintf(bufTmp, "%c ", o->owner_char);
			strcat(msgTmp, bufTmp);
			for(petidx = 0; petidx < o->num_pets; petidx++)
			{
				sprintf(bufTmp, "%c ", o->pet_char);
				strcat(msgTmp, bufTmp);
			}
		}
		printk(msgTmp);
	}
	
	/*
	printk(KERN_NOTICE"printing elevator list:");
	
	strcpy(msgTmp, "");
	
	i = 1;
	list_for_each_safe(temp, dummy, &elevator.list)
	{
		o = list_entry(temp, Owner, list);
		
		//if(elevator.cur_flr == i){isOnFlr = '*';}else{isOnFlr = ' ';}
		sprintf(bufTmp, "[%c] Floor %d --> %d:\t", isOnFlr, o->start_floor, o->dest_floor);
		strcat(msgTmp, bufTmp);
		sprintf(bufTmp, "%c ", o->owner_char);
		strcat(msgTmp, bufTmp);
		for(petidx = 0; petidx < o->num_pets; petidx++)
		{
			sprintf(bufTmp, "%c ", o->pet_char);
			strcat(msgTmp, bufTmp);
		}
		i++;
		strcat(msgTmp, "\n");
	}
	printk(msgTmp);
	*/
	
	
	//strcat(message, "\n");
	//kfree(buf);
	
	
	return 0;
}

//										DELETE PASSENGERS
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/

void delete_passenger(int type)
{
	//struct list_head move_listE;
	//struct list_head move_listF[11];
	struct list_head *temp;
	struct list_head *dummy;
	int i_FlrIdx;
	Owner *o;
	
	printk(KERN_NOTICE "passenger type %d had  entries\n", type);

	// free up memory allocation of passengers
	//list_for_each_prev_safe(temp, dummy, &move_list) { //backwards
	list_for_each_safe(temp, dummy, &elevator.list)
	{ o = list_entry(temp, Owner, list);
		list_del(temp);	// removes entry from list
		kfree(o);
	}
	
	for(i_FlrIdx = 1; i_FlrIdx <  11; i_FlrIdx++)
	{
		list_for_each_safe(temp, dummy, &floor_list[i_FlrIdx].list)
		{ o = list_entry(temp, Owner, list);
			list_del(temp);	//removes entry from list
			kfree(o);
		}
	}
}

//										PROC OPEN
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/

int elevator_proc_open(struct inode *sp_inode, struct file *sp_file)
{	
	read_p = 1;
	message = kmalloc(sizeof(char) * ENTRY_SIZE, __GFP_RECLAIM | __GFP_IO | __GFP_FS);
	if (message == NULL) {
		printk(KERN_WARNING "elevator_proc_open");
		return -ENOMEM;
	}
	
	printk(KERN_NOTICE"entered: elevator_proc_open");

	return 0;
}

//										PROC READ
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/

ssize_t elevator_proc_read(struct file *sp_file, char __user *buf, size_t size, loff_t *offset)
{
	int len = strlen(message);
	
	printk(KERN_NOTICE"entered: elevator_proc_read");
	
	read_p = !read_p;
	if (read_p)
		return 0;
		
	copy_to_user(buf, message, len);
	return len;
}

//										PROC RELEASE
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/

int elevator_proc_release(struct inode *sp_inode, struct file *sp_file)
{
	printk(KERN_NOTICE"entered: elevator_proc_release");
	kfree(message);
	return 0;
}



//										CALL INITIALIZE FUNCTION
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/

int my_run(void*data) {
	char* name = (char*)data;
	int i = 0;
	printk("entered: my_run");
	while (!kthread_should_stop()) 
	{
		while (i < 20)
		{
			ElevatorSchedule();
			//schedule();
			ssleep(1);
			i++;
		}
	}
	return 0;
}

//		System call to start elevator process
/*									*/





/*		System calls		*/
/*									*/

// START ELEVATOR	
extern int (*STUB_start_elevator)(void);
int my_start_elevator(void)
{
	int temp;

if (elevator.state==OFFLINE)
{
	elevator.num_pas = 0;
        elevator.weight = 0;
        elevator.pet_type = PASSENGER_NOPET;
        elevator.num_req = 0;
        elevator.num_served = 0;
        elevator.cur_flr = 1;
        elevator.highest_flr = 1;
        elevator.lowest_flr = 1;
        elevator.stop_flr = 0;
        elevator.State = IDLE;
        INIT_LIST_HEAD(&elevator.list);
	kthread = kthread_run(my_run, (void*)KTHREAD_STRING_1, KTHREAD_STRING_1);

        if (IS_ERR(kthread)!=0 )
        {
                printk("ERROR kthread_run!\n");
                temp = -1;
        }
	else
	{
		temp =0;
	}
}
else
{
	temp =1;
}
return temp;
}

// ISSUE REQUEST
extern int(*STUB_issue_request)(int,int,int,int);
int my_issue_request(int numpets, int pettype,int start, int destination)
{
	AddPas2List(numpets,pettype,start,destination);
}



// STOP ELEVATOR	
extern int (*STUB_stop_elevator)(void);
int my_stop_elevator(void)
{
	struct list_head * temp = NULL;
	struct list_head * dummy = NULL;
	struct Owner * owner = NULL;

	list_for_each_safe(temp, dummy, &elevator.list[elevator.curFloor - 1])
	{
		owner = list_entry(temp, Owner, list);

		elevator.size -= 1;
                elevator.num_pas -= owner->num_pas;
                elevator.weight -= owner->weight;

		list_del(&owner->list);
		kfree(owner);
		
		return 1;
	}

	return 0;
}








//			INITIALIZE DEFINITION
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/

static int elevator_init(void)
{
	STUB_start_elevator=my_start_elevator;
	STUB_issue_request=my_issue_request;
	STUB_stop_elevator=my_stop_elevator;


	elevator.num_pas = 0;
        elevator.weight = 0;
        elevator.pet_type = PASSENGER_NOPET;
        elevator.num_req = 0;
        elevator.num_served = 0;
        elevator.cur_flr = 1;
        elevator.highest_flr = 1;
        elevator.lowest_flr = 1;
        elevator.stop_flr = 0;
        elevator.State = IDLE;
        INIT_LIST_HEAD(&elevator.list);


	int i_FlrIdx;

	int num_pets[10] = { 1, 2, 1, 3, 1, 3, 2, 1, 3, 3 };
	int pet_type[10] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	int start_floor[10] = { 3, 6, 2, 5, 8, 9, 10, 4, 1, 2 };
	int dest_floor[10] = { 4, 7, 1, 4, 3, 6, 2, 1, 10, 8 };
	int AddIndx;

	printk(KERN_NOTICE"entered: elevator_init");
	
	fops.open = elevator_proc_open;
	fops.read = elevator_proc_read;
	fops.release = elevator_proc_release;
	
	if (!proc_create(ENTRY_NAME, PERMS, NULL, &fops))
	{
		printk(KERN_WARNING "elevator_init\n");
		remove_proc_entry(ENTRY_NAME, NULL);
		return -ENOMEM;
	}
	
	//may need to move.
	for(i_FlrIdx = 0; i_FlrIdx < NUM_FLOORS; i_FlrIdx++)
	{
		INIT_LIST_HEAD(&floor_list[i_FlrIdx].list);
		floor_list[i_FlrIdx].total_cnt = 0;
		floor_list[i_FlrIdx].total_weight = 0;
		floor_list[i_FlrIdx].req_up = 0;
		floor_list[i_FlrIdx].req_dwn = 0;
	}

	for (AddIndx = 0; AddIndx < 10; AddIndx++)
	{
		AddPas2List(num_pets[AddIndx], pet_type[AddIndx], start_floor[AddIndx], dest_floor[AddIndx]);
	}
	
	//print_passenger();

	kthread = kthread_run(my_run, (void*)KTHREAD_STRING_1, KTHREAD_STRING_1);

	if (IS_ERR(kthread))
	{
		printk("ERROR kthread_run!\n");
		return(PTR_ERR(kthread));
	}

	return 0;
}


module_init(elevator_init);
//				EXIT DEFINITION
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/

static void elevator_exit(void)
{
	STUB_start_elevator=NULL;
	STUB_issue_request=NULL;
	STUB_stop_elevator=NULL;

	int ret = kthread_stop(kthread);
	printk(KERN_NOTICE"entered: elevator_exit");
	
	//delete_passenger(PASSENGER_PERSON);
	//delete_passenger(PASSENGER_CAT); 
	//delete_passenger(PASSENGER_DOG);

	if (ret != -EINTR)
	{
		printk("kthread has stop\n");
	}
	
	remove_proc_entry(ENTRY_NAME, NULL);
}

//										CALL EXIT FUNTION
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/

module_exit(elevator_exit);

/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/
