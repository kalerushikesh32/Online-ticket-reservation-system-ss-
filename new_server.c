#include <sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<stdio.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>


//--------------- structures declaration of train and user------------//
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
void server_cli_service(int socket);
void login_validation(int socket);
void signup_create(int socket);
int login_menu(int socket,int acc_type,int login_id);
int user_options(int socket,int choice,int acc_type,int login_id);
void train_modifications(int socket);
void user_modifications(int socket);

//---------------------------------------------------------------------


// Creating Main with socket connectors initialization and connections
int main(void){

	struct sockaddr_in server, client;
	int sd,nsd,client_len;
	
	sd = socket(AF_INET, SOCK_STREAM,0);

	if (sd == -1) { 
        	printf("Socket creation failed.\n"); 
    	}else
    		printf("Socket created successfully.\n"); 
	
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(8112);
	
	if(bind(sd,(struct sockaddr*)(&server), sizeof(server))<0){
		printf("Error occured while binding. \n");
	}else
		printf("Binded ! \n");
	
	listen(sd,10);
	printf("Listening ! \n");
	
	client_len = sizeof(client);
	
	while(1){
		
		nsd = accept(sd,(struct sockaddr *)&client,&client_len);
		
		if(!fork()){
			close(sd);
			server_cli_service(nsd);                                     // Start servicing client
			exit(0);
		}
		else{
			close(nsd);
		}
	}
	
	//close(nsd);
	//close(sd);
	return(0);	
}

//---------------------------------------------------------------------


// ----------------- Server start servicing client from here. 

void server_cli_service(int socket){
	int choice;
	printf("\n Client [%d] Connected Successfully \n", socket);
	while(1){
		read(socket, &choice, sizeof(int));		
		if(choice==1){
			printf("login \n");
			login_validation(socket);
			}
		if(choice==2){
			printf("signup \n");
			signup_create(socket);
			}
		if(choice==3){
			printf("Exit \n");
			break;
			}
	}

	close(socket);
	printf("\n Client [%d] Disconnected. \n", socket);
}

//---------------------------------------------------------------------

//------------------ New login credentials created with sign up function

void signup_create(int socket){
	int fd_user = open("user_database",O_RDWR);
	int acc_type,login_id=0;
	char name[50],password[50];
	struct user u,temp;

	read(socket, &acc_type, sizeof(acc_type));                  // Read from client
	read(socket, &name, sizeof(name));
	read(socket, &password, sizeof(password));

	int fp = lseek(fd_user, 0, SEEK_END);

	struct flock lock;
	lock.l_type = F_WRLCK;
	lock.l_start = fp;
	lock.l_len = 0;
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();


	fcntl(fd_user,F_SETLKW, &lock);

	// if file is empty, login id will start from 1
	// else it will increment from the previous value
	if(fp==0){
		u.login_id = 1;
		strcpy(u.name, name);
		strcpy(u.password, password);
		u.type=acc_type;
		write(fd_user, &u, sizeof(u));
		write(socket, &u.login_id, sizeof(u.login_id));
	}
	else{
		fp = lseek(fd_user, -1 * sizeof(struct user), SEEK_END);
		read(fd_user, &u, sizeof(u));
		u.login_id++;
		strcpy(u.name, name);
		strcpy(u.password, password);
		u.type=acc_type;
		write(fd_user, &u, sizeof(u));
		write(socket, &u.login_id, sizeof(u.login_id));
	}
	lock.l_type = F_UNLCK;
	fcntl(fd_user, F_SETLK, &lock);

	close(fd_user);
	
}


//------------------ To validate login credentials got from client

