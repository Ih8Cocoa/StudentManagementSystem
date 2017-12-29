#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<ctype.h>
#include<time.h>
#ifdef _WIN32
	#include<conio.h>
	const char CLR[]="cls"; //define clear screen command on Windows
	const char RESIZE[]="mode 300,40"; //define resize command on Windows
#elif __linux__
	#include <termios.h>
	const char CLR[]="clear"; //define clear screen command on Linux
	const char RESIZE[]="stty cols 300 && stty line 40"; //define resize command on Linux
	//getch(), strupr() and strlwr() are only available on Windows, go they need to be re-declared on other platforms
	char _getch(); 
	void _strupr(char inp[]);
	void _strlwr(char inp[]);
#else
	#include <termios.h>
	//Does not support clear-screen and resizing commands on other platforms at the moment
	const char CLR[]="clear";
	const char RESIZE[]="";
	//re-declare getch(), strupr() and strlwr()
	char _getch(); 
	void _strupr(char inp[]);
	void _strlwr(char inp[]);
#endif

//define type student
typedef struct{
    char sId[10], sName[50], sDoB[11], sEmail[50], sPhone[20], sAddress[50];
    short exist; //if a slot's exist value = 0 then it is allowed to be overwritten
} student;

//main functions
student addStudent(student st0); //adding/editing students
void searchStudent(student s[], char search[]); //search for students
void help(); //prints out help

//functions that handles entering info
void getId(char Id_get[]); //get the ID

//side functions
short chkId(char Id[]); //check if a student ID is valid
short isBirthday(unsigned, unsigned, unsigned); //check if it's a birthday
short isDatabaseEmpty(student s[]); //check if the database is empty
short isEmail(char inp[]); //check if it's email
short isNum(char inp[]); //check if the whole string is comprised of numbers
short isPhone(char inp[]); //check if it's a phone number
short isText(char inp[]); //check if the whole string is comprised of numbers
void antiOvf(char Jeff[], unsigned short); //prevent overflowing data
void clr(); //clear screen
void tab(); //prepare tabular format
void tab_s(student st); //print out student's info in a tabular format

char Id_e[30]; //this variable is global, and can be used by all functions
const unsigned int MAX=1001;

