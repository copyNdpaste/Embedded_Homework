#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <mysql/mysql.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#define MAXTIMINGS 85
#define RETRY 5
#define FAN 22 // BCM_GPIO 6
#define RGBLEDPOWER  24 //BCM_GPIO 19
#define RED 7
#define GREEN 9
#define BLUE 8
#define CS_MCP3208 11 //GPIO 8
#define SPI_CHANNEL 0
#define SPI_SPEED 1000000
#define MAX 1
int dark = 2000;
int buffer1[MAX], buffer2[MAX];
int fill_ptr = 0;
int use_ptr1 = 0, use_ptr2 = 0;
int count1 = 0, count2 = 0;
int ret_temp;
static int DHTPIN = 11;
static int dht22_dat[5] = { 0,0,0,0,0 };
int read_dht22_dat_temp();
int get_temperature_sensor();
int get_light_sensor();
void get1();
int adcValue_light = 0;
int adcChannel_light = 0;
int temp, light;
int loop = 1000000;
void sig_handler(int signo);
void db();
char query[1024];
#define DBHOST "localhost"//cloud IP
//#define DBHOST "115.68.232.116" //로컬과 클라우드 서버 모두 연동이 안됩니다
#define DBUSER "root"
#define DBPASS "root"
#define DBNAME "demofarmdb"
MYSQL *connector;
MYSQL_RES *result;
MYSQL_ROW row;
void put(int t, int l) { //for temp storing
	printf("put\n");
	buffer1[fill_ptr] = t;
	printf("%d %u\n", t, l);
	buffer2[fill_ptr] = l;
	fill_ptr = (fill_ptr + 1) % MAX;
	count1++;
}
void get1() { //각각의 버퍼에 온도와 조도 값을 넣습니다.
	printf("get\n");
	temp = buffer1[use_ptr1];
	light = buffer2[use_ptr2];
	use_ptr1 = (use_ptr1 + 1) % MAX;
	use_ptr2 = (use_ptr2 + 1) % MAX;
	count1--;
	printf("get temp : %d get light:%u\n", temp, light);
}
pthread_cond_t empty, fill;
pthread_mutex_t mutex;
int read_mcp3208_adc(unsigned char adcChannel) {
	unsigned char buff[3];
	int adcValue = 0;
	buff[0] = 0x06 | ((adcChannel & 0x07) >> 2);
	buff[1] = ((adcChannel & 0x07) << 6);
	buff[2] = 0x00;
	digitalWrite(CS_MCP3208, 0);
	wiringPiSPIDataRW(SPI_CHANNEL, buff, 3);
	buff[1] = 0x0f & buff[1];
	adcValue = (buff[1] << 8) | buff[2];
	digitalWrite(CS_MCP3208, 1);
	return adcValue;
}
void *templight(void *arg) {
	int i = 0;
	printf("pro1\n");
	for (i = 0; i<loop; i++) {
		printf("pro2\n");
		//  temp = get_temperature_sensor(); // Temperature Sensor
		//  light = read_mcp3208_adc(adcChannel_light); // Illuminance Sensor
		pthread_mutex_lock(&mutex); //락을 걸고 count1이 MAX면 empty신호가 올 때까지 기다림
		while (count1 == MAX)
			pthread_cond_wait(&empty, &mutex); //소비자로부터 empty 신호가 오면 mutex 풀기
		printf("pro3\n");
		temp = get_temperature_sensor(); //온도와 조도값을 put 함수로 전달
		light = read_mcp3208_adc(adcChannel_light);
		put(temp, light);
		pthread_cond_signal(&fill); //소비자에 값을 생성했다고 알림
		pthread_mutex_unlock(&mutex); //락 풀기
		printf("temp : %d light : %u\n", temp, light);
		sleep(1);
	}
}
void *fan(void *arg) {
	int i = 0;
	printf("fan1\n");
	for (i = 0; i<loop; i++) {
		printf("fan2\n");
		pthread_mutex_lock(&mutex);
		while (count1 == 0)
			pthread_cond_wait(&fill, &mutex);
		printf("fan temp : %d\n", temp);
		if (temp >= 30) {  //temp가 30도가 넘으면 FAN을 5초간 돌린다
			digitalWrite(FAN, 1);
			delay(5000);
		}
		else { digitalWrite(FAN, 0); }
		pthread_cond_signal(&empty);
		pthread_mutex_unlock(&mutex);
		sleep(1);
	}
}

