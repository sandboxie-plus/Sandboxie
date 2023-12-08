#pragma once

struct SVolumeInfo
{
    std::wstring volumeName;
    std::wstring deviceName;
    std::vector<std::wstring> mountPoints;
    //int parNumb = 0;                   // partition number
    //unsigned int dskType = 0;          // disk type
    struct SDisk 
    {
        std::wstring deviceName;
        unsigned int dskNumber = 0;    // disk number
        unsigned int dskType = 0;      // disk type
        //unsigned __int64 prtStart = 0; // partition start position in disk
        //unsigned __int64 prtSize = 0;  // partition size in disk
    };
    std::vector<SDisk> disks;
};

std::list<SVolumeInfo> ListAllVolumes();

struct SDriveInfo
{
    std::wstring Name;
    std::wstring Path;
    std::wstring Enum;
};

std::map<std::wstring, SDriveInfo> ListAllDrives();