int main(){
	system(RESIZE); //resize console window (only tested on Windows) by passing the defined string to system()
    unsigned short select, srAv=0; //select = user option, srAv = search availability
    unsigned i,j;
    char sr[101]; //contains search information entered by user
    student s[1000]; //initiate storage
    for(i=0; i<MAX; i++){
    	s[i].exist=0; //set all slots to be overwritable
	}
    while(2!=1){ //loop infinitely until user enters 0
    	sr[0]='\0';
    	Id_e[0]='\0';
    	select=0;
    	clr(); //clear screen
    	printf("==========================\n\n\t1. Add new student\n\t2. Display student database\n\t3. Search for a student\n\t4. Del");
    	printf("ete a student\n\t5. Update student information\n\t6. Help\n\t0. Exit\n\n==========================\n\nPlease choose: ");
    	//prevent crashing when user enters invalid input
    	if((!scanf("%hu", &select))||(select>6)){
    		printf("Error: Invalid input");
    		scanf("%*[^\n]"); //fflush(stdin) but works on all platforms
			_getch();
			#ifndef _WIN32
				_getch();
			#endif
    		continue;
		}
		getchar(); //scanf leaves a "\n". getchar() prevents fgets from getting it.
		clr(); //clear screen
		if(select<6&&select>0){
			/* This is no longer true, since the console is automatically resized.
			printf("Please maximize the console window for the best experience\n");*/
			if(select!=2){
				printf("At any time, input \"-1\" to return to the main menu\n\n");
			}
			//printf("\n");
		}
		switch(select){
			case 0: //exit
				exit(0);
			case 1: //adding student
				for(i=0; i<MAX; i++){
					if(s[i].exist==0){ //check if a student slot is allowed to be overwritten
						printf("Student ID (7 characters, must start with GC or GT and ends with 5 number digits): ");
						getId(sr);
						if(strcmp(sr, "-1")==0){
							//jump to caseEnd
							goto caseEnd;
						}
						if(strlen(sr)==7){
							strcpy(Id_e, sr); //
						} else {
							Id_e[0]='\0';
						}
						_strupr(Id_e); //upcase the whole string
						//check if the ID already exist
						for(j=0; j<MAX; j++){
							if((strncmp(Id_e,s[j].sId, 7)==0)&&(i!=j)&&s[j].exist){
								printf("ERROR: Student ID already exist in database");
								_getch();
								goto caseEnd; //jump to the end of case 1
							}
						}
						//if not, add the user to database
						s[i]=addStudent(s[i]);
						break;
					}
				}
				break;
			case 2: //View all students
				//if the database is empty, output "Database empty"
				if(isDatabaseEmpty(s)){
					puts("Database empty");
				} else {
					//print out student list in a tabular format
					tab();
					for(i=0; i<MAX; i++){
						//only print out students that was marked as exist
						if(s[i].exist){
							tab_s(s[i]);
						}
					}
				}
				printf("\nPress any key to return...");
				_getch();
				break;
			case 3:
				//if the database is empty, output "Database empty"
				if(isDatabaseEmpty(s)){
					puts("Database empty");
				} else { //search student
					printf("Input student name to search (Maximum 25 characters): ");
					fgets(sr, 40, stdin);
					while(strlen(sr)>25||!isText(sr)){
						if(strcmp(sr, "-1\n")==0){
							//jump to caseEnd
							goto caseEnd;
						}
						antiOvf(sr, 39);
						printf("Search key is ");
						if(strlen(sr)>25){
							printf("too long");
						} else if(!isText(sr)){
							printf("invalid");
						}
						printf(", please re-enter: ");
						sr[0]='\0';
						fgets(sr, 40, stdin);
					}
					sr[strlen(sr)-1]='\0';
					//downcasing "search"
					_strlwr(sr);
					//pass list and search key to searchStudent
					searchStudent(s, sr);
				}
				printf("\nPress any key to return...");
				_getch();
				break;
			case 4: //delete student
				if(isDatabaseEmpty(s)){
					printf("Database Empty");
				} else {
					printf("Enter student's ID to delete (7 characters, must start with GC or GT and ends with 5 number digits): ");
					getId(sr);
					if(strcmp(sr, "-1")==0){
						//go back to main menu
						break;
					}
					_strupr(sr); //upcase the student ID to be deleted
					for(i=0; i<MAX; i++){
						if((strcmp(s[i].sId, sr)==0)&&(s[i].exist)){
							//s[i].exist=0; //set the student slot to be overwritable
							puts("");
							tab(); //prepare tabular format
							srAv=1; //remember that a match was found
							break;
						}
					}
					//if a match was found, inform user that it was deleted.
					//otherwise, inform user that it wasn't found
					if(srAv){
						for(i=0; i<MAX; i++){
							if((strcmp(s[i].sId, sr)==0)&&(s[i].exist)){
								//s[i].exist=0; //set the student slot to be overwritable
								tab_s(s[i]); //print out student's info
								printf("Would you like to delete this student? (y/n): "); //prompt the user
								//mark the loopback
								while(j!='y'&&j!='n'){
									j=_getch();
									printf("%c\n", j);
									switch(j){
										case 'y':
											//if the input is y
											printf("Student %s (ID: %s) has been deleted.\n", s[i].sName, s[i].sId);
											s[i]=(student){0};
											s[i].exist=0;
											break;
										case 'n':
											//if the input is n
											printf("");
											break;
										default:
											//otherwise (invalid input)
											printf("Invalid input, please re-enter (y/n): ");
											break;
									}
								}
								j=0;
								break;
							}
						}
					} else {
						//if there are no students with a matching ID
						printf("Student ID %s not found\n", sr);
					}
				}
				srAv=0;
				printf("\nPress any key to return...");
				_getch();
				break;
			case 5: //update student's info
				if(isDatabaseEmpty(s)){
					printf("Database Empty\n\nPress any key to return...");
					_getch();
				} else {
					printf("Enter student ID to update (7 characters, must start with GC or GT and ends with 5 number digits): ");
					getId(sr);
					if(strcmp(sr, "-1")==0){
						//go back to main menu
						break;
					}
					_strupr(sr); //upcase student ID
					for(i=0; i<MAX; i++){
						if((strcmp(s[i].sId, sr)==0)&&(s[i].exist)){
							//if a match was found and the student exists, print out tabular format
							tab();
							tab_s(s[i]);
							//then perform the same steps as addStudent()
							s[i]=addStudent(s[i]);
							srAv=1; //match found
							break;
						}
					}
					if(!srAv){
						//if a match wasn't found (srAv=0)
						printf("Student %s not found\n\nPress any key to return...", sr);
						_getch();
					}
				}
				srAv=0;
				break;
			case 6:
				//help
				printf("ON-SCREEN HELP\n\nPress the number key equivalent to the function that you need help with, or press 0 to return to ");
				printf("the main menu\n\n==========================\n\n\t1. Add new student\n\t2. Display student database\n\t3. Search for");
			    printf(" a student\n\t4. Delete a student\n\t5. Update student information\n\n==========================\n\nPlease choose: ");
				help();
				break;
			default:
				exit(0);
		}
		caseEnd:
			//does nothing
			printf("");
	}
}

