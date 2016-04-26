// CR2Reader.cpp : Defines the entry point for the console application.
// http://lclevy.free.fr/cr2/

#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <memory>
#include <map>
#include <set>
#include "stdafx.h"
#include "CR2Reader.h"

using std::cout;
using std::endl;

namespace
{
    const int s_tiff_header_offset = 0;
    const int s_ifd_entry_size = 12;

    template <typename T>
    void read_value(std::ifstream& file, T& value) {
        file.read((char*)&value, sizeof(T));
        if (!file)
            throw std::runtime_error("File error");
    }

    char* read_bytes(std::ifstream& file, int n) {
        char* value = new char[n];
        file.read(value, n);
        if (!file)
            throw std::runtime_error("File error");
        return value;
    }

    template <typename T>
    T read_value(std::ifstream& file) {
        T value;
        read_value<T>(file, value);
        return value;
    }

   




    std::map<int, const char*> s_canon_settings_tags = {
        { 1, "MacroMode" },
        { 2, "SelfTimer" },
        { 3, "Quality" },
        { 4, "CanonFlashMode" },
        { 5, "ContinuousDrive" },
        { 7, "FocusMode" },
        { 9, "RecordMode" },
        { 10, "CanonImageSize" },
        { 11, "EasyMode" },
        { 12, "DigitalZoom" },
        { 13, "Contrast" },
        { 14, "Saturation" },
        { 15, "Sharpness" },
        { 16, "CameraISO" },
        { 17, "MeteringMode" },
        { 18, "FocusRange" },
        { 19, "AFPoint" },
        { 20, "CanonExposureMode" },
        { 22, "LensType" },
        { 23, "MaxFocalLength" },
        { 24, "MinFocalLength" },
        { 25, "FocalUnits" },
        { 26, "MaxAperture" },
        { 27, "MinAperture" },
        { 28, "FlashActivity" },
        { 29, "FlashBits" },
        { 32, "FocusContinuous" },
        { 33, "AESetting" },
        { 34, "ImageStabilization" },
        { 35, "DisplayAperture" },
        { 36, "ZoomSourceWidth" },
        { 37, "ZoomTargetWidth" },
        { 39, "SpotMeteringMode" },
        { 40, "PhotoEffect" },
        { 41, "ManualFlashOutput" },
        { 42, "ColorTone" },
        { 46, "SRAWQuality" },
    };
    TableConfig s_canon_settings(&s_canon_settings_tags, 0);

