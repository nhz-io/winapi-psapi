#include "windows.h"

#include "psapi.h"

NAN_MODULE_INIT(InitAll) {
	Nan::Set(
		target,
		Nan::New("enumerateProcessesAsync").ToLocalChecked(),
		Nan::GetFunction(Nan::New<v8::FunctionTemplate>(EnumerateProcessesAsync)).ToLocalChecked()
	);

	Nan::Set(
		target,
		Nan::New("enumerateProcessesSync").ToLocalChecked(),
		Nan::GetFunction(Nan::New<v8::FunctionTemplate>(EnumerateProcessesSync)).ToLocalChecked()
	);

    Nan::Set(
        target,
        Nan::New("enumerateModulesSync").ToLocalChecked(),
        Nan::GetFunction(Nan::New<v8::FunctionTemplate>(EnumerateModulesSync)).ToLocalChecked()
    );

	Nan::Set(
        target,
        Nan::New("enumerateModulesAsync").ToLocalChecked(),
        Nan::GetFunction(Nan::New<v8::FunctionTemplate>(EnumerateModulesAsync)).ToLocalChecked()
    );
}

NODE_MODULE(psapi, InitAll)