void login_validation(int socket){
	int fd_user = open("user_database",O_RDWR);
	int login_id,acc_type,flag=0,user_valid=0;
	char password[50];
	struct user u;
	read(socket,&login_id,sizeof(login_id));             //read from client
	read(socket,&password,sizeof(password));
	
	struct flock lock;                                       // applying lock mechanism
	
	lock.l_start = (login_id-1)*sizeof(struct user);
	lock.l_len = sizeof(struct user);
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();

	lock.l_type = F_WRLCK;
	fcntl(fd_user,F_SETLKW, &lock);
	
	while(read(fd_user,&u,sizeof(u))){
		if(u.login_id==login_id){
			user_valid=1;
			if(!strcmp(u.password,password)){
				flag = 1;
				acc_type = u.type;
				break;
			}
			else{
				flag = 0;
				break;
			}	
		}		
		else{
			user_valid = 0;
			flag=0;
		}
	}
	
	// same agent is allowed from multiple terminals.. 
	// so unlocking his user record just after checking his credentials and allowing further
	if(acc_type!=2){
		lock.l_type = F_UNLCK;
		fcntl(fd_user, F_SETLK, &lock);
		close(fd_user);
	}
	
	// if valid user, show him menu
	if(user_valid)
	{
		write(socket,&flag,sizeof(flag));
		if(flag){
			write(socket,&acc_type,sizeof(acc_type));
			//printf("Successful 1 \n");
			while(login_menu(socket,acc_type,login_id)!=-1);                                   // <------------- going to login menu
		}
	}
	else
		//printf("UnSuccessful 1 \n");
		write(socket,&flag,sizeof(flag));
	
	// same user is not allowed from multiple terminals.. 
	// so unlocking his user record after he logs out only to not allow him from other terminal
	if(acc_type==2){
		lock.l_type = F_UNLCK;
		fcntl(fd_user, F_SETLK, &lock);
		close(fd_user);
	}
} 


//---------------------------------------------------------------------

// ---------------------------- Login Menu comes after successful login --------------
int login_menu(int socket,int acc_type,int login_id){
	int choice,ret;

	// for admin
	if(acc_type==0){
		read(socket,&choice,sizeof(choice));
		if(choice==1){					// Changes on train database
			train_modifications(socket);
			return login_menu(socket,acc_type,login_id);	
		}
		else if(choice==2){				// changes on user database
			user_modifications(socket);
			return login_menu(socket,acc_type,login_id);
		}
		else if (choice ==3)				// Logout
			return -1;
	}
	else if(acc_type==2 || acc_type==1){				// For agent and customer
		read(socket,&choice,sizeof(choice));
		ret = user_options(socket,choice,acc_type,login_id);
		if(ret!=5)
			return login_menu(socket,acc_type,login_id);
		else if(ret==5)
			return -1;
	}
}

//---------------------------------------------------------------------

