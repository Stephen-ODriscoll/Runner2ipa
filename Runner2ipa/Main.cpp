#include <iostream>
#include <fstream>
#include <atlbase.h>
#include <filesystem>

#include "zip.h"
#include "unzip.h"

namespace fs = std::experimental::filesystem;


void getFile(fs::path &path)
{
    // Open file explorer and set path to the path of the selected file
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


void copyToTemp(const fs::path &path, fs::path* pTempPath)
{
    // Do our work in %temp% folder
    char* envVarValue = nullptr;
    size_t size = 0;
    
    _dupenv_s(&envVarValue, &size, "TEMP");
    fs::path tempFolder = std::string(envVarValue, size) / fs::path("Runner2ipa");

    if (!envVarValue)
    {
        throw std::exception("Failed to get environment variable TEMP");
    }

    fs::create_directories(tempFolder);
    fs::copy(path, tempFolder);

    *pTempPath = tempFolder / path.filename();
}


void process(const fs::path &path, const fs::path &tempPath)
{
    const fs::path runnerAppZip = tempPath.parent_path() /  "Runner.app.zip";
    const fs::path payload = tempPath.parent_path() /       "Payload";
    const fs::path payloadZip = tempPath.parent_path() /    "Payload.zip";
    const fs::path payloadIpa = path.parent_path() /        "Payload.ipa";

    CComBSTR fileToUnzip(runnerAppZip.string().c_str());
    CComBSTR folderToUnzipTo(tempPath.parent_path().string().c_str());

    CComBSTR folderToZip(payload.string().c_str());
    CComBSTR folderToZipTo(payloadZip.string().c_str());


    std::cout << "Processing..." << std::endl;

    fs::rename(tempPath, runnerAppZip);                                 // Rename Runner.app to Runner.app.zip

    if (!Unzip2Folder(fileToUnzip, folderToUnzipTo))                    // Unzip Runner.app.zip to Runner.app (Folder)
    {
        throw std::exception("Unzipping failed");
    }

    fs::create_directories(payload);                                    // Create folder called Payload
    fs::rename(tempPath, payload / tempPath.filename());                // Move Runner.app into Payload

    if (!zipFolder(folderToZip, folderToZipTo, payloadZip.string()))    // Zip folder Payload into Payload.zip
    {
        throw std::exception("Zipping failed");
    }

    fs::rename(payloadZip, payloadIpa);                                 // Rename Payload.zip to Payload.ipa and Move
}


void removeParent(const fs::path &tempPath)
{
    //Delete our temporary working folder
    if (fs::exists(tempPath.parent_path()))
    {
        fs::remove_all(tempPath.parent_path());
    }
}


int main(int argc, char* argv[])
{
    auto exitCode = EXIT_SUCCESS;
    std::cout << "Convert Runner.app to Payload.ipa" << std::endl;

    fs::path path;
    fs::path tempPath;

    try
    {
        if (argc == 1)
        {
            getFile(path);

            if (path.filename().empty())
            {
                // User clicked cancel
                return EXIT_SUCCESS;
            }
        }
        else
        {
            path = argv[1];

            if (!fs::exists(path))
            {
                throw std::invalid_argument("File doesn't exist - " + path.string());
            }
            else if (fs::is_directory(path))
            {
                throw std::invalid_argument("Expected file but got directory - " + path.string());
            }
        }

        if (path.filename() != "Runner.app")
        {
            throw std::invalid_argument("Expected Runner.app but got - " + path.filename().string());
        }
        
        copyToTemp(path, &tempPath);
        process(path, tempPath);
    }
    catch (std::exception ex)
    {
        std::cout << "Error: " << ex.what() << "\n";
        std::cout << "Exiting in 5 seconds" << std::endl;

        exitCode = EXIT_FAILURE;
        Sleep(5000);
    }

    removeParent(tempPath);
    return exitCode;
}
