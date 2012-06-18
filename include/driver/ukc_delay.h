#pragma once


#ifdef __cplusplus
extern "C" {
#endif


void ukc_ssleep(unsigned int seconds);

void ukc_mssleep(unsigned int milliseconds);

void ukc_ussleep(unsigned int microseconds);

void ukc_nssleep(unsigned int nanoseconds);




#ifdef __cplusplus
}
#endif
