#include <sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<stdio.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<time.h>
#include<string.h>

//--------------- structures declaration of train and user------------
struct train{
		int train_number;
		char train_name[50];
		int total_seats;
		int available_seats;
		};
struct user{
		int login_id;
		char password[50];
		char name[50];
		int type;
		};

struct booking{
		int booking_id;
		int type;
		int uid;
		int tid;
		int seats;
		};
//---------------------------------------------------------------------

// All function Calls declearation
int client_main(int socket);
int login_menu(int socket,int acc_type);
int user_options(int socket,int choice);
int train_modifications(int socket,int choice);
int user_modifications(int socket,int choice);

//---------------------------------------------------------------------


// Creating Main with socket connectors initialization and connections
int main(void){
	struct sockaddr_in server;
	int sd;
	char buffer[100];
	
	sd = socket(AF_INET, SOCK_STREAM,0);
	if (sd == -1) { 
        	printf("Socket creation failed.\n"); 
    	} 
    	printf("Socket created successfully.\n"); 
    	
    	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(8112);
	
	connect(sd, (struct sockaddr*)(&server), sizeof(server));
	printf("Something Happen1 \n");
	while(client_main(sd)!=3);                           // Start client main functions
	printf("Something Happen \n");
	close(sd);
	return(0);
	
	
}

//---------------------------------------------------------------------


// ------------ From Here client start functionaing operations

int client_main(int socket){
	int choice;
	//system("clear");
	
	printf("\n\n --------- TRAIN RESERVATION SYSTEM ------------- \n\n");
	printf("  1. Login \n");
	printf("  2. Sign Up\n");
	printf("  3. Exit\n");
	printf(" Enter Your Choice:: ");
	scanf("%d", &choice);
	write(socket, &choice, sizeof(choice));               // Wrote to server
	
	
	if (choice == 1){					// Log in
		int login_id,acc_type,flag;
		char password[50];
		printf(" Enter User ID:: ");
		scanf("%d", &login_id);
		strcpy(password,getpass(" Password:: "));
		write(socket, &login_id, sizeof(login_id)); // To server login & passw.
		write(socket, &password, sizeof(password));
		read(socket, &flag, sizeof(flag));           // from server flag validation
		if(flag){
			printf("Login successfully\n");
			read(socket,&acc_type,sizeof(acc_type));       // read type of account logeed in
			printf("Comes In \n");
			while(login_menu(socket,acc_type)!=-1);
			//system("clear");
			return 1;
		}
		else{
			printf(" Login Unsuccessful : Incorrect Login details. \n");
			return 1;
		}
	}
	
	
	else if(choice == 2){					// Sign up
		int acc_type,login_id;
		char name[50],password[50],secret_pin[6];      // secret_pin to create admin account. Here it is0 "admin"
		system("clear");
		printf("\n  Enter The Type Of Account:: \n");
		printf(" 0. Admin\n");
		printf(" 1. Agent\n");
		printf(" 2. Customer\n");
		printf(" Enter Your Choice :: ");
		scanf("%d", &acc_type);
		printf(" Please enter Your Name: ");
		scanf("%s", name);
		strcpy(password,getpass(" Enter Your Password: "));
		
		/*
		if(!acc_type){
			while(1){
				strcpy(secret_pin, getpass(" Please Enter secret PIN to create ADMIN account: "));
				if(strcmp(secret_pin, "admin")!=0) 					
					printf("\tInvalid PIN. Please Try Again.\n");
				else
					break;
			}
		}
		*/
		
		write(socket, &acc_type, sizeof(acc_type));
		write(socket, &name, sizeof(name));
		write(socket, &password, strlen(password));
		
		read(socket, &login_id, sizeof(login_id));
		printf("Please note down below login id for future logins \n");
		printf(" Generated login id :: %d\n", login_id);
		//print("Check1");
		return 2;
	}
	else{
		//print("Check2");							// Log out
		return 3;
	}
}



//---------------------------------------------------------------------

// ---------------------------- Login Menu comes after successful login --------------

int login_menu(int socket,int acc_type){
	int choice;                             //default
	if(acc_type==2 || acc_type==1){					// Agent and Customer
		printf("\n 1. Book Ticket\n");
		printf(" 2. View Bookings\n");
		printf(" 3. Update Booking\n");
		printf(" 4. Cancel booking\n");
		printf(" 5. Logout\n\n");
		printf(" select your Choice: ");
		//printf("\n\n");
		scanf("%d",&choice);
		write(socket,&choice,sizeof(choice));
		return user_options(socket,choice);
	}
	else if(acc_type==0){					// Admin
		printf("\n\n");
		printf("\n 1. operations on train\n");
		printf(" 2.  operations on user\n");
		printf(" 3. Logout\n");
		printf(" Select Your Choice: ");
		scanf("%d",&choice);
		write(socket,&choice,sizeof(choice));
			if(choice==1){
				printf("\n\n");
				printf(" 1. Add train\n");
				printf(" 2. View train\n");
				printf(" 3. Modify train\n");
				printf(" 4. Delete train\n");
				printf("\n\n");
				printf(" Select Your Choice: ");
				scanf("%d",&choice);	
				write(socket,&choice,sizeof(choice));
				return train_modifications(socket,choice);
			}
			else if(choice==2){
				printf("\n\n");
				printf(" 1. Add User\n");
				printf(" 2. View all users\n");
				printf(" 3. Modify user\n");
				printf(" 4. Delete user\n");
				printf("\n\n");
				printf(" Select Your Choice: ");
				scanf("%d",&choice);
				write(socket,&choice,sizeof(choice));
				return user_modifications(socket,choice);
			
			}
			else if(choice==3)
				return -1;
	}	
	
}


