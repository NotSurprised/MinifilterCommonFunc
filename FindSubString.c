/*
//=================Just a note====================
*/
BOOLEAN FindSubString(_In_ PUNICODE_STRING String, _In_ PUNICODE_STRING SubString)
{
	ULONG index;

	if (RtlEqualUnicodeString(String, SubString, TRUE)) 
	{
		return TRUE;
	}

	for (index = 0; index + (SubString->Length / sizeof(WCHAR)) <= (String->Length / sizeof(WCHAR));index++) 
	{
		if (_wcsnicmp(&String->Buffer[index], SubString->Buffer, (SubString->Length / sizeof(WCHAR))) == 0) 
		{
			return TRUE;
		}
	}
	return FALSE;
}