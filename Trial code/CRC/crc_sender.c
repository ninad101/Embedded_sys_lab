#include <stdio.h>
#include <string.h>

#define MESSAGE_SIZE 50

int main()
{

	char msg[MESSAGE_SIZE], crc[MESSAGE_SIZE];

	printf("%s\n", "Enter the message: ");
	
	fgets(msg, MESSAGE_SIZE, stdin);

	printf("%s\n", "Enter the crc generator polynomial = ");

	fgets(crc, MESSAGE_SIZE, stdin);


	//Dirty hack to get the length to 8, getting rid of last enter character
	msg[(strlen(msg)-1)] = NULL;
	crc[(strlen(crc)-1)] = NULL;

	//printf("%s %s\n", "Message given: ", msg);

	int m = strlen(msg);
	int n = strlen(crc);

	char encoded[(m + n -2)]; 

	for (int i = 0; i < m; i++)
	{
		encoded[i] = msg[i];
	}

	//Add zeros to the message
	for(int i=0; i<=n-2; i++)
		encoded[m+i] = '0';

	printf("%s %s\n", "Message given: ", encoded);
	printf("%s %ld\n", "Length of encoded message: ", strlen(encoded));

	for (int i = 0; i < strlen(encoded) - n+2; )
	{
		for (int j = 0; j < n; ++j)
		{
			encoded[i+j] = encoded[i+j] == crc[j]? '0':'1';
		}
		for( ; i<(strlen(encoded)+1) && encoded[i]!='1'; i++);
			printf("%s %d\n","i:", i);
		
	}

	printf("%s %s\n", "Encoded message is: ", encoded);

	printf("%s %ld\n", "Length of message: ", strlen(msg));

	return 0;
}