//---------------------------------------------------------------------

//---------------------- User Options client-----------------------

int user_options(int socket,int choice){
	int valid =0;
	if(choice==1){										// Book tickets
		int view=2,tid,seats;
		write(socket,&view,sizeof(int));
		train_modifications(socket,view);
		printf("\n\n");
		printf(" Enter the train number you want to book: ");
		scanf("%d",&tid);
		write(socket,&tid,sizeof(tid));
				
		printf("\n Enter the no. of seats you want to book: ");
		scanf("%d",&seats);
		write(socket,&seats,sizeof(seats));
	
		read(socket,&valid,sizeof(valid));
		if(valid)
			printf("\n Ticket booked successfully.\n");
		else
			printf("\n Seats were not available.\n");

		return valid;
	}
	else if(choice==2){									// View the bookings
		int no_of_bookings;
		int id,tid,seats;
		read(socket,&no_of_bookings,sizeof(no_of_bookings));
		printf("\n\n");
		printf(" B_id\tT_no\tSeats\n");
		while(no_of_bookings--){
			read(socket,&id,sizeof(id));
			read(socket,&tid,sizeof(tid));
			read(socket,&seats,sizeof(seats));
			
			if(seats!=0)
				printf(" %d\t%d\t%d\n",id,tid,seats);
		}

		return valid;
	}
		
	else if(choice==3){									// Update a booking (increment/ decrement seats)
		int choice = 2,bid,val,valid;
		user_options(socket,choice);                                                 // Show their booking details
		printf("\n  Enter the Booking id you want to modify: ");
		scanf("%d",&bid);
		write(socket,&bid,sizeof(bid));
		printf("\n\n");
		printf("\n 1. Increase number of seats");
		printf("\n 2. Decrease number of seats\n");
		printf("\n\n");
		printf("Please select Your Choice: ");
		scanf("%d",&choice);
		write(socket,&choice,sizeof(choice));

		if(choice==1){
			printf("\n No. of tickets to increase :: ");
			scanf("%d",&val);
			write(socket,&val,sizeof(val));
		}
		else if(choice==2){
			printf("\n No. of tickets to decrease :: ");
			scanf("%d",&val);
			write(socket,&val,sizeof(val));
		}
		read(socket,&valid,sizeof(valid));
		if(valid)
			printf("\n Booking updated successfully.\n");
		else
			printf("\n Updation failed. No more seats available.\n");
		return valid;
	}
	
	else if(choice==4){									// Cancel the entire booking
		int choice = 2,bid,valid;
		user_options(socket,choice);
		printf("\n Enter the Booking id you want to cancel: ");
		scanf("%d",&bid);
		write(socket,&bid,sizeof(bid));
		read(socket,&valid,sizeof(valid));
		if(valid)
			printf("\n Booking cancelled successfully.\n");
		else
			printf("\n Cancellation failed.\n");
		return valid;
	}
	else if(choice==5)									// Logout
		return -1;
	
}


//---------------------------------------------------------------------

//---------------------- Train Options & Modificatios (Admin)-----------------------

