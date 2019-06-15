#include <iostream>
#include <fstream>
#include <atlbase.h>
#include <filesystem>

#include "zip.h"
#include "unzip.h"

namespace fs = std::experimental::filesystem;


void getFile(fs::path &path)
{
    //Open file explorer and set path to the path of the selected file
    OPENFILENAME fileName;
    char szFile[100];

    ZeroMemory(&fileName, sizeof(fileName));
    fileName.lStructSize = sizeof(fileName);
    fileName.hwndOwner = NULL;
    fileName.lpstrFile = szFile;
    fileName.lpstrFile[0] = '\0';
    fileName.nMaxFile = sizeof(szFile);
    fileName.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
    fileName.nFilterIndex = 1;
    fileName.lpstrFileTitle = NULL;
    fileName.nMaxFileTitle = 0;
    fileName.lpstrInitialDir = NULL;
    fileName.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    GetOpenFileName(&fileName);
    path = fileName.lpstrFile;
}


void process(const fs::path &path)
{
    static const fs::path s_runnerAppZip = path.parent_path() /   "Runner.app.zip";
    static const fs::path s_payloadZip = path.parent_path() /     "Payload.zip";
    static const fs::path s_payloadIpa = path.parent_path() /     "Payload.ipa";

    CComBSTR fileToUnzip(s_runnerAppZip.string().c_str());
    CComBSTR folderToUnzipTo(path.parent_path().string().c_str());

    CComBSTR folderToZip(path.string().c_str());
    CComBSTR folderToZipTo(s_payloadZip.string().c_str());


    std::cout << "Processing..." << std::endl;

    fs::rename(path, s_runnerAppZip);                                       //Runner.app to Runner.app.zip

    if (!Unzip2Folder(fileToUnzip, folderToUnzipTo))                        //Unzip Runner.app.zip to Runner.app (Folder)
    {
        throw std::exception("Unzipping failed");
    }

    if (!zipFolder(folderToZip, folderToZipTo, s_payloadZip.string()))      //Zip folder Runner.app into Payload.zip
    {
        throw std::exception("Zipping failed");
    }

    fs::rename(s_payloadZip, s_payloadIpa);                                 //Rename Payload.zip to Payload.ipa

    fs::remove_all(path);                                                   //Remove folder Runner.app

    fs::rename(s_runnerAppZip, path);                                       //Rename Runner.app.zip back to Runner.app
}


void cleanUp(const fs::path &path)
{
    //Put everything back to the way we found it (Except for Payload.ipa)

    static const fs::path s_runnerAppZip = path.parent_path() /   "Runner.app.zip";
    static const fs::path s_payloadZip = path.parent_path() /     "Payload.zip";

    if (fs::exists(path) && fs::is_directory(path))
    {
        fs::remove_all(path);
    }

    if (fs::exists(s_runnerAppZip) && !fs::exists(path))
    {
        fs::rename(s_runnerAppZip, path);
    }
    
    if (fs::exists(s_payloadZip))
    {
        fs::remove_all(s_payloadZip);
    }
}


int main(int argc, char* argv[])
{
    auto exitCode = EXIT_SUCCESS;
    std::cout << "Convert Runner.app to Payload.ipa" << std::endl;

    fs::path path;
    try
    {
        if (argc == 1)
        {
            getFile(path);
        }
        else
        {
            path = argv[1];

            if (fs::is_directory(path))
            {
                throw std::invalid_argument("Expected file but got directory - " + path.string());
            }
        }

        if (path.filename() != "Runner.app")
        {
            throw std::invalid_argument("Expected Runner.app but got - " + path.filename().string());
        }
        
        process(path);
    }
    catch (std::exception ex)
    {
        std::cout << "Error: " << ex.what() << std::endl;
        exitCode = EXIT_FAILURE;
    }

    cleanUp(path);
    std::cout << "Exiting in 3 Seconds" << std::endl;
    Sleep(3000);

    return exitCode;
}