//---------------------- User Options -----------------------//
int user_options(int socket,int choice,int acc_type,int login_id){
	int valid=0;
	if(choice==1){						// book ticket
		train_modifications(socket);
		struct flock lockt;
		struct flock lockb;
		struct train tdb;
		struct booking bdb;
		int fd_train = open("train_database", O_RDWR);
		int fd_book = open("booking_database", O_RDWR);
		int tid,seats;
		read(socket,&tid,sizeof(tid));		
				
		lockt.l_type = F_WRLCK;
		lockt.l_start = tid*sizeof(struct train);
		lockt.l_len = sizeof(struct train);
		lockt.l_whence = SEEK_SET;
		lockt.l_pid = getpid();
		
		lockb.l_type = F_WRLCK;
		lockb.l_start = 0;
		lockb.l_len = 0;
		lockb.l_whence = SEEK_END;
		lockb.l_pid = getpid();
		
		fcntl(fd_train, F_SETLKW, &lockt);
		lseek(fd_train,tid*sizeof(struct train),SEEK_SET);
		
		read(fd_train,&tdb,sizeof(tdb));
		read(socket,&seats,sizeof(seats));

		if(tdb.train_number==tid)
		{		
			if(tdb.available_seats>=seats){
				valid = 1;
				tdb.available_seats -= seats;
				fcntl(fd_book, F_SETLKW, &lockb);
				int fp = lseek(fd_book, 0, SEEK_END);
				
				if(fp > 0){
					lseek(fd_book, -1*sizeof(struct booking), SEEK_CUR);
					read(fd_book, &bdb, sizeof(struct booking));
					bdb.booking_id++;
				}
				else 
					bdb.booking_id = 0;

				bdb.type = acc_type;
				bdb.uid = login_id;
				bdb.tid = tid;
				bdb.seats = seats;
				write(fd_book, &bdb, sizeof(struct booking));
				lockb.l_type = F_UNLCK;
				fcntl(fd_book, F_SETLK, &lockb);
			 	close(fd_book);
			}
		
		lseek(fd_train, -1*sizeof(struct train), SEEK_CUR);
		write(fd_train, &tdb, sizeof(tdb));
		}

		lockt.l_type = F_UNLCK;
		fcntl(fd_train, F_SETLK, &lockt);
		close(fd_train);
		write(socket,&valid,sizeof(valid));
		return valid;		
	}
	
	else if(choice==2){							// View bookings
		struct flock lock;
		struct booking bdb;
		int fd_book = open("booking_database", O_RDONLY);
		int no_of_bookings = 0;
	
		lock.l_type = F_RDLCK;
		lock.l_start = 0;
		lock.l_len = 0;
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();
		
		fcntl(fd_book, F_SETLKW, &lock);
	
		while(read(fd_book,&bdb,sizeof(bdb))){
			if (bdb.uid==login_id)
				no_of_bookings++;
		}

		write(socket, &no_of_bookings, sizeof(int));
		lseek(fd_book,0,SEEK_SET);

		while(read(fd_book,&bdb,sizeof(bdb))){
			if(bdb.uid==login_id){
				write(socket,&bdb.booking_id,sizeof(int));
				write(socket,&bdb.tid,sizeof(int));
				write(socket,&bdb.seats,sizeof(int));
			}
		}
		lock.l_type = F_UNLCK;
		fcntl(fd_book, F_SETLK, &lock);
		close(fd_book);
		return valid;
	}
	
	else if (choice==3){							// update booking
		int choice = 2,bid,val;
		user_options(socket,choice,acc_type,login_id);
		struct booking bdb;
		struct train tdb;
		struct flock lockb;
		struct flock lockt;
		int fd_book = open("booking_database", O_RDWR);
		int fd_train = open("train_database", O_RDWR);
		read(socket,&bid,sizeof(bid));

		lockb.l_type = F_WRLCK;
		lockb.l_start = bid*sizeof(struct booking);
		lockb.l_len = sizeof(struct booking);
		lockb.l_whence = SEEK_SET;
		lockb.l_pid = getpid();
		
		fcntl(fd_book, F_SETLKW, &lockb);
		lseek(fd_book,bid*sizeof(struct booking),SEEK_SET);
		read(fd_book,&bdb,sizeof(bdb));
		lseek(fd_book,-1*sizeof(struct booking),SEEK_CUR);
		
		lockt.l_type = F_WRLCK;
		lockt.l_start = (bdb.tid)*sizeof(struct train);
		lockt.l_len = sizeof(struct train);
		lockt.l_whence = SEEK_SET;
		lockt.l_pid = getpid();

		fcntl(fd_train, F_SETLKW, &lockt);
		lseek(fd_train,(bdb.tid)*sizeof(struct train),SEEK_SET);
		read(fd_train,&tdb,sizeof(tdb));
		lseek(fd_train,-1*sizeof(struct train),SEEK_CUR);

		read(socket,&choice,sizeof(choice));
	
		if(choice==1){							// increase number of seats required of booking id
			read(socket,&val,sizeof(val));
			if(tdb.available_seats>=val){
				valid=1;
				tdb.available_seats -= val;
				bdb.seats += val;
			}
		}
		else if(choice==2){						// decrease number of seats required of booking id
			valid=1;
			read(socket,&val,sizeof(val));
			tdb.available_seats += val;
			bdb.seats -= val;	
		}
		
		write(fd_train,&tdb,sizeof(tdb));
		lockt.l_type = F_UNLCK;
		fcntl(fd_train, F_SETLK, &lockt);
		close(fd_train);
		
		write(fd_book,&bdb,sizeof(bdb));
		lockb.l_type = F_UNLCK;
		fcntl(fd_book, F_SETLK, &lockb);
		close(fd_book);
		
		write(socket,&valid,sizeof(valid));
		return valid;
	}
	
	else if(choice==4){							// Cancel an entire booking
		int choice = 2,bid;
		user_options(socket,choice,acc_type,login_id);
		struct booking bdb;
		struct train tdb;
		struct flock lockb;
		struct flock lockt;
		int fd_book = open("booking_database", O_RDWR);
		int fd_train = open("train_database", O_RDWR);
		read(socket,&bid,sizeof(bid));

		lockb.l_type = F_WRLCK;
		lockb.l_start = bid*sizeof(struct booking);
		lockb.l_len = sizeof(struct booking);
		lockb.l_whence = SEEK_SET;
		lockb.l_pid = getpid();
		
		fcntl(fd_book, F_SETLKW, &lockb);
		lseek(fd_book,bid*sizeof(struct booking),SEEK_SET);
		read(fd_book,&bdb,sizeof(bdb));
		lseek(fd_book,-1*sizeof(struct booking),SEEK_CUR);
		
		lockt.l_type = F_WRLCK;
		lockt.l_start = (bdb.tid)*sizeof(struct train);
		lockt.l_len = sizeof(struct train);
		lockt.l_whence = SEEK_SET;
		lockt.l_pid = getpid();

		fcntl(fd_train, F_SETLKW, &lockt);
		lseek(fd_train,(bdb.tid)*sizeof(struct train),SEEK_SET);
		read(fd_train,&tdb,sizeof(tdb));
		lseek(fd_train,-1*sizeof(struct train),SEEK_CUR);

		tdb.available_seats += bdb.seats;
		bdb.seats = 0;
		valid = 1;

		write(fd_train,&tdb,sizeof(tdb));
		lockt.l_type = F_UNLCK;
		fcntl(fd_train, F_SETLK, &lockt);
		close(fd_train);
		
		write(fd_book,&bdb,sizeof(bdb));
		lockb.l_type = F_UNLCK;
		fcntl(fd_book, F_SETLK, &lockb);
		close(fd_book);
		
		write(socket,&valid,sizeof(valid));
		return valid;
		
	}
	else if(choice==5)										// Logout
		return 5;
	
}


