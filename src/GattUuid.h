// Copyright 2017 Paul Nettle.
//
// This file is part of Gobbledegook.
//
// Gobbledegook is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Gobbledegook is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Gobbledegook.  If not, see <http://www.gnu.org/licenses/>.

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// >>
// >>>  INSIDE THIS FILE
// >>
//
// This class manages the creation of properly formatted GATT UUIDs
//
// >>
// >>>  DISCUSSION
// >>
//
// A GATT UUID represents a unique identifier for a GATT service, characteristic or descriptor. UUIDs allow clients to recognize
// the purpose of a service (battery level, temperature data, etc.) Similarly, they are used to identify characteristics (the
// battery percentage or actual temperature value) and descriptors (for example, the format of a characteristic data item.)
//
// GATT UUIDs come in three flavors: 16-bit, 32-bit and 128-bit. In practice, you will see two different kinds of UUIDs: a short
// UUID (ex: "2901") and a long UUID (ex: "00000002-1E3C-FAD4-74E2-97A033F1BFAA").
//
// The short form refer to 16-bit UUID that is assigned to a standard interface defined as part of the Bluetooth Wireless Standard.
// The short UUID "2901" refers to a standard (assigned by the Bluetooth Wireless Standard) descriptor named "Characteristic User
// Description". These are commonly called "Assigned Numbers".
//
// In fact, all UUIDs are 128-bit values. The 16-bit form is just an abbreviated way of specifying a standard interface that uses
// what Bluetooth calls the "Base Universally Unique Identifier (UUID)". This UUID is defined by the standard as
// "00000000-0000-1000-8000-00805f9b34fb". The 16-bit value "2901" would be inserted into the Base UUID like so:
// "00002901-0000-1000-8000-00805f9b34fb".
//
// When defining a custom service (or characteristic or descriptor) UUIDs are typically generated by the developer. These UUIDs are
// then used by the client software to recognize the custom service (or characteristic or descriptor.)
//
// The `GattUuid` class is a helper class for creating instances of UUIDs. By instantiating a GattUuid with a 4-character string,
// the GattUuid will treat the string as a 16-bit UUID and wrap it with the Base Universally Unique Identifier. Similarly, an 8-
// character string will be treated as a standard 32-bit UUID, also wrapping it with the Base Universally Unique Identifier. A 32-
// character string will be treated as a 128-bit GATT UUID.
//
// When specifying your UUIDs, feel free to use the format that suits you best (with or without dashes, dots in place of dashes,
// etc.) The string will first be sanitized, removing all non-hex characters, and then re-formatted appropriately with the dashes
// in their standard places: "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx".
//
// By represetng a UUID in a custom class like this, we are able to give a UUID its own type, and use type safety to ensure that we
// don't confuse regular strings with GATT UUIDs throughout the codebase.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#pragma once

#include <string>
#include <algorithm>
#include <stdint.h>
#include <ctype.h>

#include <iostream>
#include "Logger.h"

// "0000180A-0000-1000-8000-00805f9b34fb"
struct GattUuid
{
	static constexpr const char *kGattStandardUuidPart1Prefix = "0000";
	static constexpr const char *kGattStandardUuidSuffix = "-0000-1000-8000-00805f9b34fb";

	// Construct a GattUuid from a partial or complete string UUID
	//
	// This constructor will do the best it can with the data it is given. It will first clean the input by removing all non-hex
	// characters (see `clean`) and the remaining characters are processed in the following way:
	//
	//     4-character string is treated as a 16-bit UUID
	//     8-character string is treated as a 32-bit UUID
	//     32-character string is treated as a 128-bit UUID
	//
	// If the input string is not one of the above lengths, the  UUID will be left uninitialized as an empty string with a bit
	// count of 0.
	//
	// Finally, dashes are inserted into the string at the appropriate locations (see `dashify`).
	GattUuid(const char *strUuid)
	{
		*this = GattUuid(std::string(strUuid));
	}

	// Construct a GattUuid from a partial or complete string UUID
	//
	// This constructor will do the best it can with the data it is given. It will first clean the input by removing all non-hex
	// characters (see `clean`) and the remaining characters are processed in the following way:
	//
	//     4-character string is treated as a 16-bit UUID
	//     8-character string is treated as a 32-bit UUID
	//     32-character string is treated as a 128-bit UUID
	//
	// If the input string is not one of the above lengths, the  UUID will be left uninitialized as an empty string with a bit
	// count of 0.
	//
	// Finally, dashes are inserted into the string at the appropriate locations (see `dashify`).
	GattUuid(std::string strUuid)
	{
		// Clean the string
		strUuid = clean(strUuid);

		// It's hex, so each character represents 4 bits
		bitCount = strUuid.length() * 4;

		if (bitCount == 16)
		{
			strUuid = kGattStandardUuidPart1Prefix + strUuid + kGattStandardUuidSuffix;
		}
		else if (bitCount == 32)
		{
			strUuid += kGattStandardUuidSuffix;
		}
		else if (bitCount != 128)
		{
			bitCount = 0;
			strUuid = "";
		}

		uuid = dashify(strUuid);
	}

