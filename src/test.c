#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define true 1
#define false 0

int check_answer(char* msg);

int main(void) {


	/*
	FILE* fp;
	char buffer[300];
	char fname[20] = "2호선";
	char word[20] = "잠실";

	fp = fopen(fname, "r"); 
	if(fp == NULL){
		fprintf(stderr, "%s 파일을 열 수 없습니다. \n", fname);
		exit(1);
	}

	while(fgets(buffer, 300, fp)){
		
		if(strstr(buffer, word)){ //문자열을 찾아주는 함수
			printf("%s단어 발견!\n", word);
			break;
		}
	}

	fclose(fp);
	*/

	char msg[200] = "없는 역";

	if(check_answer(msg)){ 
		//msg1 = "정답입니다.\n";
		//send_msg(msg1,20);
		//answer_cnt++;
		printf("정답입니다.\n");

	}else{
		//msg1 = "틀렸습니다.\n"; //OO역은 OO노선에 해당하지 않습니다.
		//send_msg(msg1,20);
		printf("틀렸습니다.\n");
	}
	
	return 0;
}



//사용자가 입력한 답이 맞는지 체크하는 기능
int check_answer(char* msg){ //어떤 파일인지도 인자로 넘겨줘야 함.. 현재 
	
	FILE* fp;
	char buffer[300];
	char fname[20] = "2호선";
	//char word[20] = "잠실";

	fp = fopen(fname, "r"); 
	if(fp == NULL){
		fprintf(stderr, "%s 파일을 열 수 없습니다. \n", fname);
		exit(1);
	}

	while(fgets(buffer, 300, fp)){
		
		if(strstr(buffer, msg)){ //문자열을 찾아주는 함수
			//printf("%s단어 발견!\n", word);
			return true; //단어를 발견하면 true 값(1)을 반환
		}
	}

	fclose(fp);

	return false;

}