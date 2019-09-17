/*
SUDIP GHALE
ID: 1001557881
LAB: 02
*/

#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

/*** Constants that define parameters of the simulation ***/

#define MAX_SEATS 3        /* Number of seats in the professor's office */
#define BREAK_LIMIT 10 /* Number of students the professor can help before he needs a break */
#define MAX_STUDENTS 1000  /* Maximum number of students in the simulation */

#define CLASSA 0 // 0 means from class A
#define CLASSB 1 // 1 means from class B

sem_t mutex; // semaphore varible
// rest are mutex and its conditional variables
pthread_mutex_t lock_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t class_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t class_a_cv = PTHREAD_COND_INITIALIZER;
pthread_cond_t class_b_cv = PTHREAD_COND_INITIALIZER;


static int students_in_office;   /* Total numbers of students currently in the office */
static int classa_inoffice;      /* Total numbers of students- from class A currently in the office */
static int classb_inoffice;      /* Total numbers of students from class B in the office */
static int students_since_break = 0; // Total number of students after professor takes a break
static int count_class_a=0; // count for class A students
static int count_class_b=0; // count for class B students
static int total_a =0; // total number of studetns from class A
static int total_b=0;// total number of students from class B

typedef struct
{
  int arrival_time;  // time between the arrival of this student and the previous student
  int question_time; // time the student needs to spend with the professor
  int student_id;   // students unique id
  int class;  // whihc class student is from ( A or B)
} student_info;

/*  initializing all synchronization
 variables and other global variables that you add.
 */
static int initialize(student_info *si, char *filename)
{
  students_in_office = 0;
  classa_inoffice = 0;
  classb_inoffice = 0;
  students_since_break = 0;

  /* Initializing  synchronization variables and
   other variables including semaphore , and mutex variables
   */

   sem_init(&mutex, 0 , 3);
   if(pthread_mutex_init(& lock_mutex, NULL)!=0)
   {
     printf("\n failed: mutex init\n");
     return -1;
   }
   if(pthread_mutex_init(& class_mutex, NULL)!=0)
   {
     printf("\n failed: mutex init\n");
     return -1;
   }


  /* Read in the data file and initialize the student array */
  FILE *fp;

  if((fp=fopen(filename, "r")) == NULL)
  {
    printf("Cannot open input file %s for reading.\n", filename);
    exit(1);
  }

  int i = 0;
  while ( (fscanf(fp, "%d%d%d\n", &(si[i].class), &(si[i].arrival_time), &(si[i].question_time))!=EOF) &&
           i < MAX_STUDENTS )
  {
    i++;
  }

 fclose(fp);
 return i;
}

/* Code executed by professor to simulate taking a break
 * You do not need to add anything here.
 */
static void take_break()
{
  printf("\nThe professor is taking a break now.\n\n");
  sleep(5);
  assert( students_in_office == 0 );
  printf("Profesor is resuming his office hour \n\n" );
  usleep(2);
  students_since_break = 0;
}

/* Code for the professor thread. This is fully implemented except for synchronization
 * with the students.  See the comments within the function for details.
 */
void *professorthread(void *junk)
{
  printf("The professor arrived and is starting his office hours\n");

  /* Loop while waiting for students to arrive. */
  while (1)
  {

    /* TODO */
    /* Add code here to handle the student's request.             */
    /* Currently the body of the loop is empty. There's           */
    /* no communication between professor and students, i.e. all  */
    /* students are admitted without regard of the number         */
    /* of available seats, which class a student is in,           */
    /* and whether the professor needs a break. You need to add   */
    /* all of this.                                               */
    int wait = 1; // wait variable for the while loop

    if(students_since_break ==BREAK_LIMIT) // is students since last break is zero then proessor takes a break
    {
      pthread_mutex_lock(&lock_mutex); // lock the mutex
      while(wait) // professor will wait untill all the students in the office are gone
      {
        if(students_in_office ==0) wait =0;
        usleep(0);
      }

      take_break(); // calling the take break fun while professor takes 5s spleep

      pthread_mutex_unlock(&lock_mutex); // unlock the mutex

    }
/*
if the counts student b is the last B student then release thread a
*/
    if( count_class_b == total_b)
    {
      pthread_cond_signal(&class_a_cv);
    }
    /*
    if the counts student A is the last B student then release thread b
    */
    if(count_class_a == total_a)
    {
      pthread_cond_signal(&class_b_cv);
    }



  }
  pthread_exit(NULL);
}


/* Code executed by a class A student to enter the office.
 * You have to implement this.  Do not delete the assert() statements,
 * but feel free to add your own.
 */
void classa_enter()
{

  /* TODO */
  /* Request permission to enter the office.  You might also want to add  */
  /* synchronization for the simulations variables below                  */
  /*  YOUR CODE HERE.                                                     */
  int wait = 1; // wait varible for the while loop
  sem_wait(&mutex); // semaphore waits, it will decrease count variable 3 by 1

  pthread_mutex_lock( &lock_mutex); // lock the mutex

  while(wait) // class A students wait untill  all the class B students in office are done
  {
    if(classb_inoffice ==0) wait =0;
    usleep(0);
  }

  pthread_mutex_unlock(&lock_mutex);// unlocking the mutex

  if(count_class_a ==5) // if students from class A been count 5 then wait and let B studetns go
  {
    pthread_cond_signal(&class_b_cv);
    pthread_cond_wait(&class_a_cv, &class_mutex);
    count_class_a = 0;
  }

        students_in_office += 1;
        students_since_break += 1;
        classa_inoffice += 1;
        count_class_a ++;
        count_class_b =0;

  usleep(0);
}

/* Code executed by a class B student to enter the office.
 * You have to implement this.  Do not delete the assert() statements,
 * but feel free to add your own.
 */