	// Constructs a GattUuid from a 16-bit Uuid value
	//
	// The result will take the form:
	//
	//     0000????-0000-1000-8000-00805f9b34fb
	//
	// ...where "????" is replaced by the 4-digit hex value of `part`
	GattUuid(const uint16_t part)
	{
		bitCount = 16;
		char partStr[5];
		snprintf(partStr, sizeof(partStr), "%04x", part);
		uuid = std::string(kGattStandardUuidPart1Prefix) + partStr + kGattStandardUuidSuffix;
	}

	// Constructs a GattUuid from a 32-bit Uuid value
	//
	// The result will take the form:
	//
	//     ????????-0000-1000-8000-00805f9b34fb
	//
	// ...where "????????" is replaced by the 8-digit hex value of `part`
	GattUuid(const uint32_t part)
	{
		bitCount = 32;
		char partStr[9];
		snprintf(partStr, sizeof(partStr), "%04x", part);
		uuid = std::string(partStr) + kGattStandardUuidSuffix;
	}

	// Constructs a GattUuid from a 5-part set of input values
	//
	// The result will take the form:
	//
	//     11111111-2222-3333-4444-555555555555
	//
	// ...where each digit represents the part from which its hex digits will be pulled from.
	//
	// Note that `part5` is a 48-bit value and will be masked such that only the lower 48-bits of `part5` are used with all other
	// bits ignored.
	GattUuid(const uint32_t part1, const uint16_t part2, const uint16_t part3, const uint16_t part4, const uint64_t part5)
	{
		bitCount = 128;
		char partsStr[37];
		uint32_t part5a = (part5 >> 4) & 0xffffffff;
		uint32_t part5b = part5 & 0xffff;
		snprintf(partsStr, sizeof(partsStr), "%08x-%04x-%04x-%04x-%08x%04x", part1, part2, part3, part4, part5a, part5b);
		uuid = std::string(partsStr);
	}

	// Returns the bit count of the input when the GattUuid was constructed. Valid values are 16, 32, 128.
	//
	// If the GattUuid was constructed imporperly, this method will return 0.
	int getBitCount() const
	{
		return bitCount;
	}

	// Returns the 16-bit portion of the GATT UUID or an empty string if the GattUuid was not created correctly
	//
	// Note that a 16-bit GATT UUID is only valid for standarg GATT UUIDs (prefixed with "0000" and ending with
	// "0000-1000-8000-00805f9b34fb").
	std::string toString16() const
	{
		if (uuid.empty()) { return uuid; }
		return uuid.substr(4, 4);
	}

	// Returns the 32-bit portion of the GATT UUID or an empty string if the GattUuid was not created correctly
	//
	// Note that a 32-bit GATT UUID is only valid for standarg GATT UUIDs (ending with "0000-1000-8000-00805f9b34fb").
	std::string toString32() const
	{
		if (uuid.empty()) { return uuid; }
		return uuid.substr(0, 8);
	}

	// Returns the full 128-bit GATT UUID or an empty string if the GattUuid was not created correctly
	std::string toString128() const
	{
		return uuid;
	}

	// Returns a string form of the UUID, based on the bit count used when the UUID was created. A 16-bit UUID will return a
	// 4-character hex string. A 32-bit UUID will return an 8-character hex string. Otherwise the UUID is assumed to be 128 bits
	// (which is only true if it was created correctly) and the full UUID is returned
	std::string toString() const
	{
		if (bitCount == 16) return toString16();
		if (bitCount == 32) return toString32();
		return toString128();
	}

	// Returns a new string containing the lower case contents of `strUuid` with all non-hex characters (0-9, A-F) removed
	static std::string clean(const std::string &strUuid)
	{
		if (strUuid.empty()) return strUuid;

		// Lower case
		std::string cleanStr = strUuid;
		std::transform(cleanStr.begin(), cleanStr.end(), cleanStr.begin(), ::tolower);

		// Remove all non-hex characters
		cleanStr.erase
		(
			std::remove_if
			(
				cleanStr.begin(), 
				cleanStr.end(),
				[](char c)
				{
					return !((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'));
				}
			), cleanStr.end()
		);

		return cleanStr;
	}

	// Returns a clean string (see `clean`) that has dashes ('-') inserted at the proper locations. If the string is not a full
	// UUID, this routine will cleanup and add as many dashes as possible until it runs out of characters.
	//
	// Example transforms:
	//
	//     "0000180A-0000-1000-8000-00805f9b34fb"        -> "0000180a-0000-1000-8000-00805f9b34fb"
	//     "0000180A00001000800000805f9b34fb"            -> "0000180a-0000-1000-8000-00805f9b34fb"
	//     "0000180A/0000.1000_zzzzzz_8000+00805f9b34fb" -> "0000180a-0000-1000-8000-00805f9b34fb"
	//     "0000180A"                                    -> "0000180a"
	//     "0000180A.0000.100"                           -> "0000180a-0000-100"
	//     "rqzp"                                        -> ""
	//
	static std::string dashify(const std::string &str)
	{
		// Ensure we have a clean string to start with
		std::string dashed = clean(str);

		// Add each dash, provided there are enough characters
		if (dashed.length() > 8) { dashed.insert(8, 1, '-'); }
		if (dashed.length() > 13) { dashed.insert(13, 1, '-'); }
		if (dashed.length() > 18) { dashed.insert(18, 1, '-'); }
		if (dashed.length() > 23) { dashed.insert(23, 1, '-'); }
		return dashed;
	}

private:

	std::string uuid;
	int bitCount;
};
