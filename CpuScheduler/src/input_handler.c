#include <stdio.h>

int input_int_range(char* question, int start, int end)
{
	while (1)
	{
		printf("%s", question);
		
		int input;
		scanf("%d", &input);

		if (input < start || input > end)
		{
			printf("Wrong input.");
		}
		else
		{
			return input;
		}
	}
}

int input_int_once(int def_val)
{
	int ch;
	while ((ch = getchar()) != '\n' && ch != EOF);
	
	char buf[128];
	if (fgets(buf, sizeof(buf), stdin) == NULL)
	{
		return def_val;
	}

	if (buf[0] == '\n')
	{
		return def_val;
	}

	char* end;
	long value = strtol(buf, &end, 10);

	return (int)value;
}