#ifndef PSAPI_GRAB_H
#define PSAPI_GRAB_H

#include <nan.h>

NAN_METHOD(EnumerateProcessesAsync);
NAN_METHOD(EnumerateProcessesSync);

NAN_METHOD(EnumerateModulesAsync);
NAN_METHOD(EnumerateModulesSync);

#endif