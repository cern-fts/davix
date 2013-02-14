#pragma once
#ifndef SIMPLE_GET_PASS_H
#define SIMPLE_GET_PASS_H


#ifdef __cplusplus
extern "C" {
#endif

int simple_get_pass(char* passwd, size_t max_size);


void decimate_passwd(char* p);

#ifdef __cplusplus
}
#endif

#endif // SIMPLE_GET_PASS_H