//start main functions

student addStudent(student st0){
	unsigned d,m,y; //for storing date to verify
	char Id[30], Name[70], DoB[20], Email[70], Phone[50], Address[70]; //initiate temporary storage
	if(st0.exist){
		//update student's ID
		printf("Please enter the new information below, or just press enter to kepp the old one\n");
		printf("Student ID: ");
		regetId: //get ID again
		fgets(Id, 15, stdin);
		while(strlen(Id)>10){
			antiOvf(Id, 14);
			printf("Invalid student ID. Please re-enter: ");
			goto regetId;
		}
		Id[strlen(Id)-1]='\0';
		_strupr(Id);
		while(chkId(Id)==0){
			if(strcmp(Id, "-1")==0){
				return st0;
			}
			if(Id[0]=='\0'&&st0.exist)
				break;
			Id[0]='\0';
			printf("Invalid student ID. Please re-enter: ");
			goto regetId;
		}
	} else {
		//just add student
		strcpy(Id, Id_e);
	}
	printf("Student name (Maximum 25 characters): ");
	fgets(Name, 40, stdin);
	while(strlen(Name)>25||strcmp(Name, "-1\n")==0||(Name[0]=='\n'&&st0.exist==0)||isText(Name)==0){
		if(strcmp(Name, "-1\n")==0){
			//return to main memu
			return st0;
		}
		antiOvf(Name, 39); //prevent overflow
		printf("Invalid name. Please re-enter name: ");
		Name[0]='\0'; //terminate string
		fgets(Name, 40, stdin);
	}
	Name[strlen(Name)-1]='\0'; //remove "trailing newline" - StackOverflow
	printf("Date of birth (dd/mm/yyyy): ");
	fgets(DoB, 15, stdin);
	while(strlen(DoB)>11||(!sscanf(DoB, "%d/%d/%d", &d, &m, &y))||isBirthday(d,m,y)==0){
		if(DoB[0]=='\n'&&st0.exist){
			break;
		}
		antiOvf(DoB, 14); //prevent overflow
		if(strcmp(DoB, "-1\n")==0)
			return st0;
		DoB[0]='\0';
		printf("Invalid birthday. Please re-enter: ");
		fgets(DoB, 15, stdin);
	}
	if(DoB[0]!='\n'){
		DoB[0]='\0'; //reset
		sprintf(DoB, "%02d/%02d/%02d", d, m, y); //reorganizing birthday
	} else {
		DoB[0]='\0'; //reset
	}
	printf("Email address (Maximum 30 character, must have a @ or a dot and no space): ");
	fgets(Email, 40, stdin);
	while(strlen(Email)>31||(!isEmail(Email))||(Email[0]=='\n'&&st0.exist==0)){
		if(strcmp(Email, "-1\n")==0){
			//go back
			return st0;
		}
		antiOvf(Email, 39); //prevent overflow
		Email[0]='\0';
		printf("Invalid email. Please re-enter: ");
		fgets(Email, 40, stdin);
	}
	Email[strlen(Email)-1]='\0'; //remove "trailing newline" - StackOverflow
	_strlwr(Email);
	printf("Phone number (Maximum 15 numbers, either only numbers or start with \"+\" and only number afterwards): ");
	fgets(Phone, 20, stdin);
	while((strlen(Phone)>15||!isPhone(Phone))||(strcmp(Phone, "-1\n")==0)||(Phone[0]=='\n'&&st0.exist==0)){
		if(strcmp(Phone, "-1\n")==0){
			//go back
			return st0;
		}
		antiOvf(Phone, 19); //prevent overflow
		Phone[0]='\0';
		printf("Invalid phone number. Please re-enter: ");
		fgets(Phone, 20, stdin);
	}
	Phone[strlen(Phone)-1]='\0'; //remove "trailing newline" - StackOverflow
	printf("Address (Maximum 30 characters): ");
	fgets(Address,40,stdin);
	while(strlen(Address)>31||strcmp(Address, "-1\n")==0||(Address[0]=='\n'&&st0.exist==0)){
		if(strcmp(Address, "-1\n")==0){
			//go back
			return st0;
		}
		antiOvf(Address, 39); //prevent overflow
		printf("Invalid address. Please re-enter: ");
		Address[0]='\0';
		fgets(Address,40,stdin);
	}
	Address[strlen(Address)-1]='\0'; //remove "trailing newline" - StackOverflow
	student st; //initiate struct st
	st=(student){0}; //set it as empty
	switch(st0.exist){
		case 0: //adding student
			strcpy(st.sAddress, Address);
			strcpy(st.sDoB, DoB);
			strcpy(st.sEmail, Email);
			strcpy(st.sName, Name);
			strcpy(st.sPhone, Phone);
			strcpy(st.sId, Id);
			break;
		case 1: //updating student
			st=st0;
			if(Address[0]!='\0'){
				strcpy(st.sAddress, Address);
			}
			if(DoB[0]!='\0'){
				strcpy(st.sDoB, DoB);
			}
			if(Email[0]!='\0'){
				strcpy(st.sEmail, Email);
			}
			if(Name[0]!='\0'){
				strcpy(st.sName, Name);
			}
			if(Phone[0]!='\0'){
				strcpy(st.sPhone, Phone);
			}
			if(Id[0]!='\0'){
				strcpy(st.sId, Id);
			}
			break;
	}
	if(!st0.exist){
		//add student
		printf("Student %s has been added to the database", st.sName);
	} else {
		//update student
		printf("Student %s has been updated", st0.sId);
	}
	st.exist=1;
	_getch();
	return st;
}

