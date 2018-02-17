/*
//=================Just a note====================
*/
NTSTATUS FillOutUnicodeString(_In_ WCHAR *tempString, _Inout_ UNICODE_STRING tagetUnicodeString)
{
	tempString = ExAllocatePoolWithTag(NonPagedPool, 1024, 0);
	RtlStringCbPrintfW(tempString, 1024 * sizeof(WCHAR), L"%ws%wZ%d", WCHAR test, UnicodeString  test2, PsGetProcessId(PsGetCurrentProcess()));
	RtlInitUnicodeString(&tagetUnicodeString, tempString);
}
