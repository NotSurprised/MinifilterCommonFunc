/*
//=================usage sample====================
UNICODE_STRING ProcessImageName;
WCHAR *sourceProcessImageName;
sourceProcessImageName = ExAllocatePoolWithTag(NonPagedPool, 512, 0);
ProcessImageName.Buffer = sourceProcessImageName;
ProcessImageName.Length = 0x0;
ProcessImageName.MaximumLength = sizeof(sourceProcessImageName);
GetProcessImageName(&ProcessImageName);
*/
NTSTATUS GetProcessImageName(_Inout_ PUNICODE_STRING ProcessImageName)
{
	NTSTATUS status;
	ULONG returnedLength;
	ULONG bufferLength;
	PVOID buffer;
	PUNICODE_STRING imageName;

	PAGED_CODE(); // this eliminates the possibility of the IDLE Thread/Process

	if (NULL == ZwQueryInformationProcess) {

		UNICODE_STRING routineName;

		RtlInitUnicodeString(&routineName, L"ZwQueryInformationProcess");

		ZwQueryInformationProcess =
			(QUERY_INFO_PROCESS)MmGetSystemRoutineAddress(&routineName);

		if (NULL == ZwQueryInformationProcess) {
			DbgPrint("Cannot resolve ZwQueryInformationProcess\n");
		}
	}
	//
	// Step one - get the size we need
	//
	status = ZwQueryInformationProcess(NtCurrentProcess(),
		ProcessImageFileName,
		NULL, // buffer
		0, // buffer size
		&returnedLength);

	if (STATUS_INFO_LENGTH_MISMATCH != status) {
		DbgPrint("STATUS_INFO_LENGTH_MISMATCH\n");
		return status;

	}

	//
	// Is the passed-in buffer going to be big enough for us?  
	// This function returns a single contguous buffer model...
	//
	bufferLength = returnedLength - sizeof(UNICODE_STRING);

	if (ProcessImageName->MaximumLength < bufferLength) {

		ProcessImageName->Length = (USHORT)bufferLength;
		DbgPrint("STATUS_BUFFER_OVERFLOW\n");
		return STATUS_BUFFER_OVERFLOW;

	}

	//
	// If we get here, the buffer IS going to be big enough for us, so 
	// let's allocate some storage.
	//
	buffer = ExAllocatePoolWithTag(PagedPool, returnedLength, 'ipgD');

	if (NULL == buffer) {
		DbgPrint("STATUS_INSUFFICIENT_RESOURCES\n");
		return STATUS_INSUFFICIENT_RESOURCES;

	}

	//
	// Now lets go get the data
	//
	status = ZwQueryInformationProcess(NtCurrentProcess(),
		ProcessImageFileName,
		buffer,
		returnedLength,
		&returnedLength);

	if (NT_SUCCESS(status)) {
		//
		// Ah, we got what we needed
		//	
		imageName = (PUNICODE_STRING)buffer;
		RtlCopyUnicodeString(ProcessImageName, imageName);
	}

	//
	// free our buffer
	//
	ExFreePool(buffer);

	//
	// And tell the caller what happened.
	//    
	return status;
}

