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

typedef vector<tuple<wstring, DWORD>> Enumeration;

int enumerateProcesses(Enumeration &enumeration) 
{
	DWORD aProcessIds[1024], bytesReturned;
	int i;

	if (!EnumProcesses(aProcessIds, sizeof(aProcessIds), &bytesReturned))
	{
		return GetLastError();
	}

	for (i = 0; i < bytesReturned / sizeof(DWORD); i++)
	{
		if(aProcessIds[i] != 0)
		{
			wchar_t processName[MAX_PATH] = L"<unknown>";

			HANDLE hProcess = OpenProcess(
				PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
				FALSE,
				aProcessIds[i]
			);

			if (hProcess != NULL) {
				HMODULE hModule;
				
				if (EnumProcessModules(hProcess, &hModule, sizeof(hModule), &bytesReturned)) 
				{
					GetModuleFileNameExW(
						hProcess, 
						hModule, 
						processName, 
						sizeof(processName) / sizeof(wchar_t)
					);
				}

				enumeration.push_back(make_tuple(wstring(processName), aProcessIds[i]));
			}
		}
	}

	return 0;
}

Local<Array> mapResult(Enumeration &enumeration)
{
	Local<Array> result = New<Array>(enumeration.size());

	char buffer[MAX_PATH * 2];

	for (Enumeration::iterator it = enumeration.begin(); it != enumeration.end(); ++it)
	{
		wcstombs(buffer, get<0>(*it).c_str(), sizeof(buffer));

		Local<Object> process = New<Object>();

		Nan::Set(
			process,
			New("path").ToLocalChecked(),
			New(buffer).ToLocalChecked()
		);

		Nan::Set(
			process,
			New("pid").ToLocalChecked(),
			New((uint32_t)get<1>(*it))
		);

		Nan::Set(
			result,
			New((uint32_t)(it - enumeration.begin())),
			process
		);
	}

	return result;
}

NAN_METHOD(EnumerateProcessesSync) 
{
	vector<tuple<wstring, DWORD>> enumeration;

	int errno = enumerateProcesses(enumeration);

	if (errno) {
		return ThrowError(ErrnoException(errno));
	}

	info.GetReturnValue().Set(mapResult(enumeration));
}

class EnumerateProcessesWorker : public AsyncWorker {
	public:
		EnumerateProcessesWorker(Nan::Callback *callback) : AsyncWorker(callback), err(0), enumeration() {}
		~EnumerateProcessesWorker() {}

		void Execute () {
			err = enumerateProcesses(enumeration);
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
		int err;
		Enumeration enumeration;
};

NAN_METHOD(EnumerateProcessesAsync) {
	if (info[0]->IsFunction()) {
		return AsyncQueueWorker(new EnumerateProcessesWorker(
			new Nan::Callback(info[0].As<Function>())
		));
	}	

	ThrowTypeError("Expected callback");
}