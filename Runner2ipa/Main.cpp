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


void copyToTemp(fs::path &path, fs::path* pTempPath)
{
	//Do our work in %temp% folder
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


void process(const fs::path &tempPath)
{
    static const fs::path s_runnerAppZip = tempPath.parent_path() /   L"Runner.app.zip";
    static const fs::path s_payloadZip = tempPath.parent_path() /     L"Payload.zip";
    static const fs::path s_payloadIpa = tempPath.parent_path() /     L"Payload.ipa";

    CComBSTR fileToUnzip(s_runnerAppZip.string().c_str());
    CComBSTR folderToUnzipTo(tempPath.parent_path().string().c_str());

    CComBSTR folderToZip(tempPath.string().c_str());
    CComBSTR folderToZipTo(s_payloadZip.string().c_str());


    std::cout << "Processing..." << std::endl;

    fs::rename(tempPath, s_runnerAppZip);                                   //Runner.app to Runner.app.zip

    if (!Unzip2Folder(fileToUnzip, folderToUnzipTo))                        //Unzip Runner.app.zip to Runner.app (Folder)
    {
        throw std::exception("Unzipping failed");
    }

    if (!zipFolder(folderToZip, folderToZipTo, s_payloadZip.string()))      //Zip folder Runner.app into Payload.zip
    {
        throw std::exception("Zipping failed");
    }

    fs::rename(s_payloadZip, s_payloadIpa);                                 //Rename Payload.zip to Payload.ipa
}


void cleanUp(const fs::path &path, const fs::path &tempPath)
{
	static const fs::path s_payloadIpa = tempPath.parent_path() / L"Payload.ipa";

	//If a Payload.ipa file was successfully created move it back
	if (fs::exists(s_payloadIpa))
	{
		fs::rename(s_payloadIpa, path.parent_path() / s_payloadIpa.filename());
	}

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
        }
        else
        {
            path = argv[1];

            if (fs::is_directory(path))
            {
                throw std::invalid_argument("Expected file but got directory - " + path.string());
            }
        }

        if (path.filename() != L"Runner.app")
        {
            throw std::invalid_argument("Expected Runner.app but got - " + path.filename().string());
        }
		else if (!fs::exists(path))
		{
			throw std::invalid_argument("File doesn't exist - " + path.string());
		}
        
		copyToTemp(path, &tempPath);
        process(tempPath);
    }
    catch (std::exception ex)
    {
        std::cout << "Error: " << ex.what() << std::endl;
        exitCode = EXIT_FAILURE;
    }

    cleanUp(path, tempPath);
    std::cout << "Exiting in 3 Seconds" << std::endl;
    Sleep(3000);

    return exitCode;
}
