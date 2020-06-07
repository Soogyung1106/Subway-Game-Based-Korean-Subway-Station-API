#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define true 1
#define false 0



int main(void) {

	// 주의: 메시지가 전달될 때 "[jsk] 대림" 식으로 전달됨.
	char* msg = "[jsk] 대림";

	//1차
	char* ptr = strtok(msg, " "); //" " 공백 문자를 기준으로 문자열 자름

	//2차
	ptr = strtok(NULL, " ");

	//확인
	printf("%s 안녕하세요\n", ptr);
	msg = ptr;
	printf("%s 안녕하세요\n", msg);
	
	return 0;
}


