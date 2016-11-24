#ifndef _NT_H_
#define _NT_H_

#include <windows.h>
#include <ntstatus.h>
#include <Winternl.h>

namespace nt {

#define DDKAPI

typedef ::SYSTEM_PROCESS_INFORMATION SYSTEM_PROCESS_INFORMATION;
typedef SYSTEM_PROCESS_INFORMATION SYSTEM_PROCESSES;

typedef struct _SYSTEM_HANDLE_ENTRY {
	ULONG  OwnerPid;
	BYTE   ObjectType;
	BYTE   HandleFlags;
	USHORT HandleValue;
	PVOID  ObjectPointer;
	ULONG  AccessMask;
} SYSTEM_HANDLE_ENTRY, *PSYSTEM_HANDLE_ENTRY;

typedef struct _SYSTEM_HANDLE_INFORMATION {
	ULONG               Count;
	SYSTEM_HANDLE_ENTRY Handle[1];
} SYSTEM_HANDLE_INFORMATION, *PSYSTEM_HANDLE_INFORMATION;

typedef NTSTATUS NTAPI (* NtQuerySystemInformationType)(
	/*IN*/ SYSTEM_INFORMATION_CLASS  SystemInformationClass,
	/*IN OUT*/ PVOID  SystemInformation,
	/*IN*/ ULONG  SystemInformationLength,
	/*OUT*/ PULONG  ReturnLength  /*OPTIONAL*/);

typedef NTSTATUS NTAPI (* NtQueryObjectType)(
	/*IN*/ HANDLE  ObjectHandle,
	/*IN*/ OBJECT_INFORMATION_CLASS  ObjectInformationClass,
	/*OUT*/ PVOID  ObjectInformation,
	/*IN*/ ULONG  ObjectInformationLength,
	/*OUT*/ PULONG  ReturnLength  /*OPTIONAL*/);
  
typedef VOID DDKAPI (* RtlInitAnsiStringType)(
	/*IN OUT*/ PANSI_STRING  DestinationString,
	/*IN*/ PCSZ  SourceString);

typedef NTSTATUS DDKAPI (* RtlUnicodeStringToAnsiStringType)(
	/*IN OUT*/ PANSI_STRING  DestinationString,
	/*IN*/ PUNICODE_STRING  SourceString,
	/*IN*/ BOOLEAN  AllocateDestinationString);

typedef VOID DDKAPI (* RtlFreeAnsiStringType)(
	/*IN*/ PANSI_STRING  AnsiString);

}

#endif