//---------------------------------------------------------------------


//---------------------- Train Options & Modificatios -----------------------

void train_modifications(int socket){
	int valid=0;	
	int choice;
	read(socket,&choice,sizeof(choice));
	if(choice==1){  					// Add train  	
		char tname[50];
		int tid = 0;
		read(socket,&tname,sizeof(tname));
		struct train tdb,temp;
		struct flock lock;
		int fd_train = open("train_database", O_RDWR);
		
		tdb.train_number = tid;
		strcpy(tdb.train_name,tname);
		tdb.total_seats = 50;				// by default, we are taking 50 seats
		tdb.available_seats = 50;

		int fp = lseek(fd_train, 0, SEEK_END); 

		lock.l_type = F_WRLCK;
		lock.l_start = fp;
		lock.l_len = 0;
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();

		fcntl(fd_train, F_SETLKW, &lock);

		if(fp == 0){
			valid = 1;
			write(fd_train, &tdb, sizeof(tdb));
			lock.l_type = F_UNLCK;
			fcntl(fd_train, F_SETLK, &lock);
			close(fd_train);
			write(socket, &valid, sizeof(valid));
		}
		else{
			valid = 1;
			lseek(fd_train, -1 * sizeof(struct train), SEEK_END);
			read(fd_train, &temp, sizeof(temp));
			tdb.train_number = temp.train_number + 1;
			write(fd_train, &tdb, sizeof(tdb));
			write(socket, &valid,sizeof(valid));	
		}
		lock.l_type = F_UNLCK;
		fcntl(fd_train, F_SETLK, &lock);
		close(fd_train);
		
	}
	
	else if(choice==2){					// View/ Read trains
		struct flock lock;
		struct train tdb;
		int fd_train = open("train_database", O_RDONLY);
		
		lock.l_type = F_RDLCK;
		lock.l_start = 0;
		lock.l_len = 0;
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();
		
		fcntl(fd_train, F_SETLKW, &lock);
		int fp = lseek(fd_train, 0, SEEK_END);
		int no_of_trains = fp / sizeof(struct train);
		write(socket, &no_of_trains, sizeof(int));

		lseek(fd_train,0,SEEK_SET);
		while(fp != lseek(fd_train,0,SEEK_CUR)){
			read(fd_train,&tdb,sizeof(tdb));
			write(socket,&tdb.train_number,sizeof(int));
			write(socket,&tdb.train_name,sizeof(tdb.train_name));
			write(socket,&tdb.total_seats,sizeof(int));
			write(socket,&tdb.available_seats,sizeof(int));
		}
		valid = 1;
		lock.l_type = F_UNLCK;
		fcntl(fd_train, F_SETLK, &lock);
		close(fd_train);
	}
	
	else if(choice==3){					// Update train
		train_modifications(socket);
		int choice,valid=0,tid;
		struct flock lock;
		struct train tdb;
		int fd_train = open("train_database", O_RDWR);

		read(socket,&tid,sizeof(tid));

		lock.l_type = F_WRLCK;
		lock.l_start = (tid)*sizeof(struct train);
		lock.l_len = sizeof(struct train);
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();
		
		fcntl(fd_train, F_SETLKW, &lock);

		lseek(fd_train, 0, SEEK_SET);
		lseek(fd_train, (tid)*sizeof(struct train), SEEK_CUR);
		read(fd_train, &tdb, sizeof(struct train));
		
		read(socket,&choice,sizeof(int));
		if(choice==1){							// update train name
			write(socket,&tdb.train_name,sizeof(tdb.train_name));
			read(socket,&tdb.train_name,sizeof(tdb.train_name));
			
		}
		else if(choice==2){						// update total number of seats
			write(socket,&tdb.total_seats,sizeof(tdb.total_seats));
			read(socket,&tdb.total_seats,sizeof(tdb.total_seats));
		}
	
		lseek(fd_train, -1*sizeof(struct train), SEEK_CUR);
		write(fd_train, &tdb, sizeof(struct train));
		valid=1;
		write(socket,&valid,sizeof(valid));
		lock.l_type = F_UNLCK;
		fcntl(fd_train, F_SETLK, &lock);
		close(fd_train);	
	}
	
	else if(choice==4){						// Delete train
		train_modifications(socket);
		struct flock lock;
		struct train tdb;
		int fd_train = open("train_database", O_RDWR);
		int tid,valid=0;

		read(socket,&tid,sizeof(tid));

		lock.l_type = F_WRLCK;
		lock.l_start = (tid)*sizeof(struct train);
		lock.l_len = sizeof(struct train);
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();
		
		fcntl(fd_train, F_SETLKW, &lock);
		
		lseek(fd_train, 0, SEEK_SET);
		lseek(fd_train, (tid)*sizeof(struct train), SEEK_CUR);
		read(fd_train, &tdb, sizeof(struct train));
		strcpy(tdb.train_name,"deleted");
		lseek(fd_train, -1*sizeof(struct train), SEEK_CUR);
		write(fd_train, &tdb, sizeof(struct train));
		valid=1;
		write(socket,&valid,sizeof(valid));
		lock.l_type = F_UNLCK;
		fcntl(fd_train, F_SETLK, &lock);
		close(fd_train);	
	}	
	
}

