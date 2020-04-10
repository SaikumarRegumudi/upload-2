# upload-2
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#define Default_num 5
sem_t sem_students;//global semaphore for student
sem_t sem_ta;//global semaphore for teaching assistant
pthread_mutex_t Mutex;//mutex
int Waiting_chair[3];//number of seats available in halll way
int Num_Stu_Wait = 0;//counter for student waiting
int Next_Stu = 0;//counter for next student
int Next_Teach_Action = 0;//teaching assistant state
int TeachingAssitant_Flag = 0;//flag for teaching assitant state

int isNum(char number[])//function to check wether the argrument is in integer format or not
{
    int i;
		for ( i = 0 ; number[i] != 0; i++ )
    {
        if (!isdigit(number[i]))//inbuilt funtion to check its anumber
            return 0;
    }
    return 1;
}

int iswait( int student_id ) 
{	int i;
	for ( i = 0; i < 3; i++ )// checking chair(3) out in the hall
	 {
		if ( Waiting_chair[i] == student_id ) // check wether the specifed student  is still waiting or not
		{ return 1; }
	}
	return 0;// return zero if specified student is not waiting
}

void* student_actions( void* student_id ) //function to define action of student accepting wild pointer as argument
{	int ID = *(int*)student_id;// type conversion of wild pointer to integer type
	while( 1 )//infinte loop for checking conditon of student constantl
	 {//if student is waiting, continue waiting
		if ( iswait( ID ) == 1 ) { continue; }// jump statemen
		//student is programming.
		int time = rand() % 5;//random time taken by student to finish program
		printf( "\tStudent %d is programming for %d seconds.\n", ID, time );
		sleep( time );
		pthread_mutex_lock( &Mutex );//entering in crtical section for student
		if( Num_Stu_Wait < 3 ) 
		{	Waiting_chair[Next_Stu] = ID;//if any other student comes and find vacant seat
			Num_Stu_Wait++;// student sits
			//student takes a seat in the hallway.
			printf( "\t\tStudent %d takes a seat. Students waiting = %d.\n", ID, Num_Stu_Wait );
			Next_Stu = ( Next_Stu + 1 ) % 3;//student next in line
        	pthread_mutex_unlock( &Mutex );//eixting crtical section
			//wake TA if sleeping
			sem_post( &sem_students );//atomic actions
			sem_wait( &sem_ta );	}
		else {
			pthread_mutex_unlock( &Mutex );
			//No chairs available. Student will try later.
			printf( "\t\t\tStudent %d will try later.\n",ID );
		}
	}
}
void* ta_actions() //teaching assitant state calculation
{
    printf( "Checking for students.\n" );
	while( 1 ) // infinte loop for constantly checking
	{	//if students are waiting
		if ( Num_Stu_Wait > 0 ) {
			TeachingAssitant_Flag = 0;
			sem_wait( &sem_students );//atomic action on semaphore
			pthread_mutex_lock( &Mutex );//crtical section lock
			int help_time = rand() % 5;//random time that can be taken by student
			//TA helping student.
			printf( "Helping a student for %d seconds. Students waiting = %d.\n", help_time, (Num_Stu_Wait - 1) );//state of got help and left
			printf( "Student %d receiving help.\n",Waiting_chair[Next_Teach_Action] );//student next in line
			Waiting_chair[Next_Teach_Action]=0;
			Num_Stu_Wait--;//student deducted from line
			Next_Teach_Action = ( Next_Teach_Action + 1 ) % 3;//teacing asssistant shift to next student postion
			sleep( help_time );//atomic action on semaphore
			pthread_mutex_unlock( &Mutex );// out of crtical section
			sem_post( &sem_ta );

		}
		//if no students are waiting
		else {//teaching assistant sleeping there is no student in que for help
			if ( TeachingAssitant_Flag == 0 ) {
				printf( "No students waiting. Sleeping.\n" );
				TeachingAssitant_Flag = 1;
			}
		}
	}
}
int main( int argc, char **argv )//predefined Argument argc to cound number of arguments passed with name of program itself
//argv an character pointer listing all arguments
{
	int i;
	int no_Stud;// number of students
	if (argc > 1 ) //to check wether any argument is passed because one argument is name itself
	{
		if ( isNum( argv[1] ) == 1) //checking wether passed argument is number or not
		{
			no_Stud = atoi( argv[1] );// changing charcter type to integer type by atoi inbuilt function
		}
		else {
			printf("Invalid input. Quitting program.");//failsafe condition
			return 0;
		}
	}
	else {
		no_Stud = Default_num;//failsafe condition if no argument is passed
	}
	int student_ids[no_Stud];//intializing student ids
	pthread_t students[no_Stud];//intializing thread array
	pthread_t ta;//intializing thread for flag of teaching assisstant
	sem_init( &sem_students, 0, 0 );//semaphore intialization
	sem_init( &sem_ta, 0, 1 );
	//Create threads.
	pthread_mutex_init( &Mutex, NULL );//taking no exceess argument
	pthread_create( &ta, NULL, ta_actions, NULL );//taking no excess argument
	for( i = 0; i < no_Stud; i++ )//creating different number of threads for each student
	{
		student_ids[i] = i + 1;
		pthread_create( &students[i], NULL, student_actions, (void*) &student_ids[i] );
	}
	//Join threads
	pthread_join(ta, NULL);
	for( i =0; i < no_Stud; i++ )
	{
		pthread_join( students[i],NULL );//combinig all light weight threads
	}
	return 0;
}
