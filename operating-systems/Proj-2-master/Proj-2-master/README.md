# Proj-2

download elevator.c from the main page for work & testing.




_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-

RULES (general)

add passenger & (0 - 3) pets. all pets must be able to fit.

dog cannot be on an elevator with a cat & vice versa.

one person cannot have a dog & a cat.

animal cannot board without person.

no persons destination floor can be their starting floor.

elevator must be going in the direction of perosns destination floor.

_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-

SPECIFICATIONS (general)

elevator MAX_LOAD = 15 units.

2.0 seconds between floors.

1.0 seconds stop on loading floor.

lowest floor = 1.

top floor = 10.

weight units:
	person = 3.
	dog = 2.
	cat = 1.
_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-

SPECIFICATIONS (start_elevator())

parameters:
	-none

elevator starts servicing request:
	-goes up
	-goes down
	-picks up passengers
	-drops off passengers
	
return values:
	-already active = 1.
	-sucessful start = 0.
	-could not allocate memory = -ENOMEM
	
initialization:
	-STATE = IDLE.
	-current floor = 1.
	-current load = 0.

_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-

SPECIFICATIONS(issue_request())

parameters:
	-int num_pets.
	-int pet_type. 1 = cat. 2 = dog.
	-int start_floor.
	-int destination_floor.
	
return values:
	-not valid = 1.
	-sucessful add = 0.
	
_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-

SPECIFICATIONS(stop_elevator())

must process no new request.

must offload all passengers.

once the above is done, set STATE = IDLE.

_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-

SPECIFICATIONS (proc)

name must be: /proc/elevator

print:
	-STATE.
	-type of animals on the elevator.
	-current floor.
	-current load (# passengers & animals).
	-total number of passengers waiting.
	-total number of passengers serviced.
	
print (2) for each floor of the building
	-elevator on floor?.
	-# waiting passengers & char indicating type.
		-person = '|'.
		-dog 'x'.
		-cat 'o'.

_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-

tail -f will print in real time.

echo > /dev/null | sudo tee /var/log/kern.log to delete kern log.

use ssleep() to wait between floors. and to wait at floor while loading 
	passengers.
	
break works on list_for_each

_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-

TO DO:
	-lock list when updating & when printing with proc file.
	-change ElevatorSchedule() UP / DOWN / OFFLINE logic.
		-can possibly be fixed by creating a state OFFLINE7UP & OFFLINE7DOWN.


_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-


what to lock & when to unlock.

in ElevatorSchedule() may need to change while < low/high floor to better
	accomodate locking by returning from scheduling after every floor service.

assign stub in module init.

stub set back to NULL after completion.

syscall moudule at the top of elevator.c












