#include <fstream>
#include <memory>
#include <map>
#include <vector>
#include <cstdint>

struct TableConfig
{
    TableConfig(std::map<int, const char*>* tt, std::map<int, TableConfig*>* nt);
    std::map<int, TableConfig*>* nested_tables;
    std::map<int, const char*>* tag_translation;
};

struct TiffHeader
{
    struct TiffHeaderPod
    {
        char byte_order[2];
        std::int16_t tiff_magic_word;
        std::int32_t tiff_ifd_offset;
        std::int16_t cr_magic_word;
        std::int8_t cr2_major_version;
        std::int8_t cr2_minor_version;
        std::int32_t raw_ifd_offset;
    };

    TiffHeader(std::ifstream& file, int location);
    int GetIfdLocation() const;

private:
    TiffHeaderPod m_pod;
};

class IfdTable;

class IfdEntry
{
public:
    struct IfdEntryPod 
    {
        std::uint16_t tag_id;
        std::int16_t tag_type;
        std::int32_t size;
        union {
            std::int32_t i32;
            std::uint32_t ui32;
            char c;
            float f;
        } value;
    };

    IfdEntry(std::ifstream& file, int location, TableConfig* config);

    std::uint16_t tag_id() const;
    std::string tag_name(std::map<int, const char*>*) const;
    std::string value() const;
    std::shared_ptr<IfdTable> table() const;

private:
    IfdEntryPod m_pod;
    std::ifstream& m_file;
    std::shared_ptr<IfdTable> m_table;
};


class IfdTable
{
public:
    IfdTable(std::ifstream& file, int location, TableConfig* config);

    void PrintTable() const;
    int GetNextIfdLocation() const;
    std::vector<IfdEntry>::const_iterator entries_begin() const;
    std::vector<IfdEntry>::const_iterator entries_end() const;

private:
    std::vector<IfdEntry> m_entries;
    TableConfig* m_config;
    int m_next_ifd;
};

class Cr2
{
public:
    Cr2(const std::string& filename);

    std::vector<IfdTable>::const_iterator tables_begin() const;
    std::vector<IfdTable>::const_iterator tables_end() const;

private:
    std::ifstream m_file;
    TiffHeader m_tiff_header;
    std::vector<IfdTable> m_ifd_tables;
    std::shared_ptr<IfdTable> m_exif_table;
    std::shared_ptr<IfdTable> m_gps_table;
};
