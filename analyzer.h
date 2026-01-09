#include <vector> // include for std::vector
#include <string> // include for std::string
#include <unordered_map> // include for std::unordered_map

struct ZoneCount {
    std::string zone;
    unsigned int count;
};
struct SlotCount {
    std::string zone;
    int hour;              // 0-23
    unsigned int count;
};
struct ZoneData {
    std::string name; // Store name only once per unique zone
    unsigned int tripCount;
    int tripsPerHour[24];
    ZoneData() : tripCount(0) {
        for (int i = 0; i < 24; i++) tripsPerHour[i] = 0;
    }
};
class TripAnalyzer {
public:
    void ingestFile(const std::string& filename);
    std::vector<ZoneCount> topZones(int k = 10) const;
    std::vector<SlotCount> topBusySlots(int k = 10) const;
private:
    std::unordered_map<unsigned long long, ZoneData> zoneMap; // store zone data using hashed zone name as key (use number instead of string)
};

