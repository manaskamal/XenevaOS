#include <stdint.h>
#include <_null.h>


#define MAX_STRING_LENGTH 4095

typedef size_t WT;
#define WS (sizeof(WT))


void memset(void* targ, int val, uint32_t len) {
	uint8_t* t = (uint8_t*)targ;
	while (len--)
		*t++ = (unsigned char*)val;
}

int memcmp(const void* first, const void* second, size_t length) {
	size_t count;
	for (count = 0; count < length; count++)
	{
		if (((unsigned char*)first)[count] != ((unsigned char*)second)[count])
		{
			if (((unsigned char*)first)[count] < ((unsigned char*)second)[count])
			{
				return (-1);
			}
			else
			{
				return (1);
			}
		}
	}

	return (0); //return successful code
}


void memcpy(void* __restrict dest, void* __restrict src, size_t len) {
	//_fastcpy(dest, src, count);
	uint8_t* t = (uint8_t*)dest;
	uint8_t* s = (uint8_t*)src;
	if (len > 0) {
		// check to see if target is in the range of src and if so, do a memmove() instead
		if ((t > s) && (t < (s + len))) {
			t += (len - 1);
			s += (len - 1);
			while (len--)
				*t-- = *s++;
		}
		else {
			while (len--)
				*t++ = *s++;
		}
	}
}


int strcmp(const char* str1, const char* str2) {
	if (str1 == NULL || str2 == NULL)
		return -1;

	int res = 0;
	while (!(res = *(unsigned char*)str1 - *(unsigned char*)str2) && *str2)
		++str1, ++str2;

	if (res < 0)
		res = -1;
	if (res > 0)
		res = 1;

	return res;
}

char* strcpy(char* __restrict s1, const char* __restrict s2)
{
	if (s1 == NULL || s2 == NULL)
		return NULL;

	char* s1_p = s1;
	for (; (*s1 = *s2); s2++, s1++);
	return s1_p;
}


size_t strlen(const char* s) {
	const char* a = s;
	for (; *s; s++);
	return s - a;
}


int strncmp(const char* s1, const char* s2, size_t n) {
	while (n > 0 && *s1 != '\0' && *s1 == *s2) {
		n--, s1++, s2++;
	}

	return (n == 0) ? 0 : (size_t)((unsigned char)*s1 - (unsigned char)*s2);
}

char* strncpy(char* destString, const char* sourceString, size_t maxLength) {
	unsigned count;

	if ((destString == (char*)NULL) || (sourceString == (char*)NULL))
	{
		return (destString = NULL);
	}

	if (maxLength > MAX_STRING_LENGTH)
		maxLength = MAX_STRING_LENGTH;

	for (count = 0; count < maxLength; count++)
	{
		destString[count] = sourceString[count];

		if (sourceString[count] == '\0')
			break;
	}

	if (count >= MAX_STRING_LENGTH)
	{
		return (destString = NULL);
	}

	return (destString);
}

//! locates first occurance of character in string
char* strchr(char* str, int character) {

	do {
		if (*str == character)
			return (char*)str;
	} while (*str++);

	return 0;
}

char* strcat(char* destString, const char* sourceString)
{
	int count1, count2;

	// Find the end of the first String
	for (count1 = 0; count1 < MAX_STRING_LENGTH;)
	{
		if (!destString[count1])
			break;

		count1 += 1;
	}

	// If this is true, then we possibly have an unterminated string constant.
	// Checking for a string that exceeds MAXSTRINGLENGTH will help to prevent
	// the function from running off too far into memory.
	if (count1 >= MAX_STRING_LENGTH)
	{
		//errno = ERR_BOUNDS;
		return (destString = NULL);
	}

	// Now copy the source string into the dest until the source is a NULL
	// character.
	for (count2 = 0; count1 < MAX_STRING_LENGTH; count1++, count2++)
	{
		destString[count1] = sourceString[count2];

		if (!sourceString[count2])
			break;
	}

	return (destString);
}

char* strncat(char* destString, const char* sourceString, size_t maxLength)
{
	unsigned count1, count2;
	int endFlag = 0;
	char sourceChar;

	// Find the end of the first String
	for (count1 = 0; count1 < MAX_STRING_LENGTH;)
	{
		if (destString[count1] == '\0')
			break;
		else
			count1++;
	}

	// If this is true, then we probably have an unterminated string
	// constant.  Checking for a string that exceeds MAXSTRINGLENGTH will
	// help to prevent the function from running off too far into memory.
	if (count1 >= MAX_STRING_LENGTH)
	{
		//errno = ERR_BOUNDS;
		return (destString = NULL);
	}

	// Now copy the source string into the dest.  If source is shorter than
	// maxLength, pad dest with NULL characters.
	for (count2 = 0; count2 < maxLength;)
	{
		if ((sourceString[count2] == '\0') || (endFlag == 1))
		{
			endFlag = 1;
			sourceChar = (char)NULL;
		}
		else
		{
			sourceChar = sourceString[count2];
		}

		destString[count1] = sourceChar;
		count1++; count2++;
	}

	// Make sure there's a NULL at the end
	destString[count1] = NULL;

	// Return success
	return (destString);
}

char* strdup(const char* c) {
	char* out = (char*)0; // kmalloc(strlen(c) + 1);
	//memcpy(out, (void*)c, strlen(c) + 1);
	return out;
}


char* strstr(const char* s1, const char* s2) {
	int count = 0;
	int s2Len = strlen(s2);

	for (count = 0; s1[0]; count++) {
		if (!strncmp(s1, s2, s2Len))
			return ((char*)s1);
		else
			s1++;
	}

	return NULL;
}