    std::map<int, const char*> s_canon_maker_tags = {
        { 0x0000, "0x0000" },
        { 0x0001, "CameraSettings" },
        { 0x0002, "FocalLength" },
        { 0x0003, "0x0003" },
        { 0x0004, "ShotInfo" },
        { 0x0005, "Panorama" },
        { 0x0006, "ImageType" },
        { 0x0007, "FirmwareVersion" },
        { 0x0008, "FileNumber" },
        { 0x0009, "OwnerName" },
        { 0x000c, "SerialNumber" },
        { 0x000d, "CameraInfo" },
        { 0x000f, "CustomFunctions" },
        { 0x0010, "ModelID" },
        { 0x0012, "PictureInfo" },
        { 0x0013, "ThumbnailImageValidArea" },
        { 0x0015, "SerialNumberFormat" },
        { 0x001a, "SuperMacro" },
        { 0x0026, "AFInfo" },
        { 0x0083, "OriginalDecisionDataOffset" },
        { 0x00a4, "WhiteBalanceTable" },
        { 0x0095, "LensModel" },
        { 0x0096, "InternalSerialNumber" },
        { 0x0097, "DustRemovalData" },
        { 0x0099, "CustomFunctions" },
        { 0x00a0, "ProcessingInfo" },
        { 0x00aa, "MeasuredColor" },
        { 0x00b4, "ColorSpace" },
        { 0x00b5, "0x00b5" },
        { 0x00c0, "0x00c0" },
        { 0x00c1, "0x00c1" },
        { 0x00d0, "VRDOffset" },
        { 0x00e0, "SensorInfo" },
        { 0x4001, "ColorData" }
    };
    std::map<int, TableConfig*> s_canon_maker_children = {
        //{ 0x0001, &s_canon_settings }
    };
    TableConfig s_canon_maker(&s_canon_maker_tags, &s_canon_maker_children);

    
    std::map<int, const char*> s_exif_tags = {
        { 0x829a, "ExposureTime" },
        { 0x829d, "FNumber" },
        { 0x8822, "ExposureProgram" },
        { 0x8827, "ISOSpeedRatings" },
        { 0x9000, "ExifVersion" },
        { 0x9003, "DateTimeOriginal" },
        { 0x9004, "DateTimeDigitized" },
        { 0x9101, "ComponentConfiguration" },
        { 0x9102, "CompressedBitsPerPixel" },
        { 0x9201, "ShutterSpeedValue" },
        { 0x9202, "ApertureValue" },
        { 0x9203, "BrightnessValue" },
        { 0x9204, "ExposureBiasValue" },
        { 0x9205, "MaxApertureValue" },
        { 0x9206, "SubjectDistance" },
        { 0x9207, "MeteringMode" },
        { 0x9208, "LightSource" },
        { 0x9209, "Flash" },
        { 0x920a, "FocalLength" },
        { 0x927c, "MakerNote" },
        { 0x9286, "UserComment" },
        { 0xa000, "FlashPixVersion" },
        { 0xa001, "ColorSpace" },
        { 0xa002, "ExifImageWidth" },
        { 0xa003, "ExifImageHeight" },
        { 0xa004, "RelatedSoundFile" },
        { 0xa005, "ExifInteroperabilityOffset" },
        { 0xa20e, "FocalPlaneXResolution" },
        { 0xa20f, "FocalPlaneYResolution" },
        { 0xa210, "FocalPlaneResolutionUnit" },
        { 0xa217, "SensingMethod" },
        { 0xa300, "FileSource" },
        { 0xa301, "SceneType" }
    };
    std::map<int, TableConfig*> s_exif_children = {
        { 0x927c, &s_canon_maker }
    };
    TableConfig s_exif(&s_exif_tags, &s_exif_children);


    std::map<int, const char*> s_tiff_tags = {
        { 0x0100, "imageWidth" },
        { 0x0101, "imageLength" },
        { 0x0102, "bitsPerSample" },
        { 0x0103, "compression" },
        { 0x0106, "photometricInterpretation" },
        { 0x010f, "make" },
        { 0x0110, "model" },
        { 0x0112, "orientation" },
        { 0x0111, "stripOffset" },
        { 0x0115, "samplesPerPixel" },
        { 0x0116, "rowPerStrip" },
        { 0x0117, "stripByteCounts" },
        { 0x011c, "planarConfiguration" },
        { 0x011a, "xResolution" },
        { 0x011b, "yResolution" },
        { 0x0128, "resolutionUnit" },
        { 0x0132, "dateTime" },
        { 0x013b, "artist" },
        { 0x0201, "thumbnailOffset" },
        { 0x0202, "thumbnailLength" },
        { 0x02bc, "xmlPacket" },
        { 0x8298, "copyright" },
        { 0x8769, "exif" },
        { 0x8825, "gpsData" }
    };
    std::map<int, TableConfig*> s_tiff_children = {
        { 0x8769, &s_exif }
    };
    TableConfig s_tiff(&s_tiff_tags, &s_tiff_children);
}

TableConfig::TableConfig(std::map<int, const char*>* tt, std::map<int, TableConfig*>* nt)
: tag_translation(tt), nested_tables(nt)
{ }

TiffHeader::TiffHeader(std::ifstream& file, int location)
{
    file.seekg(location, std::ios::beg);
    read_value<TiffHeaderPod>(file, m_pod);
}

int TiffHeader::GetIfdLocation() const {
    return m_pod.tiff_ifd_offset;
}

IfdEntry::IfdEntry(std::ifstream& file, int location, TableConfig* config)
: m_file(file)
{
    file.seekg(location, std::ios::beg);
    read_value<IfdEntryPod>(file, m_pod);
    if (config && config->nested_tables){
        auto i = config->nested_tables->find(m_pod.tag_id);
        if (i != config->nested_tables->end())
        {
            m_table.reset(new IfdTable(file, m_pod.value.i32, i->second));
        }
    }   
}

std::uint16_t IfdEntry::tag_id() const {
    return m_pod.tag_id;
}

std::shared_ptr<IfdTable> IfdEntry::table() const {
    return m_table;
}

