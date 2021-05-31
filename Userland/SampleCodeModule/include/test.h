#ifndef TEST_H
#define TEST_H
#include "test_util.h"
#include "shell.h"
#include "stdlib.h"
extern void _waitChildren();
void test_mm();
void test_prio();
void test_processes();
void test_sync();
void test_no_sync();
#endif