//---------------------------------------------------------------------

//---------------------- User Options & Modificatios -----------------------


void user_modifications(int socket){
	int valid=0;	
	int choice;
	read(socket,&choice,sizeof(choice));
	if(choice==1){    					// Add user
		char name[50],password[50];
		int type;
		read(socket, &type, sizeof(type));
		read(socket, &name, sizeof(name));
		read(socket, &password, sizeof(password));
		
		struct user udb;
		struct flock lock;
		int fd_user = open("user_database", O_RDWR);
		int fp = lseek(fd_user, 0, SEEK_END);
		
		lock.l_type = F_WRLCK;
		lock.l_start = fp;
		lock.l_len = 0;
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();

		fcntl(fd_user, F_SETLKW, &lock);

		if(fp==0){
			udb.login_id = 1;
			strcpy(udb.name, name);
			strcpy(udb.password, password);
			udb.type=type;
			write(fd_user, &udb, sizeof(udb));
			valid = 1;
			write(socket,&valid,sizeof(int));
			write(socket, &udb.login_id, sizeof(udb.login_id));
		}
		else{
			fp = lseek(fd_user, -1 * sizeof(struct user), SEEK_END);
			read(fd_user, &udb, sizeof(udb));
			udb.login_id++;
			strcpy(udb.name, name);
			strcpy(udb.password, password);
			udb.type=type;
			write(fd_user, &udb, sizeof(udb));
			valid = 1;
			write(socket,&valid,sizeof(int));
			write(socket, &udb.login_id, sizeof(udb.login_id));
		}
		lock.l_type = F_UNLCK;
		fcntl(fd_user, F_SETLK, &lock);
		close(fd_user);
		
	}
	
	else if(choice==2){					// View user list
		struct flock lock;
		struct user udb;
		int fd_user = open("user_database", O_RDONLY);
		
		lock.l_type = F_RDLCK;
		lock.l_start = 0;
		lock.l_len = 0;
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();
		
		fcntl(fd_user, F_SETLKW, &lock);
		int fp = lseek(fd_user, 0, SEEK_END);
		int no_of_users = fp / sizeof(struct user);
		no_of_users--;
		write(socket, &no_of_users, sizeof(int));

		lseek(fd_user,0,SEEK_SET);
		while(fp != lseek(fd_user,0,SEEK_CUR)){
			read(fd_user,&udb,sizeof(udb));
			if(udb.type!=0){
				write(socket,&udb.login_id,sizeof(int));
				write(socket,&udb.name,sizeof(udb.name));
				write(socket,&udb.type,sizeof(int));
			}
		}
		valid = 1;
		lock.l_type = F_UNLCK;
		fcntl(fd_user, F_SETLK, &lock);
		close(fd_user);
	}
	
	else if(choice==3){					// Update user
		user_modifications(socket);
		int choice,valid=0,uid;
		char pass[50];
		struct flock lock;
		struct user udb;
		int fd_user = open("user_database", O_RDWR);

		read(socket,&uid,sizeof(uid));

		lock.l_type = F_WRLCK;
		lock.l_start =  (uid-1)*sizeof(struct user);
		lock.l_len = sizeof(struct user);
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();
		
		fcntl(fd_user, F_SETLKW, &lock);

		lseek(fd_user, 0, SEEK_SET);
		lseek(fd_user, (uid-1)*sizeof(struct user), SEEK_CUR);
		read(fd_user, &udb, sizeof(struct user));
		
		read(socket,&choice,sizeof(int));
		if(choice==1){					// update name
			write(socket,&udb.name,sizeof(udb.name));
			read(socket,&udb.name,sizeof(udb.name));
			valid=1;
			write(socket,&valid,sizeof(valid));		
		}
		else if(choice==2){				// update password
			read(socket,&pass,sizeof(pass));
			if(!strcmp(udb.password,pass))
				valid = 1;
			write(socket,&valid,sizeof(valid));
			read(socket,&udb.password,sizeof(udb.password));
		}
	
		lseek(fd_user, -1*sizeof(struct user), SEEK_CUR);
		write(fd_user, &udb, sizeof(struct user));
		if(valid)
			write(socket,&valid,sizeof(valid));
		lock.l_type = F_UNLCK;
		fcntl(fd_user, F_SETLK, &lock);
		close(fd_user);	
	}
	
	else if(choice==4){						// Delete any particular user
		user_modifications(socket);
		struct flock lock;
		struct user udb;
		int fd_user = open("user_database", O_RDWR);
		int uid,valid=0;

		read(socket,&uid,sizeof(uid));

		lock.l_type = F_WRLCK;
		lock.l_start =  (uid-1)*sizeof(struct user);
		lock.l_len = sizeof(struct user);
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();
		
		fcntl(fd_user, F_SETLKW, &lock);
		
		lseek(fd_user, 0, SEEK_SET);
		lseek(fd_user, (uid-1)*sizeof(struct user), SEEK_CUR);
		read(fd_user, &udb, sizeof(struct user));
		strcpy(udb.name,"deleted");
		strcpy(udb.password,"");
		lseek(fd_user, -1*sizeof(struct user), SEEK_CUR);
		write(fd_user, &udb, sizeof(struct user));
		valid=1;
		write(socket,&valid,sizeof(valid));
		lock.l_type = F_UNLCK;
		fcntl(fd_user, F_SETLK, &lock);
		close(fd_user);	
	}
}