void classb_enter()
{

  /* TODO */
  /* Request permission to enter the office.  You might also want to add  */
  /* synchronization for the simulations variables below                  */
  /*  YOUR CODE HERE.                                                     */
  int wait = 1;
  sem_wait(&mutex); // semaphore waits, it will decrease count variable 3 by 1

  pthread_mutex_lock( &lock_mutex);

  while(wait) // class B students wait untill  all the class A students in office are done
  {
    if(classa_inoffice ==0) wait =0;
    usleep(0);
  }

  pthread_mutex_unlock(&lock_mutex);

  if(count_class_b==5) // if students from class A been count 5 then wait and let B studetns go
  {
    pthread_cond_signal(&class_a_cv);
    pthread_cond_wait(&class_b_cv, &class_mutex);
    count_class_b = 0;
  }



      students_in_office += 1;
      students_since_break += 1;
      classb_inoffice += 1;
      count_class_b++;
      count_class_a =0;

      usleep(0);


}

/* Code executed by a student to simulate the time he spends in the office asking questions
 * You do not need to add anything here.
 */
static void ask_questions(int t)
{
  sleep(t);
}


/* Code executed by a class A student when leaving the office.
 * You need to implement this.  Do not delete the assert() statements,
 * but feel free to add as many of your own as you like.
 */
static void classa_leave()
{
  /*
   *  TODO
   *  YOUR CODE HERE.
   */
   sem_post(&mutex); // post the semaphre variable, means it will increase the count variable by 1


  students_in_office -= 1;
  classa_inoffice -= 1;


      if( classa_inoffice ==0) // if student is the last in office, it will signal the class b student to enter
      {
        pthread_cond_signal(&class_b_cv);

      }

}

/* Code executed by a class B student when leaving the office.
 * You need to implement this.  Do not delete the assert() statements,
 * but feel free to add as many of your own as you like.
 */
static void classb_leave()
{
  /*
   * TODO
   * YOUR CODE HERE.
   */

   sem_post(&mutex); // post the semaphre variable, means it will increase the count variable by 1

  students_in_office -= 1;
  classb_inoffice -= 1;


      if( classb_inoffice ==0) // if student is the last in office, it will signal the class b student to enter
      {
        pthread_cond_signal(&class_a_cv);

      }

}

/* Main code for class A student threads.
 * You do not need to change anything here, but you can add
 * debug statements to help you during development/debugging.
 */
void* classa_student(void *si)
{
  student_info *s_info = (student_info*)si;

  /* enter office */
  classa_enter();

  printf("Student %d from class A enters the office\n", s_info->student_id);

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classb_inoffice == 0 );

  /* ask questions  --- do not make changes to the 3 lines below*/
  printf("Student %d from class A starts asking questions for %d minutes\n", s_info->student_id, s_info->question_time);
  ask_questions(s_info->question_time);
  printf("Student %d from class A finishes asking questions and prepares to leave\n", s_info->student_id);

  /* leave office */
  classa_leave();
  printf("Student %d from class A leaves the office\n\n", s_info->student_id);



  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);

  pthread_exit(NULL);
}

/* Main code for class B student threads.
 * You do not need to change anything here, but you can add
 * debug statements to help you during development/debugging.
 */
void* classb_student(void *si)
{
  student_info *s_info = (student_info*)si;

  /* enter office */
  classb_enter();


  printf("Student %d from class B enters the office \n", s_info->student_id);

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);
  assert(classa_inoffice == 0 );

  printf("Student %d from class B starts asking questions for %d minutes\n", s_info->student_id, s_info->question_time);
  ask_questions(s_info->question_time);
  printf("Student %d from class B finishes asking questions and prepares to leave\n", s_info->student_id);

  /* leave office */
  classb_leave();

  printf("Student %d from class B leaves the office\n\n", s_info->student_id);

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);

  pthread_exit(NULL);
}

/* Main function sets up simulation and prints report
 * at the end.
 */
int main(int nargs, char **args)
{
  int i;
  int result;
  int student_type;
  int num_students;
  void *status;
  pthread_t professor_tid;
  pthread_t student_tid[MAX_STUDENTS];
  student_info s_info[MAX_STUDENTS];

  if (nargs != 2)
  {
    printf("Usage: officehour <name of inputfile>\n");
    return EINVAL;
  }

  num_students = initialize(s_info, args[1]);

  if (num_students > MAX_STUDENTS || num_students <= 0)
  {
    printf("Error:  Bad number of student threads. "
           "Maybe there was a problem with your input file?\n");
    return 1;
  }

  printf("Starting officehour simulation with %d students .\n",
    num_students);

  result = pthread_create(&professor_tid, NULL, professorthread, NULL);

  if (result)
  {
    printf("officehour:  pthread_create failed for professor: %s\n", strerror(result));
    exit(1);
  }

  for (i=0; i < num_students; i++)
  {

    s_info[i].student_id = i;
  //  sleep(s_info[i].arrival_time);

    student_type = random() % 2;

    if (s_info[i].class == CLASSA)
    {
      result = pthread_create(&student_tid[i], NULL, classa_student, (void *)&s_info[i]);
      total_a++; // increment the student from class A
    }
    else // student_type == CLASSB
    {
      result = pthread_create(&student_tid[i], NULL, classb_student, (void *)&s_info[i]);
      total_b++; // increment the student count from class B
    }

    if (result)
    {
      printf("officehour: thread_fork failed for student %d: %s\n",
            i, strerror(result));
      exit(1);
    }
  }

  /* wait for all student threads to finish */
  for (i = 0; i < num_students; i++)
  {
    pthread_join(student_tid[i], &status);
  }

  /* tell the professor to finish. */
  pthread_cancel(professor_tid);

  printf("Office hour simulation done.\n");

  return 0;
}
