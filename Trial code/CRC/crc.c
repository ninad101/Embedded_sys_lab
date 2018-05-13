#include <stdio.h>
#include <string.h>

#define MESSAGE_SIZE 50

int main()
{

	//char msg[MESSAGE_SIZE], crc[MESSAGE_SIZE];

	// printf("%s\n", "Enter the message: ");
	
	// fgets(msg, MESSAGE_SIZE, stdin);

	// printf("%s\n", "Enter the crc generator polynomial = ");

	// fgets(crc, MESSAGE_SIZE, stdin);

	char msg[49] = "001100010000100000100000010000011000001110000100";


	char crc[10] = "101011100";


	//printf("%s %s\n", "Message given: ", msg);

	int m = strlen(msg);
	int n = strlen(crc);

	printf("%s %s %s %d\n", "msg:", msg , "," , m);
	printf("%s %s %s %d\n", "crc:", crc , "," , n);

	int encodedLength_s = m + n;

	char encoded[63]; 

	for (int i = 0; i < m; i++)
	{
		encoded[i] = msg[i];
	}

	//Add zeros to the message
	for(int i=0; i<n-1; i++) {
		encoded[m+i] = '0';
	}

	printf("%s %s\n", "Message given, encoded with zeros: ", encoded);
	printf("%s %ld\n", "Length of encoded message: ", strlen(encoded));

	//int encodedLength_s = strlen(encoded) + 1;

	for (int i = 0; i < encodedLength_s - n + 1; )
	{
		for (int j = 0; j < n; ++j)
		{
			encoded[i+j] = encoded[i+j] == crc[j]? '0':'1';
		}
		for( ; i<(encodedLength_s) && encoded[i]!='1'; i++);		
	}


	char msg_bad[49] = "001100010000100000100000010110011000001110000100";

	printf("%s %s\n", "encodedMsg", encoded);

	int CRCcheckBytesLength = encodedLength_s - n + 1;
	char encodedMsg[m + (encodedLength_s - n + 1)];

	int i = 0;
	for(; i < m; i++)
	{
		encodedMsg[i] = msg_bad[i];
	}
	for(; i < m + CRCcheckBytesLength; i++) 
	{
		encodedMsg[i] = encoded[i];
	}


	printf("%s %s\n", "Encoded_s Msg: " , encodedMsg);

	// int 

	for (int i = 0; i < strlen(encodedMsg) - n + 1;)
	{
		for(int j = 0; j < n; j++ ) 
		{
			//printf("%d\n", (j+i) );
			encodedMsg[i+j] = encodedMsg[i+j] == crc[j]? '0':'1';
		}
		for( ; i < (strlen(encodedMsg)) && encodedMsg[i] != '1'; i++);

	} 

	printf("%s %s\n", "Encoded message is: ", encodedMsg);




	return 0;
}