void searchStudent(student s[], char search[]){ //search student
	short i, studentAvailable=0;
	char c1[33];
	for(i=0; i<MAX; i++){
		//copy and downcase student name to c1
		strcpy(c1, s[i].sName);
		_strlwr(c1);
		//strstr: check if string2 is a substring of string1
		if(strstr(c1, search)&&s[i].exist){
			//remember that at least a student match the search criteria
			studentAvailable=1; 
			tab(); //prepare tabular format
			break;
		}
	}
	if(studentAvailable){
		//if srAv=1, print out all students that match the search criteria
		for(i=0; i<MAX; i++){
			strcpy(c1, s[i].sName);
			_strlwr(c1);
			if(strstr(c1, search)&&s[i].exist){
				tab_s(s[i]);
			}
		}
	} else {
		//srAv=0 -> inform the user that the student wasn't found
		printf("Student %s not found", search);
	}
	puts("");
}

void help(){
    char c;
    c=_getch();
    clr();
	printf("ON-SCREEN HELP\n\nPress the number key equivalent to the function that you need help with, or press 0 to return to ");
	printf("the main menu\n\n==========================\n\n\t1. Add new student\n\t2. Display student database\n\t3. Search for");
    printf(" a student\n\t4. Delete a student\n\t5. Update student information\n\n==========================\n\nPlease choose: ");
    printf("%c\n\n", c);
    switch(c){
    	case '0':
    		return;
    		break;
    	case '1':
    		puts("Adding new student");
    		puts("This function will add a new student's information to the database");
    		puts("You'll have to enter the following information:");
    		puts("1. Student ID: 7 characters, must start with GC or GT and ends with 5 number digits");
    		puts("2. Student name: Maximum 25 characters, without any numbers or special characters");
    		puts("3. Date of birth: Enter the student's birthday in the dd/mm/yyyy format");
    		puts("\tExamples of acceptable birthday inputs: 02/07/1999, 12/9/2001, 6/09/1997, ...");
    		puts("4. Email Address: Must include only one \"@\" and at least one \".\" and with no spaces between it");
    		puts("\tSpecial characters that are not allowed in the email include:");
    		puts("\tDouble quote, double period, \"(\", \")\", \",\", \":\", \";\", \"<\", \">\", \"[\", \"]\", \"\\\", etc.");
    		puts("5. Phone number: The 1st character can be either a \"+\" or a number. Characters after that must be numbers");
    		puts("6. Address: Student's home address");
    		puts("Empty inputs are not allowed at the moment.");
    		printf("At any time, input -1 to abort the process and return to the main menu.");
    		help();
    		break;
    	case '2':
    		puts("Displaying student database");
    		puts("This function will display the entire student list in the database");
    		printf("After viewing, press any key to return.");
    		help();
    		break;
    	case '3':
    		puts("Searching for a student");
    		puts("This function allows you to search for a student with a part of the student's name");
    		puts("Enter the keyword that you want to search for, then press enter. Input \"-1\" to return to the main menu");
    		printf("If a student is found with a matching name, it will be displayed.");
    		help();
    		break;
    	case '4':
    		puts("Deleting a student");
    		puts("This function allows you to remove a student's information from the database");
    		puts("Input the student's ID, then press enter. Input \"-1\" to return to the main menu");
    		puts("If the student's ID exists in the database, that student's information will be displayed.");
    		printf("You will the be prompted to confirm the deletion. Press \"y\" to confirm, or \"n\" to abort the process");
    		help();
    		break;
    	case '5':
    		puts("Updating a student's information");
    		puts("This function allows you to update a student's information in the database");
    		puts("Input the student's ID, then press enter. Input \"-1\" to return to the main menu");
    		puts("Then, input each new information as you are prompted.");
    		puts("\tBy skipping a prompt by pressing Enter key, its corresponding field's information will be retained");
    		printf("After all the information you entered is verified, the student's information in the database will be updated.");
    		help();
    		break;
    	default:
    		printf("Option unavailable");
    		help();
    		break;
	}
}

