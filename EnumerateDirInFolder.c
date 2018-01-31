/*
//=================declare filterData in FltCreate sample====================
typedef struct _FILTER_DATA
{
    //
    //  The object that identifies this driver.
    //
    
    PDRIVER_OBJECT DriverObject;
    
    //
    //  The filter handle that results from a call to
    //  FltRegisterFilter.
    //
    
    PFLT_FILTER Filter;
    
    PFLT_INSTANCE VolumeInstance;
    ULONG VolumeInstanceCnt;

}FILTER_DATA, *PFILTER_DATA;
    
FILTER_DATA filterData;
*/
NTSTATUS EnumerateDirInFolder()
{
	PVOID pBuffer = NULL;
	UNICODE_STRING DirectoryName;
	OBJECT_ATTRIBUTES DirectoryAttributes;
	NTSTATUS Status = STATUS_SUCCESS;
	HANDLE DirectoryHandle = NULL;
	PFILE_OBJECT DirectoryFileObject = NULL;
	PFLT_VOLUME DirectoryVolume = NULL;
	PFLT_INSTANCE DirectoryInstance = NULL;
	IO_STATUS_BLOCK DirectoryIoStatusBlock;
	PFILE_BOTH_DIR_INFORMATION DirInformation;
	unsigned long dwSizeOfBuffer = 2048;
	LARGE_INTEGER CurrentTime;
	KeQuerySystemTime(&CurrentTime);

	RtlInitUnicodeString(&DirectoryName, L"\\SystemRoot\\Folder");
	KdPrint(("FOLDER_DIRECTORY name: %wZ\n", DirectoryName));
	InitializeObjectAttributes(&DirectoryAttributes, &DirectoryName,OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	
	Status = FltCreateFileEx(
		filterData.Filter,
		NULL,
		&DirectoryHandle,
		&DirectoryFileObject,
		(FILE_READ_DATA | FILE_LIST_DIRECTORY | FILE_READ_ATTRIBUTES | SYNCHRONIZE),
		&DirectoryAttributes,
		&DirectoryIoStatusBlock,
		NULL,
		NULL,
		(FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE),		// FULL sharing
		FILE_OPEN,				// MUST	already exist
		FILE_DIRECTORY_FILE,	// MUST be a directory
		NULL,
		0,
		IO_NO_PARAMETER_CHECKING
	);

	if (!NT_SUCCESS(Status)) {
		KdPrint(("Unable to open %.*S, error = 0x%x\n", DirectoryName.Length / sizeof(WCHAR), DirectoryName.Buffer, Status));
		return Status;
	}

	pBuffer = ExAllocatePoolWithTag(NonPagedPool, dwSizeOfBuffer, RS_POOL_TAG);

	if (pBuffer == NULL) {
		KdPrint(("Buffer cannot get memory\r\n"));
	}
	
	//
	// We pass NO NAME which is the same as *.*
	//
	Status = FltGetVolumeFromFileObject(iclGuardData.Filter, DirectoryFileObject, &DirectoryVolume);

	if (!NT_SUCCESS(Status))
	{
		KdPrint(("get voume failed: %x\n", Status));
		return Status;
	}

	Status = FltGetVolumeInstanceFromName(
		iclGuardData.Filter,
		DirectoryVolume,
		NULL,
		&DirectoryInstance
	);

	if (!NT_SUCCESS(Status))
	{
		KdPrint(("get instance failed: %x\n", Status));
		return Status;
	}

	BOOLEAN isDir = FALSE;
	FltIsDirectory(DirectoryFileObject, DirectoryInstance, &isDir);
	
	Status = FltQueryDirectoryFile(
		DirectoryInstance,
		DirectoryFileObject,
		pBuffer,
		dwSizeOfBuffer,
		FileBothDirectoryInformation,
		TRUE,
		NULL,
		FALSE,
		NULL
	);

	if (!NT_SUCCESS(Status)) {
		KdPrint(("Unable to query directory contents, error 0x%x\n", Status));
		return Status;
	}
	KdPrint(("Directory List : \n"));
	DirInformation = (PFILE_BOTH_DIR_INFORMATION)pBuffer;
	// Loop over all files
	for (;;)
	{
		//
		// Dump the full name of the file.  We could dump the other information
		// here as well, but we'll keep the example shorter instead.
		//
		KdPrint(("  %.*S\n", DirInformation->FileNameLength / sizeof(WCHAR), &DirInformation->FileName[0]));

		KdPrint(("  %d  %d\n", CurrentTime.HighPart, DirInformation->CreationTime.HighPart));
		//
		// If there is no offset in the entry, the buffer has been exhausted.
		//
		if (DirInformation->NextEntryOffset == 0)
		{
			// Re-fill buffer
			Status = FltQueryDirectoryFile(
				DirectoryInstance,
				DirectoryFileObject,
				pBuffer,
				dwSizeOfBuffer,
				FileBothDirectoryInformation,
				TRUE,
				NULL,
				FALSE,
				NULL
			);
			if (!NT_SUCCESS(Status))
			{
				if (Status == STATUS_NO_MORE_FILES)
					break;
				KdPrint(("Unable to query directory contents,error 0x%x\n", Status));
				return Status;
			}

			DirInformation = (PFILE_BOTH_DIR_INFORMATION)pBuffer;
			continue;
		}
		//
		// Advance to the next entry.
		//
		DirInformation = (PFILE_BOTH_DIR_INFORMATION)(((PUCHAR)DirInformation) + DirInformation->NextEntryOffset);

	}
	if (DirectoryHandle)
	{
		FltClose(DirectoryHandle);
	}

	if (DirectoryVolume)
	{
		FltObjectDereference(DirectoryVolume); 
	}
	if (DirectoryInstance)
	{
		FltObjectDereference(DirectoryInstance);
	}
	return Status;
}