void *send(void *arg) {
	int i = 0;
	char query[1024];
	db();
	/*  printf("sending %d\n",count1);
	  connector=mysql_init(NULL);
	    if(!mysql_real_connect(connector,DBHOST,DBUSER,DBPASS,3306,NULL,0)){
		    fprintf(stderr,"%s\n",mysql_error(connector));
			    return 0;
				  }
				    printf("MySql opened\n");
					*/
	for (i = 0; i<loop; i++) {
		//    char query[1024];
		//db();  
		printf("sending2\n");
		pthread_mutex_lock(&mutex); //락 걸기
		printf("seding lock\n");
		while (count1 == 0) { //소비할 값이 없으면 기다림
			printf("%d\n", count1);
			pthread_cond_wait(&fill, &mutex); //생산자가 값을 생성했다고 하면 mutex락 풀기
			printf("ddd\n");
		}
		printf("sending3\n");
		printf("sending4\n");
		get1(); //값을 얻어 오고
		sprintf(query, "insert into tmp values (now(),%d,%u)", temp, light); //값을 DB에 insert
		printf("query:%s\n", query);
		   /* if(mysql_query(connector, query))
		       {
			         fprintf(stderr, "%s\n", mysql_error(connector));
					        printf("Write DB error\n");
							    }*/
		pthread_cond_signal(&empty);
		pthread_mutex_unlock(&mutex);
		printf("DB Success\n");
		// sleep(1);
		sleep(10000);
	}
	//  delay(1000); //10sec delay
}

void *led(void *arg) {
	int i = 0;
	printf("light1\n");
	for (i = 0; i<loop; i++) {
		printf("light2\n");
		pthread_mutex_lock(&mutex);
		while (count1 == 0)
			pthread_cond_wait(&fill, &mutex);
		if (light>dark) { //조도 값이 지정된 값보다 크다면(어두워지면) 빨간불 켜기
			digitalWrite(RED, 1);
			digitalWrite(BLUE, 0);
			digitalWrite(GREEN, 0);
			printf("LED ON\n");
			delay(1000);
		}
		else {
			printf("LED OFF\n");
			digitalWrite(RED, 0);
			digitalWrite(BLUE, 0);
			digitalWrite(GREEN, 0);
			delay(1000);
		}
		pthread_cond_signal(&empty);
		pthread_mutex_unlock(&mutex);
		sleep(1);
	}
}
void db() {
	connector = mysql_init(NULL);
	if (!mysql_real_connect(connector, DBHOST, DBUSER, DBPASS, DBNAME, 3307, NULL, 0))
	{
		fprintf(stderr, "%s\n", mysql_error(connector));
		printf("db error\n");
	}
	else {
		printf("db success");
	}
	if (mysql_query(connector, query))
	{
		fprintf(stderr, "%s\n", mysql_error(connector));
		printf("Write DB error\n");
	}

}
int main(int argc, char *argv[])
{
	printf("MAIN\n");
	//  db();
	if (wiringPiSetup() == -1)
		exit(EXIT_FAILURE);
	if (wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED) == -1) {
		fprintf(stdout, "wiringPiSPISetup Failed:%s\n", strerror(errno));
		return 1;
	}
	printf("MAIN2\n");
	//  db();
	printf("MAIN3\n");
	signal(SIGINT, (void *)sig_handler);
	pinMode(CS_MCP3208, OUTPUT);
	pinMode(FAN, OUTPUT);
	pinMode(RGBLEDPOWER, OUTPUT);
	pinMode(RED, OUTPUT);
	pinMode(GREEN, OUTPUT);
	pinMode(BLUE, OUTPUT);
	pthread_t p1, p2, p3, p4; //쓰레드 선언
							  //  printf("MAIN4\n");
							  //  db();
							  //  printf("MAIN5\n");
							  /*  // MySQL connection
							    connector = mysql_init(NULL);
								  if (!mysql_real_connect(connector, DBHOST, DBUSER, DBPASS, DBNAME, 3306, NULL, 0))
								    {
									    fprintf(stderr, "%s\n", mysql_error(connector));
										    printf("db error\n");
											    return 0;
												  }
												    printf("MySQL(rpidb) opened.\n");
													*/
	printf("main:thread start");
	pthread_create(&p4, NULL, templight, "templight"); //쓰레드 생성
	pthread_create(&p2, NULL, fan, "fan");
	pthread_create(&p3, NULL, led, "led");
	pthread_create(&p1, NULL, send, "send");
	pthread_join(p4, NULL); //쓰레드가 종료되는 걸 기다림
	pthread_join(p2, NULL);
	pthread_join(p3, NULL);
	pthread_join(p1, NULL);
	//    db();
	return 0;
}
int get_temperature_sensor()
{
	int received_temp;
	int _retry = RETRY;

	if (wiringPiSetup() == -1)
		exit(EXIT_FAILURE);
	if (setuid(getuid()) <0)
	{
		perror("Dropping privileges failed\n");
		exit(EXIT_FAILURE);
	}
	while (read_dht22_dat_temp() == 0 && _retry--)
	{
		delay(500); // wait 1sec to refresh
	}
	received_temp = ret_temp;
	return received_temp;
}

