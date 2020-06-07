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
#define true 1
#define false 0

//함수
void * handle_clnt(void * arg); //read 한 후 send_mssg 함수(wirte하는 함수) 호출 -> 쓰레드 
void send_msg(char * msg, int len); //서버단에서 클라이언트에게 메시지를 보낼 때 호출되는 함수
void error_handling(char * msg);
void start_game(); //게임이 시작됐을 때 호출되는 함수
void* change_line(void *arg); //게임 문제(노선)를 바꾸는 함수  -> 쓰레드
void download_api(int line); //필요한 api를 다운로드 받는 함수 
void decide_turn(); //라운드마다 사용자의 순서를 정하는 함수 (구현x)
int check_answer(char* msg); //사용자가 입력한 역이름이 노선에 해당하는지 체크 (구현 중)


//전역변수 
int clnt_cnt=0; //접속한 클라이언트 수 카운팅
int clnt_socks[MAX_CLNT]; //통신할 클라이언트 소켓들을 담는 배열
pthread_mutex_t mutx; // 동기화 기법: mutex
int round_cnt = 1; //라운드1(2호선)부터 시작, 10문제 정답을 기준으로 게임 라운드(문제) 바뀜(2호선 -> 1호선 -> 4호선) 
int answer_cnt = 0; //사용자가 맞춘 답의 개수 카운팅 

//메인
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
  
	pthread_mutex_init(&mutx, NULL); // 뮤텍스 초기화 
	serv_sock=socket(PF_INET, SOCK_STREAM, 0); // <socket 생성>: TCP 소켓을 사용

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET; 
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1) // <bind> 
		error_handling("bind() error");
	if(listen(serv_sock, 5)==-1) // <listen> 
		error_handling("listen() error");
	
	pthread_create(&t_id, NULL, change_line, NULL); //change_line 쓰레드 
	pthread_detach(t_id);

	while(1) //main 문 안 끝나게 -> 메인 쓰레드가 끝나면 다른 쓰레드들도 다 종료되므로  
	{	
		// 사용자를 3명만 받게 함 -> 3명의 클라이언트만 accept 해야 됨, 안 그러면 connect 되버림
		if(clnt_cnt<=2){
			clnt_adr_sz=sizeof(clnt_adr);
			clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr,&clnt_adr_sz); //<accept> : 접속하는 클라이언트를의 연결을 허용하는 부분
		
			pthread_mutex_lock(&mutx); 
			clnt_socks[clnt_cnt++]=clnt_sock; //임계영역
			pthread_mutex_unlock(&mutx); 
	
			pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock); //handle_clnt 쓰레드
			pthread_detach(t_id);
			printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));
			start_game(); // 3명의 클라이언트가 들어왔을 때, 게임 시작 함수 호출
		}

	}


	close(serv_sock);
	return 0;
}


void start_game(){

	if(clnt_cnt == 3){
			
			char msg1[50] = "3명이 모두 들어왔습니다.\n";
			send_msg(msg1, 50);

			sleep(1);

			char msg2[50] = "지하철 게임을 시작하겠습니다.\n\n";
			send_msg(msg2, 50);

			sleep(2);
			
			char msg3[1000] = "**********************************************************************\n**		    	Open API를 활용한 			    **\n**		    '노선별 지하철역 암기 게임' 	       	    **\n**********************************************************************\n";
			send_msg(msg3, 1000);
			
			sleep(2);

			char msg4[2000] = "<< 게임 규칙 >>\n1. 매번 문제가 바뀔 때마다, 임의로 순서가 정해집니다.\n2. 자신의 순번일 때만 답을 입력할 수 있습니다.\n3. 다른 사용자가 말한 역을 입력할 수 없습니다.\n4. 역이름이 기억나지 않는다면 'pass'를 입력하세요.\n5. 3명의 사용자가 총 10개의 역이름을 맞추면 다음 문제로 넘어갑니다.\n6. 문제는 2호선 -> 1호선 -> 4호선 순서로 반복됩니다.\n\n";
			send_msg(msg4, 2000);

			download_api(2);
			char msg5[40] = "이번 문제는 2호선입니다.\n";
			send_msg(msg5, 40);
			sleep(1);
			
	}


}

