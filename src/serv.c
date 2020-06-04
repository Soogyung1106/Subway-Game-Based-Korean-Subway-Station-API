#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUF_SIZE 100
#define MAX_CLNT 3 

//함수
void * handle_clnt(void * arg); //read 한 후 send_mssg 함수(wirte하는 함수) 호출 
void send_msg(char * msg, int len); 
void error_handling(char * msg);
void start_game(); //게임을 시작

//전역변수 
int clnt_cnt=0; //접속한 클라이언트 수 카운팅
int clnt_socks[MAX_CLNT]; //통신할 클라이언트 소켓들을 담는 배열
pthread_mutex_t mutx; // 동기화 기법: mutex

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	pthread_t t_id;
	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
  
	pthread_mutex_init(&mutx, NULL); // 뮤텍스 초기화 ①
	serv_sock=socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET; 
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");
	
	//pthread_create(&t_id, NULL, start_game, (void*)&clnt_sock); //쓰레드 생성 ①
	//pthread_detach(t_id);

	while(1)
	{	
		
			
		clnt_adr_sz=sizeof(clnt_adr);
		clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr,&clnt_adr_sz);
		
		pthread_mutex_lock(&mutx); 
		clnt_socks[clnt_cnt++]=clnt_sock; //임계영역
		pthread_mutex_unlock(&mutx); 
	
		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
		pthread_detach(t_id);
		printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));
		start_game(); // 게임 시작 함수 호출

	}
	close(serv_sock);
	return 0;
}

void start_game(){

	if(clnt_cnt == 3){
			
			/*			
			*************************************************
			**		    	Open API를 활용한 			    **
			**		    '노선별 지하철역 암기 게임' 		 **
			*************************************************
			*/

			char msg1[1000] = "3명이 모두 들어왔습니다.\n지하철 게임을 시작하겠습니다.\n**********************************************************************\n**		    	Open API를 활용한 			    **\n**		    '노선별 지하철역 암기 게임' 	       	    **\n**********************************************************************\n";
			send_msg(msg1, 1000);
			
			/*
			순서를 정하겠습니다. 
두구두구두구.....
sleep(3) 
순서는 [ 뿡뿡이]-> [깜찌기] -> [jsk] 입니다.
2호선 역을 말해주세요
			*/

	}


}



//쓰레드로서 실시간으로 돌아감.	
void * handle_clnt(void * arg)
{
	int clnt_sock=*((int*)arg);
	int str_len=0, i;
	char msg[BUF_SIZE];

	//① 클라이언트가 보낸 메시지 전달	
	while((str_len=read(clnt_sock, msg, sizeof(msg)))!=0) //클라이언트A에서 보낸 것을 읽어서 B와 C에 보내기
		send_msg(msg, str_len);
	
	//② 사후처리
	pthread_mutex_lock(&mutx);  
	for(i=0; i<clnt_cnt; i++)   // remove disconnected client
	{
		if(clnt_sock==clnt_socks[i])
		{
			while(i++<clnt_cnt-1)
				clnt_socks[i]=clnt_socks[i+1]; //임계영역(클라이언트 사후처리)
			break;
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mutx);
	close(clnt_sock);

	return NULL;
}

//쓰레드 내에서 호출됨 
void send_msg(char * msg, int len)   // send to all
{
	int i;
	pthread_mutex_lock(&mutx); 
	for(i=0; i<clnt_cnt; i++) 	
		write(clnt_socks[i], msg, len); //임계 영역
	pthread_mutex_unlock(&mutx);
}
void error_handling(char * msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