int train_modifications(int socket,int choice){
	int valid = 0;
	
	if(choice==1){				// Add train response
		char tname[50];
		printf("\n Enter train name: ");
		scanf("%s",tname);
		write(socket, &tname, sizeof(tname));
		read(socket,&valid,sizeof(valid));	
		if(valid)
			printf("\n Train added successfully\n");

		return valid;	
	}
	
	else if(choice==2){			// View train response
		int no_of_trains;
		int tno;
		char tname[50];
		int tseats;
		int aseats;
		read(socket,&no_of_trains,sizeof(no_of_trains));
		printf("\n\n");
		printf(" T_no\tT_name\tT_seats\tAvail_seats\n");
		while(no_of_trains--){
			read(socket,&tno,sizeof(tno));
			read(socket,&tname,sizeof(tname));
			read(socket,&tseats,sizeof(tseats));
			read(socket,&aseats,sizeof(aseats));
			
			if(strcmp(tname, "deleted")!=0)
				printf(" %d\t%s\t%d\t%d\n",tno,tname,tseats,aseats);
		}

		return valid;	
	}
	
	else if (choice==3){			// Update train response
		int tseats,choice=2,valid=0,tid;
		char tname[50];
		write(socket,&choice,sizeof(int));
		train_modifications(socket,choice);
		printf("\n\n");
		printf("\n  Enter the train number you want to modify: ");
		scanf("%d",&tid);
		write(socket,&tid,sizeof(tid));
		printf("\n\n");
		printf("\n 1. Train Name");
		printf("\n 2. Total Seats\n");
		printf("\n\n");
		printf(" Please select Your Choice: ");
		scanf("%d",&choice);
		write(socket,&choice,sizeof(choice));
		
		if(choice==1){
			read(socket,&tname,sizeof(tname));
			printf("\n  Current name: %s",tname);
			printf("\n  Updated name:");
			scanf("%s",tname);
			write(socket,&tname,sizeof(tname));
		}
		else if(choice==2){
			read(socket,&tseats,sizeof(tseats));
			printf("\n  Current value: %d",tseats);
			printf("\n  Updated value:");
			scanf("%d",&tseats);
			write(socket,&tseats,sizeof(tseats));
		}
		read(socket,&valid,sizeof(valid));
		if(valid)
			printf("\n  Train data updated successfully\n");
		return valid;
	}
	
	else if(choice==4){				// Delete train response
		int choice=2,tid,valid=0;
		write(socket,&choice,sizeof(int));
		train_modifications(socket,choice);
		
		printf("\n  Enter the train number you want to delete: ");
		scanf("%d",&tid);
		write(socket,&tid,sizeof(tid));
		read(socket,&valid,sizeof(valid));
		if(valid)
			printf("\n  Train deleted successfully\n");
		return valid;
	}
	
}


//---------------------------------------------------------------------

//---------------------- User Options & Modificatios (Admin)-----------------------

int user_modifications(int socket,int choice){
	int valid = 0;
	if(choice==1){							// Add user
		int type,id;
		char name[50],password[50];
		printf("\n Enter The Type Of Account:: \n");
		printf(" 1. Agent\n");
		printf(" 2. Customer\n");
		printf(" Please select Your Response: ");
		scanf("%d", &type);
		printf("  User Name: ");
		scanf("%s", name);
		strcpy(password,getpass("  Password: "));
		write(socket, &type, sizeof(type));
		write(socket, &name, sizeof(name));
		write(socket, &password, strlen(password));
		read(socket,&valid,sizeof(valid));	
		if(valid){
			read(socket,&id,sizeof(id));
			printf(" Remember Your login id For Further Logins as :: %d\n", id);
		}
		return valid;	
	}
	
	else if(choice==2){						// View user list
		int no_of_users;
		int id,type;
		char uname[50];
		read(socket,&no_of_users,sizeof(no_of_users));
		printf("\n  User Type :: 0 - Admin    1 - Agent    2 - customer");
		printf("\n  U_id\tU_name\tU_type\n");
		while(no_of_users--){
			read(socket,&id,sizeof(id));
			read(socket,&uname,sizeof(uname));
			read(socket,&type,sizeof(type));
			
			if(strcmp(uname, "deleted")!=0)
				printf("  %d\t%s\t%d\n",id,uname,type);
		}

		return valid;	
	}
	
	else if (choice==3){						// Update user
		int choice=2,valid=0,uid;
		char name[50],pass[50];
		write(socket,&choice,sizeof(int));
		user_modifications(socket,choice);
		printf("\n   Enter the U_id you want to modify: ");
		scanf("%d",&uid);
		write(socket,&uid,sizeof(uid));
		
		printf("\n 1. User Name ");
		printf("\n 2. Password \n");
		printf("\t Your Choice: ");
		scanf("%d",&choice);
		write(socket,&choice,sizeof(choice));
		
		if(choice==1){
			read(socket,&name,sizeof(name));
			printf("\n   Current name: %s",name);
			printf("\n   Updated name:");
			scanf("%s",name);
			write(socket,&name,sizeof(name));
			read(socket,&valid,sizeof(valid));
		}
		else if(choice==2){
			printf("\n  Enter Current password: ");
			scanf("%s",pass);
			write(socket,&pass,sizeof(pass));
			read(socket,&valid,sizeof(valid));
			if(valid){
				printf("\n  Enter new password:");
				scanf("%s",pass);
			}
			else
				printf("\n  Incorrect password\n");
			
			write(socket,&pass,sizeof(pass));
		}
		if(valid){
			read(socket,&valid,sizeof(valid));
			if(valid)
				printf("\n  User data updated successfully\n");
		}
		return valid;
	}
	
	else if(choice==4){						// Delete a user
		int choice=2,uid,valid=0;
		write(socket,&choice,sizeof(int));
		user_modifications(socket,choice);
		
		printf("\n  Enter the id you want to delete: ");
		scanf("%d",&uid);
		write(socket,&uid,sizeof(uid));
		read(socket,&valid,sizeof(valid));
		if(valid)
			printf("\n  User deleted successfully\n");
		return valid;
	}
}


