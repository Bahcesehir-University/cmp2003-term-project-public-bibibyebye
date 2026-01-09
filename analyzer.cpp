#include "analyzer.h" // include for definitions and declarations in analyzer.h
#include <iostream> // include for std::cout and std::cerr
#include <fstream> // include for file input operations
#include <vector>
#include <string>
#include <algorithm> // include for std::reverse
#include <unordered_map>
#include <queue> // include for std::priority_queue
#include <cstring> // include for memmove

void TripAnalyzer::ingestFile(const std::string& fileName) { // function for reading and processing CSV file
    std::ifstream file(fileName, std::ios::binary); // open file in binary mode
    if (!file) { // check if file opened
        std::cerr << "Error opening file: " << fileName << std::endl; // print error message
        return; // exit if file cannot opened
    }

    file.unsetf(std::ios::skipws); // not skip whitespace characters and read every character

    size_t bufferSize = 128 * 1024; // unsigned integer type variable for buffer size initialized as 128 KB
    std::vector<char> buffer(bufferSize); // vector of chars as buffer to store file data

    size_t offset = 0; // unsigned integer type variable to read into buffer initialized as 0, meaning buffer empty (shows already used part)

    zoneMap.clear(); // clear previous data
    zoneMap.reserve(10000); // reserve space
    bool eofReached = false; // variable to show if end of file reached, set to false meaning not
    bool headerSkipped = false; // variable to skip header

    while (true) { // loop until break reached, to read into buffer at current offset
        if (offset >= bufferSize) { // check if buffer full
            bufferSize *= 2; // double buffer size
            buffer.resize(bufferSize); // resize buffer
        }
        file.read(buffer.data() + offset, bufferSize - offset); // read file into buffer after existing data, starting at offset
        size_t bytesRead = file.gcount(); // get count of bytes read

        if (bytesRead == 0) { // check if nothing read
            eofReached = true; // indicate end of file
        }
        size_t validDataLen = offset + bytesRead; // variable to see how many bytes in buffer valid

        if (eofReached && validDataLen > 0) { // check if end of file reached but data exist
            if (buffer[validDataLen - 1] != '\n') { // check if last character not '\n'
                if (validDataLen == bufferSize) { // check if buffer full
                    bufferSize++; // increase buffer size by 1
                    buffer.resize(bufferSize); // resize buffer
                }
                buffer[validDataLen] = '\n'; // add '\n' to buffer
                validDataLen++; // increase valid data length by 1
            }
        }
        else if (validDataLen == 0 && eofReached) { // run if no data left and end of file reached
            break; // exit loop
        } 
        char* start = buffer.data(); // char pointer variable pointing to beginning of buffer
        char* end = start + validDataLen; // char pointer variable pointing to end of valid data

        char* lastNewline = end - 1; // char pointer variable to point to last valid character in buffer
        while (lastNewline >= start && *lastNewline != '\n') { // loop moving backwards until newline
            lastNewline--;
        } 
        
        char* processEnd; // char pointer variable marking end of processing
        if (lastNewline >= start) { // check if one newline found (meaning complete line)
            processEnd = lastNewline + 1; // process data including '\n'
        } 
        else { // run when if not run
            if (eofReached) { // check if end of file reached
                processEnd = end; // process everything
            } else { // no complete line, do not process
                processEnd = start;
            }
        }

        const char* current = start; // char pointer variable (cannot modify) pointing to beginning
        if (!headerSkipped) { // check if header skipped
            while (current < processEnd && *current != '\n') current++; // while until end of header line
            if (current < processEnd) current++; // if not end of processing, move forward and skip '\n'
            headerSkipped = true; // header marked as skipped
        }
        while (current < processEnd) { // while until end of processing
            const char* rowStart = current; // char pointer variable (cannot modify) pointing to beginning of row
            const char* rowEnd = rowStart; // char pointer variable (cannot modify) pointing to same as startOfRow
            while (rowEnd < processEnd && *rowEnd != '\n') rowEnd++; // move by every character until end of row

            int commaCount = 0; // variable for comma counts
            for (const char* p = rowStart; p < rowEnd; ++p) { // loop every character
                if (*p == ',') commaCount++; // incerement comma counts by one for each comma
            }
            if (commaCount < 5) { // check if malformed row
                current = rowEnd; // skip row
                if (current < processEnd) current++; // if can move, move current forward
                continue; // skip rest and go next iteration
            }

            while (current < processEnd && *current != ',') current++; // loop forward until comma
            current++; // move past comma, skip TripID

            const char* zoneStart = current; // char pointer variable (cannot modify) pointing to zone name
            while (current < processEnd && *current != ',') current++; // lopp forward until comma
            long long zoneLength = current - zoneStart; // variable to store length of zone

            while (zoneLength > 0 && (zoneStart[0] == ' ' || zoneStart[0] == '\t')) { zoneStart++; zoneLength--; }// loop when string not empty, and first character ' ' or '\t': one character forward and decrease length by one 

            while (zoneLength > 0 && (zoneStart[zoneLength - 1] == ' ' || zoneStart[zoneLength - 1] == '\t')) { zoneLength--; } // loop when string not empty, and last character ' ' or '\t'
            current++; // move past comma
            
            while (current < processEnd && *current != ',') current++; // loop forward until comma
            current++; // move past comma, skip Dropoff
            
            int hour = -1; // variable hour initialized as -1, meaning invalid
            const char* lineSearch = current; // char pointer variable to search line starting from current
            while (lineSearch < processEnd && *lineSearch != '\r' && *lineSearch != '\n') { // loop until end of row not reached
                const char* fieldStart = lineSearch; // pointer to point to beginning
                while (lineSearch < processEnd && *lineSearch != ',' && *lineSearch != '\r' && *lineSearch != '\n') {
                    lineSearch++; // forward until end, stopping when ',' '\r' '\n' encountered
                }
                long long fieldLen = lineSearch - fieldStart; // variable for length of field
                if (fieldLen >= 13 && fieldStart[4] == '-' && fieldStart[7] == '-' && fieldStart[10] == ' ') { // check if date format correct YYYY-MM-DD
                    char h1 = fieldStart[11]; // variable for first digit of hour
                    if (h1 >= '0' && h1 <= '9') { // check if first digit valid number
                        hour = h1 - '0'; // to get numeric value of first digit, converting to int
                        char h2 = fieldStart[12]; // variable for second digit of hour
                        if (h2 >= '0' && h2 <= '9') { // check if second digit valid number
                            hour = hour * 10 + (h2 - '0'); // acquire hour in desired format
                        }
                        break; // exit loop
                    }
                }
                if (lineSearch < processEnd && *lineSearch == ',') lineSearch++; // if current character comma, skip it
            }
            
            while (current < processEnd && *current != '\n') current++; // loop until end of row
            if (current < processEnd) current++; // skip '\n'

            if (zoneLength > 0 && hour >= 0 && hour <= 23) { // check if zone not empty and hour valid
                unsigned long long h = 5381; // create hash value, initialized as 5381 (fixed number) (DJB2)
                for (long long k = 0; k < zoneLength; ++k) { // loop for every character of zone name
                    h = ((h << 5) + h) + (unsigned char)zoneStart[k]; // h*33 + character value
                }
                auto it = zoneMap.find(h); // search for h (compiler will figure variable type, as RHS) in it
                if (it != zoneMap.end()) { // check if found, run if yes
                    it->second.tripCount++; // access to its second element and increase trip counts by one
                    it->second.tripsPerHour[hour]++; // increase trips for this hour
                } else { // run if zone does not already exist 
                    ZoneData& data = zoneMap[h]; // create ZoneData object called data, using default constructor
                    data.name.assign(zoneStart, zoneLength); // create zone name string 
                    data.tripCount = 1; // initialize total trip count for data as 1
                    data.tripsPerHour[hour] = 1; // initialize trip for this hour as 1
                }
            }
        }

        size_t processed = processEnd - start; // unsigned integer type to show how many bytes processed safely
        size_t remaining = validDataLen - processed; // unsigned integer type to show how many bytes not processed

        if (remaining > 0) { // check if number of left bytes more than 0
            std::memmove(buffer.data(), processEnd, remaining); // copying remanining bytes to beginning of buffer using memmove
        }
        offset = remaining; // start writing next time after leftover bytes

        if (eofReached) break; // stop if end of file reached
    }
}
std::vector<ZoneCount> TripAnalyzer::topZones(int k) const { // function for top k zones with highest trip counts
    struct InternalZone { // to help for comparison, storing minimum information needed
        const std::string* zonePtr; // string pointer variable to zone name
        unsigned int count; // variable for storing how many trips belong to zone
    };

    auto comp = [](const InternalZone& a, const InternalZone& b) { // lambda function for comparison stored in variable
        if (a.count != b.count) return a.count > b.count; // if trip counts not equal, one with more is higher priority
        return *a.zonePtr < *b.zonePtr; // if equal, names compared alphabetically
        };

    std::priority_queue<InternalZone, std::vector<InternalZone>, decltype(comp)> pq(comp); // create priority queue storing object, using comparison
    for (const auto& pair : zoneMap) { // loop for every element in zoneMap and call it as pair (const meaning read only)
        InternalZone item = { &pair.second.name, pair.second.tripCount }; // create object for zone, storing pointer to name and total trip count (pair.second is zoneData)
        if ((int)pq.size() < k) { // check if priority queue has fewer than k elements
            pq.push(item); // insert zone into priority queue
        } else { // run if priority queue already has k elements
            if (comp(item, pq.top())) { // check comparison of new zone with weakest in queue and if new better:
                pq.pop(); // remove lowest from queue
                pq.push(item); // add new zone to queue
            }
        }
    }

    std::vector<ZoneCount> results; // vector to store what function returns
    results.reserve(pq.size()); // reserve enough space
    while (!pq.empty()) { // loop while priority queue not empty
        results.push_back({ *pq.top().zonePtr, pq.top().count }); // add top zone in priority queue to end of vector
        pq.pop(); // remove element that just processed
    }
    std::reverse(results.begin(), results.end()); // reverse results order since priority queue gives from lowest to highest
    return results; // return sorted list of top k zones
}
std::vector<SlotCount> TripAnalyzer::topBusySlots(int k) const { // function for top k zone and hour combinations
    struct InternalSlot { // helper, representing zone hour combination
        const std::string* zonePtr; // pointer to zone name
        int hour; // variable for hours of day (0-23)
        unsigned int count; // number of trips of zone hour combination
    };

    auto comp = [](const InternalSlot& a, const InternalSlot& b) { // lambda function for comparison stored in variable
        if (a.count != b.count) return a.count > b.count; // if trip counts not equal, one with more is higher priority
        int cmp = a.zonePtr->compare(*b.zonePtr); // if equal, names compared alphabetically
        if (cmp != 0) return cmp < 0; // if names different, return true if A should be before B (<0 meaning left smaller)
        return a.hour < b.hour; // if names equal, earlier hour first
        };

    std::priority_queue<InternalSlot, std::vector<InternalSlot>, decltype(comp)> pq(comp); // create priority queue storing object, using comparison

    for (const auto& pair : zoneMap) { // loop for every element in zoneMap and call it as pair (const meaning read only)
        const std::string& zoneName = pair.second.name; // to acquire zone name
        const ZoneData& data = pair.second; // to acquire zone data
        for (int hour = 0; hour < 24; ++hour) { // loop for hours (0 to 23) of day
            if (data.tripsPerHour[hour] > 0) { // check if any trip exist
                InternalSlot item = { &zoneName, hour, (unsigned int)data.tripsPerHour[hour] }; // create object storing pointer to zone name, hour, trip count

                if ((int)pq.size() < k) { // // check if priority queue has fewer than k elements
                    pq.push(item); // insert item into priority queue
                }
                else { // run if priority queue already has k elements
                    if (comp(item, pq.top())) { // check comparison of new zone with weakest in queue and if new better:
                        pq.pop(); // remove lowest from queue
                        pq.push(item); // add new zone to queue
                    }
                }
            }
        }
    }

    std::vector<SlotCount> results; // vector to store what function returns
    results.reserve(pq.size()); // reserve enough space
    while (!pq.empty()) { // loop while priority queue not empty
        results.push_back({ *pq.top().zonePtr, pq.top().hour, pq.top().count }); // add top combination in priority queue to end of vector
        pq.pop(); // remove element that just processed
    }

    std::reverse(results.begin(), results.end()); // reverse results order since priority queue gives from lowest to highest

    return results; // return sorted list of top k combinations
}
