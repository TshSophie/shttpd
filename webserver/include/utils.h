#ifndef __UTILS_H_
#define __UTILS_H_

const char *get_file_type(const char *name);
int hexit(char c);
void decode_str(char *to, char *from);
void encode_str(char* to, size_t tosize, const char* from);

#endif