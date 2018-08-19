#include "stm32f10x.h"

#include "usart.h"
#include "wifi.h"
#include "switch.h"

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
////////////////////////////////////////////////
//HiMaster ���ܲ���ģ��
//2018-8-20
/////////////////////////////////////////////////////////////////////////////////////////
///�豸״̬λ0 ���� 1������
u8 wifiST=0;

//connectPISER ��ʼ������
u8 connectPISER(){
		SystemInit (); // ����ϵͳʱ��Ϊ72M 		
		while(1){              //�ȴ����ӳɹ�
			atk_8266_quit_trans();//���˳���͸  ȷ����ʼ���ɹ� ����apָ������������ͣ�
			if(!esp_8662_init()){
				break;
			}
		}
		atk_8266_send_cmd("AT+CIPMUX=0","OK",20);  //������ģʽ
		while(1){              //�ȴ������ͻ���
			if(!client_init()){
				break;       //�ɹ��ص�����
			}
		}
				
		atk_8266_send_cmd("AT+CIPMODE=1","OK",20);	//��ʼ͸��	
		atk_8266_send_cmd("AT+CIPSEND","OK",20);
		client_send("MODULE3 SWITCH CONNENT\n");
		return 0;
}

void driverinit(){
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�  ,usart�����ж�
		SystemInit (); // ����ϵͳʱ��Ϊ72M 	
		delay_init();	    	 //��ʱ������ʼ��	  
		uart_init(115200);
		SWITCH_init();
		if(connectPISER())	//connectPISER��ʼ��	 
		{		
			
				wifiST=1;// ʧ�� �豸״̬λ��1
				delay_ms(20);
		}	
}

//ע������ͳһ 
int main (void)
{		
		driverinit();
		if(wifiST){printf("ST+M3+WF+FL\n");}	
		else{printf("ST+M3+WF+GD\n");}
		while(1){
				
				receive_server();//ѭ����ȡ
				delay_ms(100);
		}
}


