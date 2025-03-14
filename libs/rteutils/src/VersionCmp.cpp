/******************************************************************************/
/* RTE - CMSIS Run-Time Environment */
/******************************************************************************/
/** @file VersionCmp.cpp
* @brief Semantic version comparison
*/
/******************************************************************************/
/*
 * Copyright (c) 2020-2021 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/******************************************************************************/

#include "VersionCmp.h"

#include "AlnumCmp.h"
#include "RteUtils.h"

#include <cstring>

using namespace std;

/**
* Internal helper class ported from CMSIS Eclipse plug-in and optimized for C
*/
static const char* ZERO_STRING = "0";
#define MAX_BUF 256

class Version {
private:
  char buf[MAX_BUF];
  char* m_ptr;
  const char* segments[3]; // first three version segments : MAJOR.MINOR.PATCH
  char* release; // remainder (after '-');

public:
  Version(const string& ver) : m_ptr(0), release(0) {
    init(ver);
  }

private:

  void init(const string& version) {
    // leave one char room for extra 0
    size_t len = version.length();
    if (len >= MAX_BUF - 1) {
      len = MAX_BUF - 2;
    }
    memcpy(buf, version.c_str(), len);
    buf[len] = '\0';
    m_ptr = buf;
    // 1. drop build metadata
    char* p = strchr(m_ptr, '+');
    if (p)
      *p = '\0';
    parse(m_ptr);
  }

  void parse(char* ptr) {
    // 2. extract release
    char* p = strchr(ptr, '-');
    if (p != 0) {
      release = p + 1;
      *p = 0;
    } else if (ptr == buf && *ptr) { // initial parse
    // check for special ST case without dash like 1.2.3b < 1.2.3
      int lastIndex = (int)strlen(buf) - 1;
      for (int pos = lastIndex; pos >= 0; pos--) {
        char ch = buf[pos];
        if (ch == '.')
          break;
        if (!isdigit(ch))
          continue;
        if (pos < lastIndex) {
          for (int i = lastIndex; i > pos; i--)
            ptr[i + 1] = ptr[i];
          ptr[lastIndex + 2] = '\0';
          pos++;
          ptr[pos] = 0;
          release = &(ptr[pos + 1]);
        }
        break;
      }
    }
    // 3. split segments
    p = ptr;
    for (int i = 0; i < 3; i++) {
      if (p && *p) {
        segments[i] = p;
        p = strchr(p, '.');
        if (p) {
          *p = '\0';
          p++;
        }
      } else {
        segments[i] = ZERO_STRING;
      }
    }
  }

public:
  char* getRelease() const {
    return release;
  }

  const char* getSegment(int index) const {
    return segments[index];
  }

  int compareTo(const Version& that) const {
    return compareTo(that, true);
  }

  int compareTo(const Version& that, bool cs) const {
    int result = 3;

    for (int i = 0; i < 3; i++) {
      const char* thisSegment = getSegment(i);
      const char* thatSegment = that.getSegment(i);
      int res = AlnumCmp::Compare(thisSegment, thatSegment, cs);
      if (res != 0)
        return res > 0 ? result : -result;
      if (result > 1)
        result--;
    }

    char* thisRelease = getRelease();
    char* thatRelease = that.getRelease();

    if (!thisRelease && !thatRelease)
      return 0;
    else if (!thisRelease)
      return 1;
    else if (!thatRelease)
      return -1;

    // compare releases
    Version v1(thisRelease);
    Version v2(thatRelease);
    result = v1.compareTo(v2, false); // the release is case - insensitive

    if (result < 0) {
      return -1;
    } else if (result > 0) {
      return 1;
    }
    return 0;
  }
};


int VersionCmp::Compare(const string& v1, const string& v2, bool cs) {
  // Split v1 and v2 according to http://semver.org/ and compare individually
  Version ver1(v1);
  Version ver2(v2);
  int res = ver1.compareTo(ver2, cs);
  return res;
}

int VersionCmp::RangeCompare(const string& version, const string& versionRange)
{
  string verMin = RteUtils::GetPrefix(versionRange);
  string verMax = RteUtils::GetSuffix(versionRange);
  if (!verMin.empty()) {
    int resMin = Compare(version, verMin);
    if (resMin < 0 || verMin == verMax) // lower than min or exact match is required?
      return resMin;
  }
  if (!verMax.empty()) {
    int resMax = Compare(version, verMax);
    if (resMax > 0)
      return resMax;
  }
  return 0;
}

string VersionCmp::RemoveVersionMeta(const string& v) {
  string::size_type pos = v.find('+');
  if (pos != string::npos)
    return v.substr(0, pos);
  return v;
}


VersionCmp::MatchMode VersionCmp::MatchModeFromString(const std::string& mode)
{
  if (mode == "fixed")
    return FIXED_VERSION;
  else if (mode == "latest")
    return LATEST_VERSION;
  else if (mode == "excluded")
    return EXCLUDED_VERSION;
  // all other cases
  return ANY_VERSION;
}

std::string VersionCmp::MatchModeToString(VersionCmp::MatchMode mode)
{
  string s;
  switch (mode) {
  case FIXED_VERSION:
    s = "fixed";
    break;
  case LATEST_VERSION:
    s = "latest";
    break;
  case ANY_VERSION:
    break; // no string
  case EXCLUDED_VERSION:
    s = "excluded";
    break;
  }
  return s;
}


// End of VersionCmp.cpp
