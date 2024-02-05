#ifndef TEXT_H_
#define TEXT_H_

#include "functions.h"
#include <stdbool.h>

// Utility functions for characters and strings

char to_upper(char c);
bool is_digit(char c);
bool is_letter(char c);
bool in_range(char c, char min, char max);
bool starts_with(const char *string, const char *prefix);
bool equals(const char *string1, const char *string2);

int char_index(const char *string, char c);

// Parse a 2 digit integer from string
int dd_to_int(const char *str, int length);

// Convert a 2 digit integer to string
void int_to_dd(char *str, int value, int width, bool full_sign);

char charn(int c, int table_idx);
#endif