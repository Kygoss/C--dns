#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>     


#define BUF_SIZE 4096


char buffer_receive[BUF_SIZE];
char buffer_send[BUF_SIZE];


int my_send(int socket_send);
int my_receive();


struct DNS_Header{
	unsigned short id: 16;
	unsigned short tag: 16;
	unsigned short queryNum: 16;
	unsigned short answerNum: 16;
	unsigned short authorNum: 16;
	unsigned short addNum: 16;
};

struct DNS_Query{
	unsigned short qtype: 16;
	unsigned short qclass: 16;
};

struct DNS_RR {
	unsigned char *name;
	unsigned short type;
	unsigned short _class;
	unsigned int ttl;
	unsigned short data_len;
	unsigned char *rdata;
};



int main(int argc, char *argv[]){

	printf("Start main...\n");
	int socket_tcp;

	
	if((socket_tcp = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("socket() failed");
		exit(EXIT_FAILURE);
	}


	my_receive();


}


int my_receive(){
	int socket_udp;

	if((socket_udp = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
		perror("socket() failed");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in recv_addr;
	memset(&recv_addr, 0, sizeof(recv_addr));//初始化结构体中的数据
	recv_addr.sin_family = AF_INET; 
	recv_addr.sin_port = htons(53); //htons 转换为网络字节序（大端序）
	recv_addr.sin_addr.s_addr = inet_addr("127.1.1.1"); 

	printf("Start bind...\n");
	if (bind(socket_udp, (struct sockaddr*)&recv_addr, sizeof(recv_addr)) == -1) {
		perror("bind() failed\n");
		exit(EXIT_FAILURE);
	}
	socklen_t addrlen = sizeof(recv_addr);

	while(1){
		printf("Start receiving...\n");
		int received_bytes = recvfrom(socket_udp, buffer_receive, BUF_SIZE, 0, (struct sockaddr*)&recv_addr, &addrlen);
		buffer_receive[received_bytes] = '\0'; // C语言中，字符串是以空字符结尾的字符序列

		printf("received from %s:%d\n", inet_ntoa(recv_addr.sin_addr), ntohs(recv_addr.sin_port)); //inet_ntoa：地址转成xxx.xxx.xxx.xxx格式 //ntohs：转小端序

		struct DNS_Header *dnsheader = (struct DNS_Header *)buffer_receive;
		printf("%d\n",ntohs(dnsheader->id));
		char *name_start = buffer_receive+sizeof(*dnsheader);
		char *name_end;
		name_end = strchr(&buffer_receive[sizeof(*dnsheader)], '\0');
		int name_len = name_end - name_start;
		printf("Name_len: %d\n", name_len);
		char name_str[name_len];
		for (int i=0; i<name_len; i++) {
			name_str[i] = *(name_start+i);
			if(isalpha(name_str[i])!=0){
				printf("%c\n", name_str[i]);
			}
		}
		struct DNS_Query *dnsquery = (struct DNS_Query *)(buffer_receive+sizeof(*dnsheader)+name_len+1);
		printf("Qtype: %d\n", ntohs(dnsquery->qtype));
		
	};
	
	close(socket_udp);

	return 1;
}


int my_send(int socket){

	printf("Create DNS Header...\n");
	struct DNS_Header dns_header;
	memset(&dns_header, 0, sizeof(dns_header));
	dns_header.id = htons(5555); // 设置标识符
	dns_header.tag = htons(0x0100); // 设置标志位，表示这是一个标准查询
	dns_header.queryNum = htons(1); // 问题数为1
	//...

	printf("Create DNS query...\n");
	struct DNS_Query dns_query;
	memset(&dns_query, 0, sizeof(dns_query));
	dns_query.qclass = htons(111);
	//...

	printf("Create DNS packet...\n");
	memset(buffer_send, 0, sizeof(buffer_send));
	memcpy(buffer_send, &dns_header, sizeof(dns_header)); // 拷贝头部
	int position = sizeof(dns_header);
	memcpy(&buffer_send[position], &dns_query, sizeof(dns_query));
	position += sizeof(dns_query);
	buffer_send[position] = '\0';

	// 打印生成的 DNS 报文
	for(int i = 0; i < position; i++){
	    printf("%02x", buffer_send[i]);
	}


	printf("Create (sockaddr_in)send_addr...\n");
	struct sockaddr_in send_addr;
	memset(&send_addr, 0, sizeof(send_addr));//初始化结构体中的数据
	send_addr.sin_family = AF_INET; 
	send_addr.sin_port = htons(8888); 
	send_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); 

	printf("Start Send...\n");
	sendto(socket, buffer_send, sizeof(buffer_send), 0, (struct sockaddr*)&send_addr, sizeof(send_addr));

	return 1;

}