//end main functions

//start side functions

short chkId(char Id[]){ //check if it's a valid ID
	char sub[30];
	if(strlen(Id)!=7){
		//if ID length is different from 7
		return 0;
	}
	strncpy(sub, Id, 10);
	if((strncmp(sub, "GC",2))&&(strncmp(sub, "GT",2))){
		//if the ID doesn't contain GC or GT
		return 0;
	}
	sub[0]='\0';
	strncpy(sub, Id+2, 5); //copy the last 5 character of the ID
	if(!(isNum(sub))){
		//if it's not a number-only string
		return 0;
	}
	return 1;
}

short isBirthday(unsigned day, unsigned month, unsigned year){
	//get current year and set it as maximum
	char yearmax[5];
	time_t timer;
	time(&timer);
	strftime(yearmax, 5, "%Y", localtime(&timer));
	//now check if it's a valid birthday
	if((year<1920)||(year>=atoi(yearmax))){
		return 0;
	}
	if(month>12){
		return 0;
	}
	switch(month){
		case 4:
		case 6:
		case 9:
		case 11:
			if(day>30)
				return 0;
			break;
		case 2:
			if((day==29)&&(year%4))
				return 0;
			else if(day>28)
				return 0;
			break;
		default:
			if(day>31)
				return 0;
			break;
	}
	return 1;
}

