#include "stm32f10x.h"
#include "includes.h"

//#include "delay.h"
#include "usart.h"
#include "led.h"
#include "beep.h"
#include "wifi.h"

extern u8 doorST;
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
extern void STEPPER_init(void);
extern void steprightoneround(void);
extern void stepleftoneround(void);
extern u8  getkey(void);


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
		client_send("MODULE2 stepper CONNENT\n");
		return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////
///�豸״̬λ0 ���� 1������
u8 stepST=0;
u8 wifiST=0;

////////////////////////////////////////////////////////////////////////////////////////

void driverinit(){		
	
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
		delay_init();	    //��ʱ������ʼ��
		uart_init(115200);   //uart1 PA9 PA10
		LED_init();					//LED_init
		BEEP_init();				//BEEP ��������ʼ��
		STEPPER_init();      //����������ʻ� 
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
  OSTaskCreate(drSTATUS_task,(void *)0,(OS_STK*)&drSTATUS_TASK_STK[drSTATUS_STK_SIZE-1],drSTATUS_TASK_PRIO);	
	OSTaskSuspend(START_TASK_PRIO);	//������ʼ����.
	OS_EXIT_CRITICAL();				//�˳��ٽ���(���Ա��жϴ��)
}



//Contrl ����  ��������CL����ҿ���
void Contrl_task(void *pdata)
{	 	
		
		while(1){	
			receive_server();
//			printf("%d\n",getkey());
//			delay_ms(100);
			if(getkey()==1){ 	//�����͵�ƽ  ���ذ��� 
					steprightoneround(); doorST=1;BEEP(ON); delay_ms(500);	BEEP(OFF) ; delay_ms(5);client_send("K M2 DOOR is ON now!!!!\n");
			}
			if(doorST == 1)    //����ſ�   ��ʱ5 ���Զ���
			{delay_ms(5000);stepleftoneround(); doorST=0; BEEP(ON); delay_ms(1000);	BEEP(OFF) ;	delay_ms(5);client_send("K M2  DOOR is OFF now!!!!\n");}
		
			
			}
	
}



//�豸״̬  ����
void drSTATUS_task(void *pdata)        //���ú�  �����豸
{	  int i;
		while(1)
		{	    
				if(wifiST){printf("ST+M2+WF+FL\n");}	
				else{printf("ST+M2+WF+GD\n");}
				delay_ms(10);
				if(stepST){printf("ST+M2+STP+FL\n");}	
				else{printf("ST+M2+STP+GD\n");}
				for(i=0;i<5;i++)
					delay_ms(60000);                 //û30���ӷ���һ��
		}			
}
