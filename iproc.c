#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#include "kbcrt.h"

	void kbd_iproc() {
         msg_env *temp_k = NULL;
		if(in_mem_p->ok_flag == 1) //check flag
			buf_index =0;
			do
			{
                PCB * temp = convert_PID(0);
				temp_k = env_DEQ(temp->receive_msg_Q);//dequeue env into msg_env
				if(temp_k ==NULL)
					printf("No envelope!!!");
				temp_k->msg_text[in_mem_p->indata[buf_index]] =in_mem_p->indata[buf_index]; //read characters from buffer into msg_env
				buf_index++;
			}
			while(in_mem_p->indata[buf_index-1]= '\0'); 
			in_mem_p->ok_flag =0;
            strcpy(temp_k->msg_type,"console_input");
			int Z =send_message(temp_k->sender_id, temp_k); //send env back to process 
			if(Z=1) // incorrect return from send_msg
				printf("Error with sending");	

		else
			printf("fix the functionn!!!!");
	}

	void crt_iproc() {
         msg_env *temp_c = NULL;
		if(out_mem_p->ok_flag == 0) //check flag
			buf_index =0;
			do
			{
				temp_c = env_DEQ(convert_PID(0)->receive_msg_Q);//dequeue env into msg_env
				if(temp_c==NULL)
					printf("No envelope!!!");
				out_mem_p->outdata[buf_index]=temp_c->msg_text[out_mem_p->outdata[buf_index]]; //send characters from msg_env into buffer
				buf_index++;
			}
			while(out_mem_p->outdata[buf_index-1]= '\0'); 
			out_mem_p->ok_flag =1;
			strcpy(temp_c->msg_type,"display_ack");
			int Z =send_message(temp_c->sender_id, temp_c); //send env back to process 
			if(Z=1) // incorrect return from send_msg
				printf("Error with sending");
			
     	else
			printf("fix the functionn!!!!");
	}
