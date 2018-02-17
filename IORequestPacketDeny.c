/*
//=================Just a note====================
*/
NTSTATUS IORequestPacketDeny(_In_ PFLT_CALLBACK_DATA Data, _In_ PCFLT_RELATED_OBJECTS FltObjects)
{
	Data->Iopb->MajorFunction = IRP_MJ_CLOSE;
	Data->IoStatus.Information = 0;
	Data->IoStatus.Status = STATUS_ACCESS_DENIED;
	Data->Iopb->TargetFileObject->FileName.Buffer = NULL;
	FLT_SET_CALLBACK_DATA_DIRTY(Data);
	return   FLT_PREOP_COMPLETE;
}