static uint8_t sizecvt(const int read)
{
	  /* digitalRead() and friends from wiringpi are defined as returning a value
	    <256. However, they are returned as int() types. This is a safety function */

	if (read >255 || read <0)
	{
		printf("Invalid data from wiringPi library\n");
		exit(EXIT_FAILURE);
	}
	return (uint8_t)read;
}

int read_dht22_dat_temp()
{
	uint8_t laststate = HIGH;
	uint8_t counter = 0;
	uint8_t j = 0, i;

	dht22_dat[0] = dht22_dat[1] = dht22_dat[2] = dht22_dat[3] = dht22_dat[4] = 0;

	// pull pin down for 18 milliseconds
	pinMode(DHTPIN, OUTPUT);
	digitalWrite(DHTPIN, HIGH);
	delay(10);
	digitalWrite(DHTPIN, LOW);
	delay(18);
	// then pull it up for 40 microseconds
	digitalWrite(DHTPIN, HIGH);
	delayMicroseconds(40);
	// prepar
	pinMode(DHTPIN, INPUT);

	// detect change and read data
	for (i = 0; i< MAXTIMINGS; i++) {
		counter = 0;
		while (sizecvt(digitalRead(DHTPIN)) == laststate) {
			counter++;
			delayMicroseconds(1);
			if (counter == 255) {
				break;
			}
		}
		laststate = sizecvt(digitalRead(DHTPIN));

		if (counter == 255) break;

		// ignore first 3 transitions
		if ((i >= 4) && (i % 2 == 0)) {
			// shove each bit into the storage bytes
			dht22_dat[j / 8] <<= 1;
			if (counter >50)
				dht22_dat[j / 8] |= 1;
			j++;
		}
	}

	// check we read 40 bits (8bit x 5 ) + verify checksum in the last byte
	// print it out if data is good
	if ((j >= 40) &&
		(dht22_dat[4] == ((dht22_dat[0] + dht22_dat[1] + dht22_dat[2] + dht22_dat[3]) & 0xFF))) {
		float t;
		t = (float)(dht22_dat[2] & 0x7F) * 256 + (float)dht22_dat[3];
		t /= 10.0;
		if ((dht22_dat[2] & 0x80) != 0)  t *= -1;
		ret_temp = (int)t;
		return ret_temp;
	}
	else
	{
		printf("Data not good, skip\n");
		return 0;
	}
}
void sig_handler(int signo) {
	printf("process stop\n");
	digitalWrite(FAN, 0);
	digitalWrite(RED, 0);
	digitalWrite(GREEN, 0);
	digitalWrite(BLUE, 0);
	digitalWrite(RGBLEDPOWER, 0);
	mysql_close(connector);

	exit(0);
}