short isDatabaseEmpty(student s[]){ //check if database is empty
	int i, rtn=1;
	for(i=0; i<MAX; i++){
		if(s[i].exist==1){
			rtn=0;
			break;
		}
	}
	return rtn;
}

short isEmail(char inp[]){
	//check if it's a valid email address
	if(strcmp(inp, "\n")==0)
		return 1;
	int f_a=0, f_dot=0,i, rtn=1;
	//these special characters are not allowed in an email address
	char invalid[13][3]={"\"", "(", ")", ",", ":", ";", "<", ">", "[", "]", "\\", "..", " "}; 
	for(i=0; i<13; i++){
		//if there is an "invalid[][]" character in the address
		if((strstr(inp, invalid[i]))){
			rtn=0;
		}
	}
	for(i=0; inp[i]!='\0'; i++){
    	if(inp[i]=='@')
           f_a++;
        if(inp[i]=='.')
        	f_dot++;
	}
	if(f_a!=1)
		rtn=0; //if it has more than one @
	if(f_dot>3||f_dot==0)
		rtn=0;  //too many dots
	if(strlen(inp)<6)
		rtn=0; //if it's too short
	return rtn;
}

short isNum(char inp[]){ //check if it's only made of numbers
	short i, rtn=1;
	for(i=0; i<strlen(inp)-1; i++){
		if(!isalnum(inp[i])){
			//if it contains a special character
			rtn=0;
		}
	}
	for(i=0; i<strlen(inp); i++){
		if(isalpha(inp[i])){
			//if it contains a letter
			rtn=0;
		}
	}
	return rtn;
}

short isPhone(char inp[]){
	if(strcmp(inp, "\n")==0)
		return 1;
	char sub[30];
	short i, rtn=1;
	if(isalnum(inp[0])==0){
		//if the first character is a special character
		if(strncmp(inp, "+", 1)!=0){
			//and if it's not a plus sign
			rtn=0;
		} else {
			//and if it's a plus sign
			strncpy(sub, inp+1, strlen(inp)-2);
			if(isNum(sub)==0){
				//and the other characters doesn't comprise a number-only string
				rtn=0;
			}
		}
	} else if(isNum(inp)==0){
		//otherwise, if it's not a number-only string
		rtn=0;
	}
	//Make sure the phone number is a valid Vietnam phone number (in 2017).
	/*else {
		sub[0]='\0';
		strncpy(sub, inp, 2);
		if((strncmp(sub, "01", 2)!=0)&&(strncmp(sub, "02", 2)!=0)&&(strncmp(sub, "09", 2)!=0)&&(strncmp(sub, "3", 1)!=0)){
			rtn=0;
		}
	}*/
	return rtn;	
}

