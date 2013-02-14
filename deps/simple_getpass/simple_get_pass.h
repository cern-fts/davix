#pragma once
#ifndef SIMPLE_GET_PASS_H
#define SIMPLE_GET_PASS_H

#include<stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

ssize_t simple_get_pass(char** passwd);


void decimate_passwd(char* p);

#ifdef __cplusplus
}
#endif

#endif // SIMPLE_GET_PASS_H
