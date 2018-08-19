/////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//himaster ���ܹܼ�M1 ���ݼ��ģ��   
//��Ҫ���ܻش�ʵʱ�¶ȣ�ʪ�� ,���յȲ���  ����ݮ�ɷ�������
//��Ӧָ����CL ��������  ��DT �������� ��STģ��״̬����
//Ҳ������Զ�̿���ģ���϶�Ӧ�豸��
//����:HHH
//���ڣ�2018-07
////////////////////////////////////////////////////////////////////



#include "stm32f10x.h"
#include "includes.h"

#include "usart.h"
#include "led.h"
#include "beep.h"
#include "ds18b20.h"
#include "wifi.h"
#include "dht11.h"

extern u8 esp_8662_init(void);
extern void openled(void);
extern void closeled(void);
extern void ESP8266_Init(void);
extern void delay_init(void);
extern void delay_ms(u16 nms);
extern u8 client_init(void);
extern u8 client_send(u8 *cmd);
extern u8 receive_server(void);
extern u8 atk_8266_quit_trans(void);
extern u8 DHT11_Read_Data(u8 *temp,u8 *humi);
extern u8 DHT11_Init(void);


/////////////////////////UCOSII��������///////////////////////////////////
// start_task  ���ڴ�����������  �����  ����ǰ�������ϣ��������ȼ����
//drSTATUS_TASK       ���ڷ����豸�� ����״̬ �����10��  delayʱ�� �����������ȼ���ߡ�
//transmission_TASK     ������ݷ���    
// Contrl_TASK          �������� �� ����    ����ʱ�����   �� ���ȼ����
// ���ӳ� �����ȼ� ���׸���� ���� ���ȼ�Ҫ�ߡ���ȵ�������ʱ����ִ�б��    ��������  
//////////////////////////////////////////////////////////////////////////

//START ����
//�����������ȼ�
#define START_TASK_PRIO      			10 //��ʼ��������ȼ�����Ϊ���
//���������ջ��С
#define START_STK_SIZE  				64
//�����ջ	
OS_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *pdata);	
 			   
//Contrl_task      ����+ִ�� CL��������
//�����������ȼ�
#define Contrl_TASK_PRIO       			6 
//���������ջ��С
#define Contrl_STK_SIZE  		    		64
//�����ջ	
OS_STK Contrl_TASK_STK[Contrl_STK_SIZE];
//������
void Contrl_task(void *pdata);


//transmission����    // �ճ��ش���������
//�����������ȼ�
#define transmission_TASK_PRIO       			5
//���������ջ��С
#define transmission_STK_SIZE  					64
//�����ջ
OS_STK transmission_TASK_STK[transmission_STK_SIZE];
//������
void transmission_task(void *pdata);


//drSTATUS����    // 10��һ�� �������豸״̬
//�����������ȼ�
#define drSTATUS_TASK_PRIO       			4 
//���������ջ��С
#define drSTATUS_STK_SIZE  					64
//�����ջ
OS_STK drSTATUS_TASK_STK[drSTATUS_STK_SIZE];
//������
void drSTATUS_task(void *pdata);




