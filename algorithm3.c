/****************************

Author: Alex Marti
Date: 10/12/19
Comments:
  
  This is a work in progress example of a potential algorithm
  for securing the can bus. it is similar to the algorithm that I 
  have made but not quite the same as I am working things out to
  put them in actual code. Use this as a basis for what will be to
  come.

*****************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

char message[128];

typedef struct{
	int ID;
} NODE;

typedef struct{
	NODE* ListNodes;
	int pos;
} ECU;

void fromDecimalToBinary(unsigned long long, unsigned long long);

void C(unsigned long long, unsigned long long);

static inline unsigned long long* B(const char*, unsigned long long*);

char* sendMessage(char*);

int random25();

int seed_random();

void generate_column(NODE* *nodes, int numNodes);

int MTD_update(NODE* *nodes, int numNodes, int newID, int position);

int find_position(NODE* *nodes, int numNodes, int old_ID);

int main( int argc, char* argv[] )
{
	memset(message, 0, 128);
	
	unsigned long long x,y;

	x=1234567890987654321;
	y=0;

	C(x,y);
	
	printf("%s\n", message);
	getchar();
	
	unsigned long long jkl[2];
	unsigned long long* q = B(message, jkl);
	
	printf("%llu %llu\n",q[0], q[1]);
	getchar();
	
	memset(message, 0, 128);
	
	fromDecimalToBinary(x,y);
	
	printf("Message2 %s\n", message);
	getchar();
	
	q = B(message, jkl);
	
	printf("%llu %llu\n",q[0], q[1]);
	getchar();

	int old_id=0;

	int position=0;

	int regenerate=1;

	int numNodes=2;
	//get the number of nodes on the BUS
	//this maybe information stored on the bus somehwere?
	
	//scanf("%d", &numNodes); // this can be used for later testing if we want to add more nodes to the bus

	ECU ecu1, ecu2;
	ecu1.ListNodes = (NODE*)malloc(numNodes * sizeof(NODE));	
	ecu2.ListNodes = (NODE*)malloc(numNodes * sizeof(NODE));

	ecu1.pos=0;
	ecu2.pos=1;

	ecu1.ListNodes[ecu1.pos].ID =0;
	ecu2.ListNodes[ecu2.pos].ID =1;

	int seed_int = 0; //because the random number I am generating is only 0-24 will need a larger sample to seed the random number
	
	int r_int = 0; //the random integer value that will either be read from a file or created for the first time
	
	FILE *fptr; //the pointer to the file that holds the previous state of the random integer

	//char *filename;
	
	//filename = "/home/alex/Documents/InsureProject/randomint.txt"; //current location of the random number file will be changed
	char file[] = "/home/alex/Documents/InsureProject/randomint.txt"; //current location of the random number file will be changed
	
	fptr = fopen(file, "w+"); //reading the text file with the previous state of the random number
	//        ^
	//        |
	/*this is not finding the file and I cannot for the life of me figure out what is going on. 
	  I know the file is there, but it wont find it.
	  */
	
	if(fptr == NULL) //if file doesnt exist, exit with error 1
	{
		printf("error\n");
		exit(1);
	}

	if(fseek(fptr, 0, SEEK_END) == 0) //if the file doesnt exit (first time running the system or reset/deleted the file) 
	{
		printf("empty\n");
		time_t t;
		srand((unsigned) time(&t)); //seed the random number generator
	}
	
	if(fseek(fptr, 0, SEEK_END) > 0) //if the file exists and is not empty then...
	{
		printf("not empty\n");
		rewind(fptr); //put the pointer back at the beginning of the file to read the number
		while(!feof(fptr))
			fscanf(fptr, "%d", &r_int); //read the number of the prvious state from the file
		srand((unsigned) r_int); //seed the random number generator
	}
		
	generate_column(&ecu1.ListNodes, numNodes); //the initial generation of the column to be used for replacing

	generate_column(&ecu2.ListNodes, numNodes); //the initial generation of the column to be used for replacing
	
	//position = find_position(&ecu1.ListNodes, numNodes, ecu1.ListNodes.ID); //returns the index of the position of the found id for the node calling the function

	//position = find_position(&ecu2.ListNodes, numNodes, ecu2.ListNodes.ID);

	/********************
	  this is where I will use the can bus api for C that I have found or
	  I can use assembly to do low level bit manipulation may not be neccesary though
	  will need to check documentation
	  **********************/
	

	//can bus send command from the Node
	//old_id is set from the command and the node is then found in the array of NODEs

	position = find_position(&ecu1.ListNodes, numNodes, ecu1.ListNodes[ecu1.pos].ID);//this is called to find the position of the ID that will be updated, this is passed to
								//the node from the bus and is used to update its own matrix of structures

	ecu1.ListNodes[position].ID = ecu2.ListNodes[position].ID = random25();//setting a new ID for the node	

	regenerate = MTD_update(&ecu1.ListNodes, numNodes, ecu1.ListNodes[position].ID, position); //calling a function to check the new ID for the node

	while(regenerate !=0) //if the id is in the system already then make another one and check it
	{
		ecu1.ListNodes[position].ID = ecu2.ListNodes[position].ID = random25(numNodes);
			
		regenerate = MTD_update(&ecu1.ListNodes, numNodes, ecu1.ListNodes[position].ID, position);
	}

	seed_int = seed_random(); //seeding the random number for the next time that the can bus is started
	
	fprintf(fptr, "%d", seed_int); //putting the random number in the file to be read for next time
	
	fclose(fptr); //closing the buffer of the file

	//free(nodes); //deallocate the memory for the nodes
	
	return(0);
}

