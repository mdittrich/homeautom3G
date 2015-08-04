/*
 * TestApplication.cpp
 *
 * Copyright Martin Dittrich
 * Hamburg University of Technology
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL I
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <string>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "gpio/gpio.h"
#include "netlib/netlib.h"
#include "serialib/serialib.h"
#define DEVICE_PORT "/dev/ttyO5"
#define BAUD_RATE 19200
#define DEBUG_MODE 0
#define CONFIRM_SMS 1

unsigned int RI = 23;   // GPIO0_23
unsigned int DTR = 26;   // GPIO0_26
unsigned int GSM_EN = 2;   // GPIO0_2
unsigned int GPIO_1 = 70;
unsigned int GPIO_2 = 71;
unsigned int GPIO_3 = 72;
unsigned int GPIO_4 = 73;
signed int val = 0;
unsigned int size = 16; char *msg[16];
unsigned int last_num = 0;
char buffer[256], *numbers[16], loop = 1;

using namespace std;

// Object of the serialib class
serialib TTY;
// Object of the serialib class
netlib WWW;

int readLine(unsigned int timeout) {
	// Read a string from the serial device
	val = TTY.ReadString(buffer, '\n', sizeof(buffer), timeout);
	if(val > 0) {
		printf(" -> RxD: %s", buffer);
		return val;
	} else {
		if(DEBUG_MODE) printf("TimeOut reached, no data received!\n");
		return val;
	}
}

void sendAT(const char *at_cmd, const char *secure_str = "") {
	// Write the AT command on the serial port
	// Output looks like "<atcmd>\r"
	char *atcmd = strdup(at_cmd), *pwd = strdup(secure_str);
	val = TTY.WriteString(atcmd);
	val *= TTY.WriteString(pwd);
	val *= TTY.WriteChar(0x0D);
	if(strlen(pwd) > 0)
		pwd = (char*)"****";
	if(val == 1) {
		printf(" <- TxD: %s%s\n", atcmd, pwd);
	} else {
		printf("Error while writing data!\n");
	}
}

int waitForReceive(const char *whitelist = "OK", const char *blacklist = "ERROR", unsigned int timeout = 1000) {
	int i = 0, j = 0, k, l, state = 0;
	char *wlist[8], *blist[8], delims[] = "|", *token = NULL;
	if(whitelist == "")
		whitelist = "OK";
	if(blacklist == "")
		blacklist = "ERROR";
	for(l = 0; l < size; l++)
		msg[l] = NULL; l = 0;
	token = strtok(strdup(whitelist), delims);
	while(token != NULL) {
		asprintf(&token, "%s\r\n", token);
		if(DEBUG_MODE) printf(" <> Token: %s", token);
		wlist[i++] = token;
		token = strtok(NULL, delims);
	}
	token = strtok(strdup(blacklist), delims);
	while(token != NULL) {
		asprintf(&token, "%s\r\n", token);
		blist[j++] = token;
		token = strtok(NULL, delims);
	}
	do {
		val = readLine(timeout);
		for(k = i-1; k >= 0; k--) {
			if(!strcmp(wlist[k], buffer) and val > 0) {
				state = 1;
			}
		}
		for(k = j-1; k >= 0; k--) {
			if(!strcmp(blist[k], buffer) and val > 0) {
				state = -1;
			}
		}
		msg[l] = new char[sizeof(buffer)];
		strcpy(msg[l], buffer);
		l++;
	} while(val > 0);
	return state;
}

int waitForRI(void) {
	// Loops infinite until a ring (RI) signal occurs
	unsigned int input = HIGH;
	do {
		gpio_get_value(RI, &input); 
		usleep(100000);
	} while(input != LOW and loop);
	if(input == LOW)
		return 1;
	return 0;
}

void wakeUpByDTR(void) {
	// Send DTR signal to activate the Serialport
	printf("Wake up SIM900 from Standby...\n");
	gpio_set_value(DTR, LOW);
	usleep(500000);
	gpio_set_value(DTR, HIGH);
	usleep(1500000);
}

void sendSMS(char *number, char *text) {
	// Send Text SMS
	asprintf(&number, "\"%s\"", number);
	sendAT("AT+CMGF=1");
	if(waitForReceive())
		sendAT("AT+CMGS=", number);
	usleep(500000);
	sendAT(text);
	val = TTY.WriteChar(0x1A);
	if(val == 1) {
		printf("SMS send successfully!\n");
	} else {
		printf("Error while processing an SMS!\n");
	}
}

void powerUD(void) {
	gpio_set_value(GSM_EN, HIGH);
	usleep(1500000);
	gpio_set_value(GSM_EN, LOW);
	usleep(500000);
}

int hash(const char *str) {
	int hash = 0;
	while(*str)
		hash = hash << 1 ^ *str++;
	return hash;
}

int count(char **arr) {
    int i = 0, j = 0;
    while(arr[i]) {
        if(strlen(arr[i]) > 0)
            j++;
        i++;
    }
    return j;
}

char *getStringPart(const char *action_str, char delim = 0x22, unsigned int first_occ = 1, unsigned int sec_occ = 16) {
	int i = 1, j[16]; j[0] = 0;
	char *pch, *str, *output = new char[16];
	if(first_occ > sec_occ) {
	    return (char*)"-1";
	}
	str = strdup(action_str);
	pch = strchr(str, delim);
	while(pch != NULL) {
		j[i] = pch-str+1;
		pch = strchr(pch+1, delim);
		i++;
	}
	if(sec_occ > i-1) {
	    sec_occ = i;
	    j[sec_occ] = strlen(str)+1;
	}
	if(first_occ == sec_occ) {
	    sec_occ += 1;
	}
	strncpy(output, str+j[first_occ], j[sec_occ]-j[first_occ]-1);
	return output;
}

char **getSMSText(char *memory) {
	// Returns the first line of the Text SMS
	int i, j = 0, k = 0;
	char *atcmd, **buf = new char*[16];
	asprintf(&atcmd, "AT+CMGR=%s", memory);
	sendAT(atcmd);
	waitForReceive();
	for(i = 0; msg[i] != NULL; i++) {
		if(!strcmp(getStringPart(msg[i], 0x3A, 0, 1), "+CMGR")) {
			while(strcmp(getStringPart(msg[i], 0x22, 3, 4), numbers[j++])) {
				if(j >= count(numbers))
					return 0;
			}
			last_num = j-1;
			while(strcmp(msg[++i], "\r\n")) {
				j = 0;
				while(msg[i][j]) {
					if(msg[i][j] == '\r' or msg[i][j] == '\n')
						msg[i][j] = '\0';
					else
						msg[i][j] = toupper(msg[i][j]);
					j++;
				}
				buf[k] = new char[32];
				strcpy(buf[k++], msg[i]);
			}
			asprintf(&atcmd, "AT+CMGD=%s", memory);
			sendAT(atcmd);
			waitForReceive();
			return buf;
		}
	}
}

int main(int argc, char *argv[]) {
	signed int return_val, cmd = 0;
	unsigned int i = 0, j;
	char **line, *smstxt, *ip;

	printf("Importing configuration settings from File...\n");
	usleep(500000);
	// Reads config data from file
	FILE *fp = fopen("numbers_allowed.conf", "r");
	if(fp == NULL) {
		printf("Error while reading file!\n");
		return -1;
	}
	while(fgets(buffer, 16, fp)) {
		if(DEBUG_MODE) printf("%s", buffer);
		numbers[i] = new char[sizeof(buffer)];
		strcpy(numbers[i], buffer);
		j = 0;
		while(numbers[i][j]) {
			if(numbers[i][j] == '\r' or numbers[i][j] == '\n')
				numbers[i][j] = '\0';
			j++;
		}
		i++;
	}
	fclose(fp);
	printf("All data successfully read!\n");
	usleep(500000);

	printf("Initiate the GPIO pins...\n");
	usleep(500000);

	gpio_export(RI);
	gpio_export(DTR);
	gpio_export(GSM_EN);
	gpio_export(GPIO_1);
	gpio_export(GPIO_2);
	gpio_export(GPIO_3);
	gpio_export(GPIO_4);
	gpio_set_dir(RI, INPUT_PIN);
	gpio_set_dir(DTR, OUTPUT_PIN);
	gpio_set_dir(GSM_EN, OUTPUT_PIN);
	gpio_set_dir(GPIO_1, OUTPUT_PIN);
	gpio_set_dir(GPIO_2, OUTPUT_PIN);
	gpio_set_dir(GPIO_3, OUTPUT_PIN);
	gpio_set_dir(GPIO_4, OUTPUT_PIN);
	gpio_set_value(DTR, HIGH);
	gpio_set_value(GSM_EN, LOW);
	gpio_set_value(GPIO_1, HIGH);
	gpio_set_value(GPIO_2, HIGH);
	gpio_set_value(GPIO_3, HIGH);
	gpio_set_value(GPIO_4, HIGH);

	printf("GPIO pins successfully set!\n");
	usleep(500000);

	printf("Initiate the TTY port...\n");
	usleep(500000);

	// Open serial port
	val = TTY.Open(DEVICE_PORT, BAUD_RATE);
	if(val != 1) {
		printf("Error while opening port!\n");
		return val;
	}
	printf("Serial port opened successfully!\n");
	usleep(500000);
	
	wakeUpByDTR();

	startpoint:	
	
	printf("Testing the operating status of the GSM-Module...\n");
	
	// Check whether the unit is switched on
	return_val = waitForReceive("+CPIN: SIM PIN");
	if(val < 0) {
		printf("Error while reading, code: %i\n", val);
		return -1;
	}
	if(return_val == 1)
		printf("SIM900 is already powered up\n");
	else {
		sendAT("AT");
		if(!waitForReceive()) {
			printf("SIM900 is powered down, try to boot it up...\n");
			powerUD();
			printf("SIM900 is now powered up\n");
		}
		else
			printf("SIM900 is already powered up\n");
	}
	
	waitForReceive("+CPIN: SIM PIN");
	usleep(500000);
	printf("Unlock SIMcard:\n");
	usleep(500000);
	sendAT("AT+CPIN=", "1866");
	return_val = waitForReceive();
	if(return_val == 1)
		printf("Wait for Provider login...\n");
	else if(return_val == -1)
		printf("PIN wrong or already set!\nWait for Provider login...\n");
	usleep(500000);
	return_val = waitForReceive("+CREG: 1|Call Ready", "", 20000);
	if(return_val)
		printf("Login successful!\n");
	else {
		sendAT("AT+CREG?");
		if(waitForReceive("+CREG: 1,1|+CREG: 1,5")) {
			waitForReceive();
			printf("Login successful!\n");
		}
		else {
			printf("Login failed!\n");
			printf("Try to restart the Module...\n");
			powerUD();
			usleep(5000000);
			powerUD();
			usleep(2000000);
			goto startpoint;
		}
	}
	usleep(500000);
	printf("Check Privider:\n");
	usleep(500000);
	sendAT("AT+COPS?");
	waitForReceive();
	printf("Check signal Quality:\n");
	usleep(500000);
	sendAT("AT+CSQ");
	waitForReceive();
	usleep(500000);
	
	do {
		printf("Go into idle mode...\n");
		if(waitForRI()) {
			waitForReceive();
			for(i = 0; msg[i] != NULL; i++) {
				// Search char(':') and get String-Part msg[0 ... pos(':')-1]
				if(!strcmp(getStringPart(msg[i], 0x3A, 0, 1), "+CMTI")) {
					// Reads SMS memory index and Text and deletes them afterwards
					line = getSMSText(getStringPart(msg[i], 0x2C));
					ip = NULL;
					for(j = 0; j < count(line); j++) {
						if(DEBUG_MODE) printf("SMS text: %s\n", line[j]);
						else switch(hash(line[j])) {
								case 542:	// "R1=0"
									printf("Set Relay 1 to \"Off\" state\n");
									gpio_set_value(GPIO_1, HIGH);
									cmd++;
									break;
								case 543:	// "R1=1"
									printf("Set Relay 1 to \"On\" state\n");
									gpio_set_value(GPIO_1, LOW);
									cmd++;
									break;
								case 530:	// "R2=0"
									printf("Set Relay 2 to \"Off\" state\n");
									gpio_set_value(GPIO_2, HIGH);
									cmd++;
									break;
								case 531:	// "R2=1"
									printf("Set Relay 2 to \"On\" state\n");
									gpio_set_value(GPIO_2, LOW);
									cmd++;
									break;
								case 8187:	// "IPADDR?"
									printf("Trying to obtain my IP-Address...\n");
									// IP service at http://ifconfig.me/ip || https://api.ipify.org/
									ip = WWW.getData("https://api.ipify.org/");
									if(isdigit(ip[0]))
										asprintf(&ip, "\nAktuelle IPv4-Adresse: %s", ip);
									else
										asprintf(&ip, "\nAktuelle IPv4-Adresse: offline!");
									cmd++;
									break;
								case 28441:	// "SHUTDOWN!"
									printf("Shutdown SIM900 and exit Program...\n");
									cmd = -1;
									break;
						}
						if(cmd < 0) break;
					}
					asprintf(&smstxt, "%d Befehl(e) erforgreich ausgefuehrt!%s", abs(cmd), ip);
					if(CONFIRM_SMS) {
						sendSMS(numbers[last_num], smstxt);
						waitForReceive();
						printf("SMS will be send...\n");
						usleep(10000000);
					}
					if(cmd < 0) {
						powerUD();
						loop = 0;
					}
					cmd = 0;
				}
				else if(!strcmp(getStringPart(msg[i], 0x3A, 0, 1), "+CLIP")) {
					printf("CALL in!\n");
				}
			}
		}
	} while(loop);
	
	gpio_unexport(RI);
	gpio_unexport(DTR);
	gpio_unexport(GSM_EN);
	gpio_unexport(GPIO_1);
	gpio_unexport(GPIO_2);
	gpio_unexport(GPIO_3);
	gpio_unexport(GPIO_4);
	
	// Close the connection with the device
	TTY.Close();
	
	printf("Program terminated!\n");
	
	return 0;
}