//connectPISER ��ʼ������
u8 connectPISER(){
		SystemInit (); // ����ϵͳʱ��Ϊ72M 		
		while(1){              //�ȴ����ӳɹ�
			atk_8266_quit_trans();//���˳���͸  ȷ����ʼ���ɹ� ����apָ������������ͣ�
			if(!esp_8662_init()){
				openled();	    //�ɹ� ��������
				break;
			}
		}
		atk_8266_send_cmd("AT+CIPMUX=0","OK",20);  //������ģʽ
		while(1){              //�ȴ������ͻ���
			if(!client_init()){
				closeled();
				break;       //�ɹ��ص�����
			}
		}
				
		atk_8266_send_cmd("AT+CIPMODE=1","OK",20);	//��ʼ͸��	
		atk_8266_send_cmd("AT+CIPSEND","OK",20);
		client_send("MODULE1 CONNENT\n");
		return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////
///�豸״̬λ0 ���� 1������
u8 ds18b20ST=0;
u8 DHT11ST=0;
u8 wifiST=0;
////////////////////////////////////////////////////////////////////////////////////////

void driverinit(){		
	
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
		delay_init();	    //��ʱ������ʼ��
		uart_init(115200);   //uart1 PA9 PA10
		LED_init();					//LED_init
		BEEP_init();				//BEEP ��������ʼ��
		if(DS18B20_Init())	//DS18B20��ʼ��	
		{		
			
				//ds18b20ST=1;// ʧ�� �豸״̬λ��1
				delay_ms(20);
		}	
		if(DHT11_Init())	//DHT11_Init��ʼ��	
		{		
			
				DHT11ST=1;// ʧ�� �豸״̬λ��1
				delay_ms(20);
		}	
		if(connectPISER())	//connectPISER��ʼ��	 
		{		
			
				wifiST=1;// ʧ�� �豸״̬λ��1
				delay_ms(20);
		}	
		
}

//ע������ͳһ wifi������ݮ��   num  drivernum   dstatus(1/0)�豸�Ƿ�����   run��1/0���Ƿ������� 
int main (void)
{		

	  driverinit(); 
		OSInit();   
		OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//������ʼ����
		OSStart();	 
}

	  
//��ʼ����
void start_task(void *pdata)
{
    OS_CPU_SR cpu_sr=0;
	pdata = pdata; 
  	OS_ENTER_CRITICAL();			//�����ٽ���(�޷����жϴ��)    
 	OSTaskCreate(Contrl_task,(void *)0,(OS_STK*)&Contrl_TASK_STK[Contrl_STK_SIZE-1],Contrl_TASK_PRIO);						   
 	OSTaskCreate(transmission_task,(void *)0,(OS_STK*)&transmission_TASK_STK[transmission_STK_SIZE-1],transmission_TASK_PRIO);	
  OSTaskCreate(drSTATUS_task,(void *)0,(OS_STK*)&drSTATUS_TASK_STK[drSTATUS_STK_SIZE-1],drSTATUS_TASK_PRIO);	
	OSTaskSuspend(START_TASK_PRIO);	//������ʼ����.
	OS_EXIT_CRITICAL();				//�˳��ٽ���(���Ա��жϴ��)
}

//Contrl ����  ��������CL����ҿ���
void Contrl_task(void *pdata)
{	 	
		while(1){	
			receive_server();
			delay_ms(10);
		}
	
}

//      transmission   �ճ���������
void transmission_task(void *pdata)
{	  //dht11_task return to ����	
		
		u8 temperature;  	    
		u8 humidity; 
		delay_ms(500);
		while(1)
		{	    	    								  
			DHT11_Read_Data(&temperature,&humidity);			
			printf("DT+M1+HM+%d\n",humidity);			  //����  ʪ��
			delay_ms(200);
			//temperature=DS18B20_Get_Temp();	
			//temperature=temperature;
			printf("DT+M1+TM+%d\n",temperature*10);			  //����  �¶�   //���ȣ�0.1C  ����ֵ���¶�ֵ ��-550~1250�� 
			delay_ms(5000);
		}	
}



//�豸״̬  ����
void drSTATUS_task(void *pdata)        //���ú�  �����豸
{	  
		while(1)
		{	    
				if(ds18b20ST){printf("ST+M1+TM+FL\n");}	
				else{printf("ST+M1+TM+GD\n");}
				delay_ms(20);
				if(DHT11ST){printf("ST+M1+HM+FL\n");}	
				else{printf("ST+M1+HM+GD\n");}
				delay_ms(20);
				if(wifiST){printf("ST+M1+WF+FL\n");}	
				else{printf("ST+M1+WF+GD\n");}
				
				delay_ms(60000);

		}			
}
