#include <vector>
#include <tuple>
#include <string>

#include "windows.h"

#include <v8.h>
#include <nan.h>

using namespace std;

using v8::Function;
using v8::Local;
using v8::Number;
using v8::Value;
using v8::Array;
using v8::Object;

using Nan::AsyncQueueWorker;
using Nan::AsyncWorker;
using Nan::Callback;
using Nan::HandleScope;
using Nan::New;
using Nan::ErrnoException;
using Nan::ThrowError;
using Nan::ThrowTypeError;
using Nan::Null;

typedef vector<wstring> Enumeration;

int enumerateModules(int pid, Enumeration &enumeration) 
{
    HMODULE hModules[1024];
    HANDLE hModule;
    DWORD bytesReturned;
    int i;

    hModule = OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
        FALSE,
        pid
    );

    if (hModule == NULL) {
        return GetLastError();
    }

    if (!EnumProcessModules(hModule, hModules, sizeof(hModules), &bytesReturned)) 
    {
        return GetLastError();
    }

    for (i = 0; i < bytesReturned / sizeof(HMODULE); i++) {
        wchar_t moduleName[MAX_PATH] = L"<unknown>";

        GetModuleFileNameEx(
            hModule, 
            hModules[i], 
            moduleName, 
            sizeof(moduleName) / sizeof(wchar_t)
        );

        enumeration.push_back(wstring(moduleName));
    }

    return 0;
}

Local<Array> mapResult(Enumeration &enumeration)
{
	Local<Array> result = New<Array>(enumeration.size());

	char buffer[MAX_PATH * 2];

	for (Enumeration::iterator it = enumeration.begin(); it != enumeration.end(); ++it)
	{
		wcstombs(buffer, (*it).c_str(), sizeof(buffer));

		Nan::Set(
			result,
			New((uint32_t)(it - enumeration.begin())),
			New(buffer).ToLocalChecked()
		);
	}

	return result;
}

NAN_METHOD(EnumerateModulesSync) 
{
	Enumeration enumeration;

	int err = enumerateModules(info[0]->Uint32Value(), enumeration);

	if (err) {
		return ThrowError(ErrnoException(err));
	}

	info.GetReturnValue().Set(mapResult(enumeration));
}

class EnumerateModulesWorker : public AsyncWorker {
	public:
		EnumerateModulesWorker(int pid, Nan::Callback *callback) : 
            AsyncWorker(callback), pid(pid), err(0), enumeration() {}
		~EnumerateModulesWorker() {}

		void Execute () {
			err = enumerateModules(pid, enumeration);
		}

		void HandleOKCallback () {
			if (err) {
				Local<Value> argv[] = {ErrnoException(err)};
				callback->Call(1, argv);
				return;
			}

			Local<Value> argv[] = {Null(), mapResult(enumeration)};

			callback->Call(2, argv);
		}

	private:
        int pid;
		int err;
		Enumeration enumeration;
};

NAN_METHOD(EnumerateModulesAsync) {
	if (info[1]->IsFunction()) {
		return AsyncQueueWorker(new EnumerateModulesWorker(
            info[0]->Uint32Value(),
			new Callback(info[1].As<Function>())
		));
	}	

	ThrowTypeError("Expected callback");
}