int random25()
{
	return rand()%100;
}

int seed_random()
{
	return rand();
}

/*
   this function is the initial generation of the column
   it takes random numbers and puts them into the array of 
   nodes for the flow of information
   */

void generate_column(NODE* *nodes, int numNodes)
{
	int j=0;
	for(j<numNodes; j+=1;)
	{
		nodes[j]->ID=random25();
	}
}

/*
   this checks if the newly generated id is in
   the array of nodes, if it exists already then return 1
   else return 0
   */

int MTD_update(NODE* *nodes, int numNodes, int newID, int position)
{
	int i=0;
	for(i<numNodes; i+=1;)
	{
		if(nodes[i]->ID == newID && i != position)
			return(1);
	}
	return(0);

}

/*
   this returns the position of the node in the array to be used
   by the update function
   */

int find_position(NODE* *nodes, int numNodes, int ID)
{
	int i=0;
	for(i<numNodes; i+=1;)
	{
		if(ID==nodes[i]->ID)
			break;
	}
	return(i);
}

void fromDecimalToBinary(unsigned long long ID, unsigned long long mess)
{
	unsigned long long k,c;
	message[0]='1';
	int i=1;
	for(c=12; c>=1; c--)
	{
		printf("first loop %d\n", i);
		k = ID >> c;
		if(k & 1)
			message[i]='1';
		else
			message[i]='0';
		printf("first loop char %c\n", message[i]);
		i++;
		if(c==1)
			break;
	}
	//printf("I am in from decimal to binary %s\n", message);
	for(i=13; i<15; i++)
	{
		message[i]='0';
		if(i==14)
			break;
	}

	i=15;
	for(c=33; c>=15; c--)
	{
		printf("secnod loop %d\n", i);
		k = mess >> c;
		if(k & 1)
			message[i]='1';
		else
			message[i]='0';
		printf("second loop char %c\n", message[i]);
		i++;
		if(c==15)
			break;
	}
	for(i=34; i<=127; i++)
	{
		message[i]='0';
		if(i==127)
			break;
	}

	printf("I am in from decimal to binary %s\n", message);
	getchar();
}

void C(unsigned long long b1, unsigned long long b2)
{
	unsigned long long k,c;
	int i=0;
	for(c=63; c>=0; c--)
	{
		k = b1 >> c;
		if(k & 1)
			message[i]='1';
		else
			message[i]='0';
		i++;
		if(c==0)
			break;
	}
	for(c=127; c>=64; c--)
	{
		k = b2 >> c;
		if(k & 1)
			message[i]='1';
		else
			message[i]='0';
		i++;
		if(c==64)
			break;
	}
}

static inline unsigned long long* B(const char* s, unsigned long long* i)
{
    int j=0;
    while (*s && j!=64) {
        i[0] <<= 1;
        i[0] += *s++ - '0';
	j++;
    }
    while(*s)
    {
	    i[1] <<= 1;
	    i[1] += *s++ - '0';
    }
    printf("%llu %llu I am q[0] and q[1]\n", i[0], i[1]);
    getchar();
    return i;
}

char* sendMessage(char* message)
{

}