//라운드를 바꿈, 문제를 바꾸는 함수
void* change_line(void *arg){

	/* 순서 정할 때, decide_turn() 함수 호출하기 
	순서를 정하겠습니다. 
	두구두구두구.....
	sleep(3) 
	순서는 [ 뿡뿡이]-> [깜찌기] -> [jsk] 입니다.
	2호선 역을 말해주세요
	*/

	char* msg;

	//1호선, 2호선, 4호선 api를 다운받았는지 여부
	int download_line4 = false; int download_line2 = false; int download_line1 = false; 

	while(1){ //쓰레드가 끝나지 않게 하기 위해 

		
		if(answer_cnt == 10 ) { //10개 맞출 때마다

			
			char msg1[100] = "10개의 역을 모두 맞추셨습니다.\n";
			send_msg(msg1, 100);	
			sleep(1);
			round_cnt++;
			answer_cnt = 0; //다음 라운드를 위해 초기화

			//2호선 -> 1호선 -> 4호선 -> 2호선 -> 1호선 -> 4호선  순서로 문제를 냄 
			if(round_cnt %3 == 0){

				if(download_line4 == false){ //api 다운 안 받았으면, 한 번만 다운받기
					download_api(4);
					download_line4 = true;
				}
				
				msg = "다음 라운드는 4호선입니다.\n";
				send_msg(msg, 100);
				sleep(1);
			

			}else if(round_cnt %3 == 1 ){
				/* 처음 시작할 때 2호선 문제를 다운받고 시작하므로.
				if(download_line2 == false){
					download_api(2);
					download_line2 = true;
				}
				*/

				msg = "다음 라운드는 2호선입니다.\n";
				send_msg(msg, 100);
				sleep(1);
			
			}else if(round_cnt %3 == 2 ){
			
				if(download_line1 == false){
					download_api(1);
					download_line1 = true;
				}

				msg = "다음 라운드는 1호선입니다.\n";
				send_msg(msg, 100);
				sleep(1);
			
			}

		}// end -> if(answer_cnt == 10 ){

	}//end while

}

//사용자가 입력한 답이 맞는지 체크하는 기능
int check_answer(char* msg){ //어떤 파일인지도 인자로 넘겨줘야 함.. 현재 
	
	FILE* fp;
	char buffer[300];
	char* fname;

	if(round_cnt %3 == 0){
		fname = "4호선";
	}else if(round_cnt %3 == 1){
		fname = "2호선";
	}else if(round_cnt %3 == 2){
		fname = "1호선";
	}


	// 주의: 메시지가 전달될 때 "[jsk] 대림\n" 식으로 전달됨.
	char* ptr = strtok(msg, " "); //1차: " " 공백 문자를 기준으로 문자열 자름
	ptr = strtok(NULL, "\n"); //2차: "\n" 엔터키에 NULL을 넣고 자름

	msg = ptr;


	fp = fopen(fname, "r"); 
	if(fp == NULL){
		fprintf(stderr, "%s 파일을 열 수 없습니다. \n", fname);
		exit(1);
	}

	while(fgets(buffer, 300, fp)){
		
		if(strstr(buffer, msg)){ //문자열을 찾아주는 함수
			return true; //단어를 발견하면 true 값(1)을 반환
			break;
		}
	}

	fclose(fp);

	return false;

}


//필요한 노선을 다운로드 받는 api 
void download_api(int line){

	if(line == 4){
		//http 요청 url의 형식: 4호선 정보 1행부터 48행까지 출력, json 형태 파일로 저장	
		system("wget http://openapi.seoul.go.kr:8088/4868476866736f6f3637734a537279/json/SearchSTNBySubwayLineInfo/1/48/%20/%20/4호선");

	}else if(line == 2){
		
		system("wget http://openapi.seoul.go.kr:8088/4868476866736f6f3637734a537279/json/SearchSTNBySubwayLineInfo/1/78/%20/%20/2호선");

	}else if(line == 1){

		system("wget http://openapi.seoul.go.kr:8088/4868476866736f6f3637734a537279/json/SearchSTNBySubwayLineInfo/1/98/%20/%20/1호선");
		
	}//end else if

}



//쓰레드로서 실시간으로 돌아감.	
void * handle_clnt(void * arg)
{

	int clnt_sock=*((int*)arg);
	int str_len=0, i;
	char msg[BUF_SIZE];
	char* msg1;

	//① 클라이언트로부터 수신된 메시지를 모든 클라이언트에게 전달하는 코드
	while((str_len=read(clnt_sock, msg, sizeof(msg)))!=0){ //쓰레드가 끝나지 않도록 while문
		send_msg(msg, str_len); 
		//printf("안녕하세요%s안녕하세요\n", msg);		
		sleep(1);

		//답 체크
		if(check_answer(msg) == true){ //<error>여기엔 문제 없음...msg에 값이 안 들어가거나 이상한 값이 들어가 있음
			msg1 = "정답입니다.\n";
			send_msg(msg1,20);
			answer_cnt++;
		}else{
			msg1 = "틀렸습니다.\n"; //<수정> OO역은 OO노선에 해당하지 않습니다.
			send_msg(msg1,20);
		}
	}
	
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


