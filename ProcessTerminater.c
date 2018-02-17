/*
//=================usaging sample====================
TerminateProcess((ULONG)PsGetCurrentProcess());
*/
NTSTATUS TerminateProcess(ULONG ProcessId)
{
	NTSTATUS status;
	PEPROCESS Process = { 0 };

	status = PsLookupProcessByProcessId((HANDLE)ProcessId, Process);

	if (!NT_SUCCESS(status))
	{
		KdPrint(("Icrl!TerminateProcess:  PsLookupProcessByProcessId failed. %x\r\n", status));
		return status;
	}

	__try {
		KeAttachProcess(Process);

		status = ZwTerminateProcess(NtCurrentProcess(), NULL);

		if (!NT_SUCCESS(status))
		{
			KdPrint(("Icrl!TerminateProcess:  ZwTerminateProcess failed. %x\r\n", status));
			return status;
		}

		KeDetachProcess();
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}

	return status;
}