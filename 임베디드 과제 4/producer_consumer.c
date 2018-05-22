#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <mysql/mysql.h>
#include <time.h>
#define MAX 5
#define CS_MCP3208  8        // BCM_GPIO_8
#define SPI_CHANNEL 0
#define SPI_SPEED   1000000   // 1MHz
#define VCC         4.8       // Supply Voltage
#define DBHOST "localhost" //���� DB�� �����ϱ� ���� ����
#define DBUSER "root"
#define DBPASS "root"
#define DBNAME "demofarmdb"
MYSQL *connector;
MYSQL_RES *result;
MYSQL_ROW row;
int read_mcp3208_adc(unsigned char adcChannel)
{
	unsigned char buff[3];
	int adcValue = 0;
	buff[0] = 0x06 | ((adcChannel & 0x07) >> 2);
	buff[1] = ((adcChannel & 0x07) << 6);
	buff[2] = 0x00;
	digitalWrite(CS_MCP3208, 0);  // Low : CS Active
	wiringPiSPIDataRW(SPI_CHANNEL, buff, 3);
	buff[1] = 0x0F & buff[1];
	adcValue = (buff[1] << 8) | buff[2];
	digitalWrite(CS_MCP3208, 1);  // High : CS Inactive
	return adcValue;
}
int buffer[MAX];
int fill_ptr = 0;
int use_ptr = 0;
int count = 0;
int loops = 1000;

void put(int value) { //�µ� ���� �޾� ���ۿ� �ִ´�.
	buffer[fill_ptr] = value;
	fill_ptr = (fill_ptr + 1) % MAX;
	count++;
}
int get() { //���ۿ� �ִ� �µ��� �޾Ƽ� DB�� insert �Ѵ�.
	char query[1024];
	int tmp = buffer[use_ptr];
	use_ptr = (use_ptr + 1) % MAX;
	count--;
	// MySQL connection
	connector = mysql_init(NULL);
	if (!mysql_real_connect(connector, DBHOST, DBUSER, DBPASS, DBNAME, 3306, NULL, 0))
	{
		fprintf(stderr, "%s\n", mysql_error(connector));
		return 0;
	}
	printf("MySQL(rpidb) opened.\n");
	sprintf(query, "insert into temp values (now(),%d)", tmp); //temp table�� ���� �ð��� �µ��� �ִ�//��.
	if (mysql_query(connector, query))
	{
		fprintf(stderr, "%s\n", mysql_error(connector));
		printf("Write DB error\n");
	}
	printf("data From DB : %d\n", tmp);
	mysql_close(connector);
	return tmp;
}
pthread_cond_t empty, fill;
pthread_mutex_t mutex;
void *producer(void *arg) {
	int adcChannel = 0;
	int adcValue[8] = { 0 };
	int i = 0;
	//delay(3000);
	char query[1024];
	for (i = 0; i<loops; i++) { /*�µ��� �о� ȭ�鿡 ����Ѵ�. ���� �ɰ� count�� MAX�� ��ٸ���. �Һ��ڷκ��� empty ��ȣ�� ���� mutex���� Ǭ��. put�Լ��� ���ۿ� �µ��� �ִ´�. fill ��ȣ�� ������ ���� Ǯ�� �Һ��ڰ� ���ۿ� �ִ� �µ��� ����� �� �ְ� ���ش�.*/
		adcValue[0] = read_mcp3208_adc(0); // Temperature Sensor
		adcValue[0] = ((adcValue[0] * VCC / 4095 - (0.1 * VCC)) / (0.8 * VCC)) * 200 - 50;

		printf("Producer : %d\n", adcValue[0]);
		delay(3000); //3sec delay
		pthread_mutex_lock(&mutex);
		//collect temp every 3s.
		while (count == MAX)
			pthread_cond_wait(&empty, &mutex);
		put(adcValue[0]);//put temp
		pthread_cond_signal(&fill);
		pthread_mutex_unlock(&mutex);
	}
}
void *consumer(void *arg) { /*�Һ��ڴ� ���ؽ� ���� �ɰ� count�� 0�̸� fill ��ȣ�� �� ������ ��ٸ��� fill ��ȣ�� ���� mutex�� Ǭ��. get�Լ��� ȣ���ؼ� ���ۿ� �ִ� ���� ������ DB�� �����Ѵ�. �Һ� �����Ƿ� empty ��ȣ�� ������ ���� Ǯ�� �����ڰ� ������ �� �ְ� ���ش�.*/
	int i = 0;
	for (i = 0; i<loops; i++) {
		pthread_mutex_lock(&mutex);
		//storing temp to DB
		while (count == 0)
			pthread_cond_wait(&fill, &mutex);
		get();
		pthread_cond_signal(&empty);
		pthread_mutex_unlock(&mutex);
	}
}
int main(int argc, char *argv[]) {
	int adcChannel = 0;
	int adcValue[8] = { 0 };
	if (wiringPiSetupGpio() == -1)
	{
		fprintf(stdout, "Unable to start wiringPi: %s\n", strerror(errno));
		return 1;
	}
	if (wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED) == -1)
	{
		fprintf(stdout, "wiringPiSPISetup Failed: %s\n", strerror(errno));
		return 1;
	}
	pinMode(CS_MCP3208, OUTPUT);
	pthread_t p1, p2;
	printf("main : begin producer and consumer\n");
	pthread_create(&p1, NULL, producer, "producer"); //�����ڿ� �Һ��� ������ ����
	pthread_create(&p2, NULL, consumer, "consumer");
	//join wait
	pthread_join(p1, NULL); //������ �Һ��� �����尡 ����Ǳ� ��ٸ�
	pthread_join(p2, NULL);
	printf("main:procon done\n");
	return 0;
}