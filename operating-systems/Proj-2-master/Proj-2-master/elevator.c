/*

\\\\\\\\\\\\                                                              >
==========================================================================>>>>
////////////                                                              >

STUDENTS    : Zachary Berrhill, Chris Lopes, James Roquebert.
CLASS       : COP 4610 Intro To Operating Systems.
ASSIGNMENT  : Project 2, Part III
DATE        : 03/04/2020

*/


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

//																		GLOBALS
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

static struct mutex floor_list_lock, elevatorDataLock, globalDataLock;
static struct task_struct* kthread;

ElevatorState IS_OFFLINEQ = IDLE;

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

//															FUNCTION DECLARATIONS
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/
int ElevatorSchedule(void);
int ServiceFloor(void);
int AddPas2Elv(ElevatorState);
int Add2ElvRmvFrmList(Owner*, u_int8_t);
int RmvPasFrmElv(void);
int MoveElevator(void);
int print_passenger(void);
int my_run(void*data);
void delete_floor_list(void);

//															ELEVATOR SCHEDULER
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/
int ElevatorSchedule(void)
{
	u_int8_t idx, tmpStopFlrP = 0, tmpStopFlrL = 0;
	int pas_addQ = 0;
	uint16_t mask = 0x0001;

	//no need for servicing.
	if (elevator.State == OFFLINE)
	{

	}

	//no one waiting to be serviced & no passengers.
	else if((elevator.num_req == 0) && (elevator.num_pas == 0) && (elevator.State != IDLE))
	{
		mutex_lock_interruptible(&elevatorDataLock);
			elevator.State = IDLE;
			elevator.pet_type = PASSENGER_NOPET;
		mutex_unlock(&elevatorDataLock);
		
		//when no passengers and no request go to first floor.
		while(elevator.cur_flr > FLOOR_BOTTOM)
		{
			MoveElevator();
		}
		elevator.lowest_flr = 1;
		elevator.highest_flr = 1;
	}
	
	//elevator was stopped but has now has requests.
	else if((elevator.State == IDLE) && (elevator.num_req > 0))
	{
		//assume when the elevator is IDLE it is on floor 1;
		mutex_lock_interruptible(&elevatorDataLock);
			elevator.State = UP;
		mutex_unlock(&elevatorDataLock);
		
		//find the highest floor with requests.
		for(idx = FLOOR_TOP; idx > 1; idx--)
		{
			if(floor_list[idx].total_cnt > 0)
			{
				elevator.highest_flr = idx;
				break;
			}
		}
	}

	//all passengers have been off-loaded & elevator should be set to OFFLINE.
	else if ((IS_OFFLINEQ == OFFLINE) && (elevator.num_pas == 0))
	{
		mutex_lock_interruptible(&elevatorDataLock);
			elevator.State = IDLE;
		mutex_unlock(&elevatorDataLock);


		mutex_lock_interruptible(&globalDataLock);
			IS_OFFLINEQ = IDLE;
		mutex_unlock(&globalDataLock);

		while (elevator.cur_flr > FLOOR_BOTTOM)
		{
			MoveElevator();
		}

		mutex_lock_interruptible(&elevatorDataLock);
			elevator.State = OFFLINE;
		mutex_unlock(&elevatorDataLock);
	}
	
	else if((elevator.State == UP) || (elevator.State == DOWN))
	{
		
		if(elevator.State == UP)
		{
			//drop off and add guest until highest floor is reached.
			if(elevator.cur_flr < elevator.highest_flr)
			{
				ServiceFloor();
				MoveElevator();
			}
			else if(elevator.cur_flr == elevator.highest_flr)
			{
				pas_addQ = ServiceFloor();
				if(pas_addQ > 0)
				{
					//do not update state.
				}
				else
				{
					mutex_lock_interruptible(&elevatorDataLock);
					elevator.State = DOWN;
					mutex_unlock(&elevatorDataLock);

					ServiceFloor();
					//do not update top & bottom floor with requests when STATE == OFFLINE.
					if(IS_OFFLINEQ != OFFLINE)
					{
						for(idx = 1; idx < elevator.cur_flr; idx++)
						{
							//idx is a stop floor for passengers.
							if ((elevator.stop_flr & (mask << idx)) > 0x0000)
							{
								//save the first floor seen from the bottom.
								tmpStopFlrP = idx;
								break;
							}
							//if((floor_list[idx].total_cnt > 0) && (idx < elevator.lowest_flr))
							if (floor_list[idx].total_cnt > 0)
							{
								tmpStopFlrL = idx;
								break;
							}
						}
						//save passenger lowest stop floor because it's the lowest.
						if (tmpStopFlrP >= tmpStopFlrL)
						{
							elevator.lowest_flr = tmpStopFlrP;
							tmpStopFlrP = 0;
						}
						else if(tmpStopFlrP < tmpStopFlrL)//save list stop floor because it's the lowest.
						{
							elevator.lowest_flr = tmpStopFlrL;
							tmpStopFlrL = 0;
						}
						else
						{
							elevator.lowest_flr = 1;
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
			//drop off and add guests until lowest floor is reached.
			if(elevator.cur_flr > elevator.lowest_flr)
			{	
				ServiceFloor();
				MoveElevator();
			}
			if(elevator.cur_flr == elevator.lowest_flr)
			{
				pas_addQ = ServiceFloor();
				
				//keep going up.
				if(pas_addQ > 0)
				{
					//do not update state.
				}
				else	//switch state to down and service again.
				{
					mutex_lock_interruptible(&elevatorDataLock);
						elevator.State = UP;
					mutex_unlock(&elevatorDataLock);

					ServiceFloor();
					//do not update top & bottom floor with requests when STATE == OFFLINE.
					if(IS_OFFLINEQ != OFFLINE)
					{
						for(idx = FLOOR_TOP; idx > elevator.cur_flr; idx--)
						{
							//idx is a stop floor for passengers.
							if ((elevator.stop_flr & (mask << idx)) > 0x0000)
							{
								//save the first floor seen from the bottom.
								tmpStopFlrP = idx;
								break;
							}
							//if((floor_list[idx].total_cnt > 0) && (idx < elevator.lowest_flr))
							if (floor_list[idx].total_cnt > 0)
							{
								tmpStopFlrL = idx;
								break;
							}
						}
						//save passenger lowest stop floor because it's the lowest.
						if (tmpStopFlrP >= tmpStopFlrL)
						{
							elevator.highest_flr = tmpStopFlrP;
							tmpStopFlrP = 0;
						}
						else if(tmpStopFlrP < tmpStopFlrL)//save list stop floor because it's the lowest.
						{
							elevator.highest_flr = tmpStopFlrL;
							tmpStopFlrL = 0;
						}
						else
						{
							elevator.highest_flr = 1;
						}
					}
					else
					{
						
					}
				}		
			}		
		}
	}
	return 1;
}

//															MOVE ELEVATOR
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/
int MoveElevator(void)
{
	if(elevator.State == UP)
	{
		if(elevator.cur_flr < FLOOR_TOP)
		{
			mutex_lock_interruptible(&elevatorDataLock);
				elevator.cur_flr += 1;
			mutex_unlock(&elevatorDataLock);
			
			ssleep(2);
		}
	}
	else if((elevator.State == DOWN)  || (elevator.State == IDLE))
	{
		if(elevator.cur_flr > FLOOR_BOTTOM)
		{
			mutex_lock_interruptible(&elevatorDataLock);
				elevator.cur_flr -= 1;
			mutex_unlock(&elevatorDataLock);
			
			ssleep(2);
		}
	}
	return 1;
}

//														SERVICE CURRENT FLOOR
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/
int ServiceFloor(void)
{
	u_int8_t ldOrDrop = 0x00;
	int returncode = 0;
	ElevatorState save_state = elevator.State;
	const uint16_t mask = (0x0001 << elevator.cur_flr);

	//passengers need to be picked up.
	if((floor_list[elevator.cur_flr].total_cnt > 0) && (IS_OFFLINEQ != OFFLINE)) 
	{
		ldOrDrop |= 0x01;
	}

	//passengers need to be dropped off.
	if((elevator.stop_flr & mask) > 0x0000)
	{
		ldOrDrop |= 0x02;
	}

	switch(ldOrDrop)
	{
		case 0x03:	//Drop off then load.
		{
			mutex_lock_interruptible(&elevatorDataLock);
				elevator.State = LOADING;
			mutex_unlock(&elevatorDataLock);

			RmvPasFrmElv();
			returncode = AddPas2Elv(save_state);
			
			mutex_lock_interruptible(&elevatorDataLock);
				elevator.State = save_state;
			mutex_unlock(&elevatorDataLock);

			ssleep(1);
		}
		case 0x02:	//just drop off passengers.
		{
			mutex_lock_interruptible(&elevatorDataLock);
				elevator.State = LOADING;
			mutex_unlock(&elevatorDataLock);
			
			RmvPasFrmElv();
			
			mutex_lock_interruptible(&elevatorDataLock);
				elevator.State = save_state;
			mutex_unlock(&elevatorDataLock);
			
			ssleep(1);
		}
		case 0x01:	//just load passengers.
		{
			mutex_lock_interruptible(&elevatorDataLock);
				elevator.State = LOADING;
			mutex_unlock(&elevatorDataLock);

			returncode = AddPas2Elv(save_state);
			
			mutex_lock_interruptible(&elevatorDataLock);
				elevator.State = save_state;
			mutex_unlock(&elevatorDataLock);
			
			ssleep(1);
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

//													ADD PASSENGERS TO ELEVATOR
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/
int AddPas2Elv(ElevatorState save_state)
{
	u_int8_t cur_flr = elevator.cur_flr;
	int returncode = 0;
	struct list_head *tmp, *dummy;
	Owner *o;
	
	//must add until the next person is seen.
	//must be all dogs or all cats.
	
	mutex_lock_interruptible(&floor_list_lock);
		list_for_each_safe(tmp, dummy, &floor_list[cur_flr].list)
		{
			o = list_entry(tmp, Owner, list);
		
			//pet type requirement met.
			if((elevator.pet_type == o->pet_type) 
					|| (elevator.pet_type == PASSENGER_NOPET))
			{		
				//weight requirement met.
				if((elevator.weight + o->weight) <= MAX_WEIGHT)
				{
					if(save_state == UP)
					{
						//pick up passengers going up.
						if(o->dest_floor > cur_flr)
						{
							list_del(tmp);

							Add2ElvRmvFrmList(o, cur_flr);
							returncode = 1;
						}
						else	//passenger not going in the current direction.
						{
							break;
						}
					}
					else if(save_state == DOWN)
					{
						//pick up passengers going down.
						if(o->dest_floor < cur_flr)
						{	
							list_del(tmp);
						
							Add2ElvRmvFrmList(o, cur_flr);
							returncode = 1;
						}
						else	//passenger not going in the current direction.
						{
							break;
						}
					}
					else
					{

					}
				
				}
				else	//weight requirement not met.
				{	
					break;	//from list_for_each.
				}
			}
			else	//pet_type not correct, go to next passenger and check pet_type.
			{
				break;	//from list_for_each.
			}
		
		}
	mutex_unlock(&floor_list_lock);
	return returncode;
}

//											ADD TO ELEVATOR REMOVE FROM FLOOR_LIST
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/

int Add2ElvRmvFrmList(Owner *o, u_int8_t cur_flr)
{
	const uint16_t mask = (0x0001 << o->dest_floor);
	
	//safe to add Owner and pets to the elevator.
	//insert at back of list.
	list_add_tail(&o->list, &elevator.list);
	
	mutex_lock_interruptible(&elevatorDataLock);
		//update elevator stats.
		if(elevator.pet_type != o->pet_type)
		{	
			elevator.pet_type = o->pet_type;
		}
		elevator.num_pas += (o->num_pets + 1);
		elevator.weight += o->weight;
		//no longer a request. passenger been serviced.
		elevator.num_req -= 1;
	mutex_unlock(&elevatorDataLock);

		//update floor stats.
		floor_list[cur_flr].total_cnt -= (o->num_pets + 1);
		floor_list[cur_flr].total_weight -= o->weight;

	//dont update.
	if((elevator.stop_flr & mask) > 0x0000)
	{

	}
	else	//update stop_flr to say someone needs to stop on o->dest_floor.
	{
		elevator.stop_flr |= mask;
	}
	
	//update minimum lowest floor visited.
	if(o->dest_floor < elevator.lowest_flr)
	{
		elevator.lowest_flr = o->dest_floor;
	}
	//update minimum highest floor visited.
	else if(o->dest_floor > elevator.highest_flr)
	{
		elevator.highest_flr = o->dest_floor;
	}
	else	//floor already in current range.
	{

	}
	return 1;
}

//											REMOVE PASSENGERS FROM ELEVATOR
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/
int RmvPasFrmElv(void)
{
	u_int8_t cur_flr = elevator.cur_flr, petseenQ = 0x00;
	const uint16_t mask = ~(0x0001 << cur_flr);
	struct list_head *tmp;
	struct list_head *dummy;
	Owner* o;
	
	mutex_lock_interruptible(&floor_list_lock);
		list_for_each_safe(tmp, dummy, &elevator.list)
		{
			o = list_entry(tmp, Owner, list);
		
			if(o->pet_type != PASSENGER_NOPET){petseenQ = 0x01;}
		
			if(o->dest_floor == cur_flr)
			{
				mutex_lock_interruptible(&elevatorDataLock);
					elevator.num_pas -= (o->num_pets + 1);
					elevator.weight -= o->weight;
					elevator.num_served += 1;
				mutex_unlock(&elevatorDataLock);
			
				list_del(tmp);	//delete from elevator list.
				kfree(o);		//de-allocate passenger.
			}
		}
	mutex_unlock(&floor_list_lock);
	
	//do nothing. pet type cannot change.
	if((petseenQ == 0x00) || (elevator.num_pas == 0))
	{
		mutex_lock_interruptible(&elevatorDataLock);
			elevator.pet_type = PASSENGER_NOPET;
		mutex_unlock(&elevatorDataLock);
	}
	else	//update elevator pet type.
	{
		
	}
	
	elevator.stop_flr &= mask;
	return 1;
}

//												ADD PASSENGERS TO FLOOR LIST
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/

//add passenger to start floor.
int AddPas2List(int num_pets, int pet_type, int start_floor, int dest_floor)
{
	//int length;
	int weight = 0;
	char *name;
	char c_type;
	Owner *o;
	
	if((start_floor == dest_floor) || (dest_floor > 10) || (start_floor < 1))
	{
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
		{return 1;}
	}

	o = kmalloc(sizeof(Owner) * 1, __GFP_RECLAIM);
		
	if(o == NULL){return -ENOMEM;} 
	

	mutex_lock_interruptible(&floor_list_lock);

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
	
	mutex_unlock(&floor_list_lock);

	return 0;
}

//															PRINT PASSENGERS
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/

int print_passenger(void)
{
	int i, petidx, cur_flr;
	char state[10], isOnFlr;
	Owner *o;
	struct list_head* temp;
	struct list_head* dummy;

	//char *buf = kmalloc(sizeof(char) * 300, __GFP_RECLAIM);
	char bufTmp[300];
	char msgTmp[300];
	
	strcpy(message, "");
	
	//OFFLINE, IDLE, LOADING, UP, DOWN
	
	mutex_lock_interruptible(&elevatorDataLock);
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
	
		sprintf(bufTmp, 
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

		cur_flr = elevator.cur_flr;

	mutex_unlock(&elevatorDataLock);

	strcat(message, bufTmp);
	
	strcpy(msgTmp, "");
	
	for(i = 1; i < 11; i++)
	{
		if(cur_flr == i){isOnFlr = '*';}else{isOnFlr = ' ';}
		
		mutex_lock_interruptible(&floor_list_lock);
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
		mutex_unlock(&floor_list_lock);

		strcat(message, "\n");
		strcat(message, msgTmp);
	}	
	
	strcat(message, "\n");

	return 0;
}

//																DELETE PASSENGERS
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/

void delete_floor_list(void)
{
	struct list_head *temp;
	struct list_head *dummy;
	int i_FlrIdx;
	Owner *o;
	
	for(i_FlrIdx = 1; i_FlrIdx <  11; i_FlrIdx++)
	{
		mutex_lock_interruptible(&floor_list_lock);
			list_for_each_safe(temp, dummy, &floor_list[i_FlrIdx].list)
			{ 
				o = list_entry(temp, Owner, list);
				list_del(temp);		//removes entry from list.
				kfree(o);			//deallocates Owner.
			}
		mutex_unlock(&floor_list_lock);
	}
}

//																	PROC OPEN
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/

int elevator_proc_open(struct inode *sp_inode, struct file *sp_file)
{	
	read_p = 1;
	message = kmalloc(sizeof(char) * ENTRY_SIZE, __GFP_RECLAIM | __GFP_IO | __GFP_FS);
	if (message == NULL) {
		printk(KERN_WARNING "elevator_proc_open");
		return -ENOMEM;
	}

	print_passenger();

	return 0;
}

//																	PROC READ
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/

ssize_t elevator_proc_read(struct file *sp_file, char __user *buf, size_t size, loff_t *offset)
{
	int len = strlen(message);
	
	read_p = !read_p;
	if (read_p)
		return 0;
		
	copy_to_user(buf, message, len);
	return len;
}

//																	PROC RELEASE
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/

int elevator_proc_release(struct inode *sp_inode, struct file *sp_file)
{
	kfree(message);
	return 0;
}

//								START ELEVATOR
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/
extern int(*STUB_start_elevator)(void);
int my_start_elevator(void)
{
	int returncode;
	mutex_lock_interruptible(&elevatorDataLock);
		if (elevator.State == OFFLINE)
		{
			elevator.num_pas = 0;
			elevator.weight = 0;
			elevator.pet_type = PASSENGER_NOPET;
			elevator.cur_flr = 1;
			elevator.highest_flr = 1;
			elevator.lowest_flr = 1;
			elevator.stop_flr = 0;
			elevator.State = IDLE;
			returncode = 0;
		}
		else
		{
			returncode = 1;
		}
	mutex_unlock(&elevatorDataLock);
	return returncode;
}

//								ISSUE REQUEST
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/
extern int(*STUB_issue_request)(int, int, int, int);
int my_issue_request(int num_pets, int pet_type, int start_floor, int dest_floor)
{
	return AddPas2List(num_pets, pet_type, start_floor, dest_floor);
}

int my_run(void*data) {
	char* name = (char*)data;
	while (!kthread_should_stop()) 
	{
		ElevatorSchedule();
		if ((elevator.State == OFFLINE) || (elevator.State == IDLE))
		{
			ssleep(1);
		}
	}
	return 0;
}

//							STOP ELEVATOR
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/
extern int (*STUB_stop_elevator)(void);
int my_stop_elevator(void)
{
	int returncode;
	u_int8_t status = 0x00;
	
	mutex_lock_interruptible(&globalDataLock);
		if (IS_OFFLINEQ == OFFLINE)
		{
			status |= 0x01;
		}
	mutex_unlock(&globalDataLock);
	
	mutex_lock_interruptible(&elevatorDataLock);
		if(elevator.State == OFFLINE)
		{
			status = 0x02;
		}
	mutex_unlock(&elevatorDataLock);

	switch(status)
	{
		case 0x03:		//ISOFFLINEQ = OFFLINE & State = OFFLINE.
		{
			//should not happen.
		}
		case 0x02:		//State = OFFLINE & IS_OFFLINEQ != OFFLINE.
		{
			//elevator already offline.
			returncode = 1;
		}
		case 0x01:		//State != OFFLINE & ISOFFLINE_Q = OFFLINE.
		{
			//OFFLINE flag has been set but elevator is not offline yet.
			returncode = 1;

		}
		case 0x00:		//neither are OFFLINE.
		{
			mutex_lock_interruptible(&globalDataLock);
				IS_OFFLINEQ = OFFLINE;
			mutex_unlock(&globalDataLock);
			
			returncode = 0;
		}
	}
	return returncode;
}

//															INITIALIZE DEFINITION
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/

static int elevator_init(void)
{
	int i_FlrIdx;
	
	STUB_start_elevator = my_start_elevator;
	STUB_issue_request = my_issue_request;
	STUB_stop_elevator = my_stop_elevator;

	fops.open = elevator_proc_open;
	fops.read = elevator_proc_read;
	fops.release = elevator_proc_release;

	if (!proc_create(ENTRY_NAME, PERMS, NULL, &fops))
	{
		printk(KERN_WARNING "elevator_init\n");
		remove_proc_entry(ENTRY_NAME, NULL);
		return -ENOMEM;
	}

	mutex_init(&floor_list_lock);
	mutex_init(&elevatorDataLock);
	mutex_init(&globalDataLock);

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

	//may need to move.
	for (i_FlrIdx = 0; i_FlrIdx < NUM_FLOORS; i_FlrIdx++)
	{
		INIT_LIST_HEAD(&floor_list[i_FlrIdx].list);
		floor_list[i_FlrIdx].total_cnt = 0;
		floor_list[i_FlrIdx].total_weight = 0;
		floor_list[i_FlrIdx].req_up = 0;
		floor_list[i_FlrIdx].req_dwn = 0;
	}

	kthread = kthread_run(my_run, (void*)KTHREAD_STRING_1, KTHREAD_STRING_1);

	if (IS_ERR(kthread))
	{
		printk("ERROR kthread_run!\n");
		return(PTR_ERR(kthread));
	}

	return 0;
}

//													CALL INITIALIZE FUNCTION
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/

module_init(elevator_init);

//															EXIT DEFINITION
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/

static void elevator_exit(void)
{
	STUB_start_elevator = NULL;
	STUB_issue_request = NULL;
	STUB_stop_elevator = NULL;

	int ret = kthread_stop(kthread);
	
	delete_floor_list();	//remove Owners from their start floor.

	if (ret != -EINTR)
	{
		printk("kthread has stop\n");
	}

	mutex_destroy(&floor_list_lock);
	mutex_destroy(&elevatorDataLock);
	mutex_destroy(&globalDataLock);
	
	remove_proc_entry(ENTRY_NAME, NULL);
}

//														CALL EXIT FUNTION
/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/

module_exit(elevator_exit);

/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/