short isText(char inp[]){ //check if it's only made of text
	short i, rtn=1;
	for(i=0; i<strlen(inp)-1; i++){
		if(isalnum(inp[i])==0&&inp[i]!=' '){
			//if there is a special character but it's not a space
			rtn=0;
		}
	}
	for(i=0; i<strlen(inp); i++){
		if(isdigit(inp[i])){
			//if it has a number digit in it
			rtn=0; 
			break;
		}
	}
	return rtn;
}

void antiOvf(char inp[], unsigned short maxlength){
	//prevent overflow inputs
	char c;
	int length = strlen(inp);
	/*fgets always leave a "\n" at the end of a character
	  If that "\n" doesn't appear after the specified fgets limit, it is assumable that the input will cause an overflow
	  and it will stuff the input with several "\n"s.
	  Thus, discard all the overflowing stuff using a "while" loop
	*/
	if(length==maxlength&&inp[maxlength-1]!='\n')
		while((c=getchar())!='\n'&&c!=EOF);
	return;
}

void clr(){ //clear screen
	system(CLR); //pass the defined clear-screen command to system
}

void getId(char Idget[]){
	//this function will get the ID from user input
	char Id_get[30];
	//mark the loopback point
	regetIdf:
	fgets(Id_get, 15, stdin);
	while(strlen(Id_get)>9){
		//if the ID is too long
		antiOvf(Id_get, 14);
		printf("Invalid student ID. Please re-enter: ");
		goto regetIdf;
	}
	Id_get[strlen(Id_get)-1]='\0'; //remove the "trailing newline"
	_strupr(Id_get); //upcase the ID
	while(chkId(Id_get)==0){
		if(strcmp(Id_get, "-1")==0){
			//if it is the termination signnal, return it as it is
			break;
		}
		//otherwise, prompt the user to re-enter ID
		Id_get[0]='\0';
		printf("Invalid student ID. Please re-enter: ");
		goto regetIdf;
	}
	//then export the ID to the main variable
	strcpy(Idget, Id_get);
	return;
}

void tab(){
	printf("%-7s\t| %-25s\t| %-10s\t| %-30s\t| %-15s\t| %s\n", "ID", "Name", "Date of birth", "Email", "Phone number", "Address");
}

void tab_s(student st){
	printf("%-7s\t| %-25s\t| %-10s\t| %-30s\t| %-15s\t| %s\n", st.sId, st.sName, st.sDoB, st.sEmail, st.sPhone, st.sAddress);
}

//end side functions

//redeclare missing functions for non-Windows platforms
#ifndef _WIN32
//getch()
char _getch()
{
    struct termios oldSettings, newSettings; 
    tcgetattr(0, &oldSettings); //get old terminal I/O settings at file descriptor 0 (which means stdin), and store it in the "old" (global) variable
	//note: 0 (stdin), 1 (stdout)
    newSettings = oldSettings; //copy old settings to new
	/*"The c_lflag member itself is an integer, and you change the flags and fields using the operators &, |, and ^.
	Don’t try to specify the entire value for c_lflag—instead, change only specific flags and leave the rest untouched"
	- https://www.gnu.org/software/libc/manual/html_node/Local-Modes.html*/
    newSettings.c_lflag &= ~(ICANON); //set new c_local-flag to disable ICANON flag using bitwise -> disable canonical (line-editing) mode, 
	//which allows user to press a key to input its value, without pressing Enter
    tcsetattr(0, TCSANOW, &newSettings); //now, apply new terminal settings to file descriptor 0 (stdin) (TCSANOW: change settings immediately)
    char ch;
    ch = getchar(); //get the desired character
    tcsetattr(0, TCSANOW, &oldSettings); //and switch canonical mode back on
    return ch;
}

//strupr()
void _strupr(char inp[]){
	unsigned i;
	for(i=0; i<strlen(inp); i++){
        inp[i] = toupper(inp[i]);
    }
}

//strlwr()
void _strlwr(char inp[]){
	unsigned i;
	for(i=0; i<strlen(inp); i++){
        inp[i] = tolower(inp[i]);
    }
}
#endif