std::string IfdEntry::tag_name(std::map<int, const char*>* tags) const {
    if (tags){
        auto i = tags->find(m_pod.tag_id);
        if (i != tags->end()) {
            return i->second;
        }
    }
    std::ostringstream ss;
    ss << "0x" << std::hex << m_pod.tag_id;
    return ss.str();
}

std::string IfdEntry::value() const {
    std::ostringstream ss;
    if (m_pod.tag_type == 1){
        m_file.seekg(m_pod.value.i32, std::ios::beg);
        char* bytes = read_bytes(m_file, m_pod.size + 1);
        bytes[m_pod.size] = 0;
        ss << bytes;
        delete[] bytes;
    }
    else if (m_pod.tag_type == 2){
        m_file.seekg(m_pod.value.i32, std::ios::beg);
        char* bytes = read_bytes(m_file, m_pod.size);
        ss << bytes;
        delete[] bytes;
    }
    else if (m_pod.tag_type == 3){
        ss << m_pod.value.ui32;
    }
    else if (m_pod.tag_type == 4){
        ss << m_pod.value.ui32;
    }
    else  if (m_pod.tag_type == 5){
        m_file.seekg(m_pod.value.i32, std::ios::beg);
        std::uint32_t n, d;
        read_value<std::uint32_t>(m_file, n);
        read_value<std::uint32_t>(m_file, d);
        ss << n << "/" << d;
    }
    else if (m_pod.tag_type == 7){
        ss << m_pod.value.i32;
    }
    else if (m_pod.tag_type == 8){
        ss << m_pod.value.i32;
    }
    else if (m_pod.tag_type == 9){
        ss << m_pod.value.i32;
    }
    else  if (m_pod.tag_type == 10){
        m_file.seekg(m_pod.value.i32, std::ios::beg);
        std::int32_t n, d;
        read_value<std::int32_t>(m_file, n);
        read_value<std::int32_t>(m_file, d);
        ss << n << "/" << d;
    }
    else{
        ss << m_pod.tag_type << " todo";
    }
    return ss.str();
}


IfdTable::IfdTable(std::ifstream& file, int location, TableConfig* config)
: m_config(config)
{
    file.seekg(location, std::ios::beg);
    int n = read_value<std::int16_t>(file);

    for (int i = 0; i < n; ++i)
    {
        int entry_location = location + 2 + (s_ifd_entry_size * i);
        m_entries.push_back(IfdEntry(file, entry_location, config));
    }

    int entry_location = location + 2 + (s_ifd_entry_size * n);
    file.seekg(entry_location, std::ios::beg);
    m_next_ifd = read_value<std::int32_t>(file);
}

int IfdTable::GetNextIfdLocation() const {
    return m_next_ifd;
}

void IfdTable::PrintTable() const {
    cout << "IFD Table" << endl;
    for (auto j = entries_begin(); j != entries_end(); ++j)
    {
        cout << j->tag_name(m_config->tag_translation) << ": " << j->value() << endl;
    }
    cout << endl;

    for (auto j = entries_begin(); j != entries_end(); ++j)
    {
        if (j->table()){
            cout << "Nested Table (" << j->tag_name(m_config->tag_translation) << ")" << endl;
            j->table()->PrintTable();
        }
    }
}

std::vector<IfdEntry>::const_iterator IfdTable::entries_begin() const {
    return m_entries.cbegin();
}

std::vector<IfdEntry>::const_iterator IfdTable::entries_end() const {
    return m_entries.cend();
}

Cr2::Cr2(const std::string& filename)
: m_file(filename, std::ios::binary),
m_tiff_header(m_file, s_tiff_header_offset)
{
    int location = m_tiff_header.GetIfdLocation();
    do {
        m_ifd_tables.push_back(IfdTable(m_file, location, &s_tiff));
        location = m_ifd_tables.back().GetNextIfdLocation();
    } while (location);
}

std::vector<IfdTable>::const_iterator Cr2::tables_begin() const {
    return m_ifd_tables.cbegin();
}

std::vector<IfdTable>::const_iterator Cr2::tables_end() const {
    return m_ifd_tables.cend();
}

int _tmain(int argc, _TCHAR* argv[])
{
    Cr2 photo("photo.cr2");
    for (auto i = photo.tables_begin(); i != photo.tables_end(); ++i)
    {
        i->PrintTable();
    }
    return 0;
}

