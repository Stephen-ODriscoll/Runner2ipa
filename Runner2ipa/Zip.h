#pragma once
#include <comutil.h>
#include <Shldisp.h>
#include <string.h>
#include <fstream>

bool zipFolder(BSTR source, BSTR dest, std::string destString)
{
    // Create Empty Zip file
    FILE* f;
    fopen_s(&f, destString.c_str(), "wb");
    fwrite("\x50\x4B\x05\x06\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 22, 1, f);
    fclose(f);

    long FilesCount = 0;
    IShellDispatch *pISD;
    Folder *pToFolder = 0L;
    FolderItems *pFilesInside = 0L;
    VARIANT vDir, vFile, vOpt;

    CoInitialize(NULL);
    if (CoCreateInstance(CLSID_Shell, NULL, CLSCTX_INPROC_SERVER, IID_IShellDispatch, (void **)&pISD) != S_OK)
        return 1;

    vDir.vt = VT_BSTR;
    vDir.bstrVal = dest;

    pISD->NameSpace(vDir, &pToFolder);
    if (!pToFolder)
    {
        pISD->Release();
        return 1;
    }

    vFile.vt = VT_BSTR;
    vFile.bstrVal = source;

    vOpt.vt = VT_I4;
    vOpt.lVal = FOF_NO_UI;

    // Copying and compressing the source files to our zip
    bool retval = pToFolder->CopyHere(vFile, vOpt) == S_OK;

    // Sleep until the number of files in the folder isn't 0 anymore or timeout after 30 seconds
    for (int i = 0; FilesCount == 0; ++i)
    {
        pToFolder->Items(&pFilesInside);
        if (!pFilesInside || i > 60)
        {
            pToFolder->Release();
            pISD->Release();
            return 1;
        }

        pFilesInside->get_Count(&FilesCount);
        Sleep(500);
    }

    pToFolder->Release();
    pISD->Release();
    CoUninitialize();

    return retval;
}
