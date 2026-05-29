/*
* Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "archive_macros.h"
#include "zip_handler_common.h"

#include <ctype.h>
#include "securec.h"
#include "errorcode.h"

#define YEAR_1980 1980
#define YEAR_1900 1900
#define YEAR_80 80
#define YEAR_20 20

#define YEAR_SHIFT 512
#define MONTH_SHIFT 32
#define DAY_SHIFT 16
#define HOUR_SHIFT 2048
#define MIN_SHIFT 32
#define SEC_SHIFT 2

LOCAL int32_t ZipInvalidDate(const struct tm *ptm)
{
#define DATE_VALUE_IN_RANGE(min, max, value) ((min) <= (value) && (value) <= (max))
    return (!DATE_VALUE_IN_RANGE(0, 127 + 80, ptm->tm_year) || /* 1980-based year, allow 80 extra */
            !DATE_VALUE_IN_RANGE(0, 11, ptm->tm_mon) || !DATE_VALUE_IN_RANGE(1, 31, ptm->tm_mday) ||
            !DATE_VALUE_IN_RANGE(0, 23, ptm->tm_hour) || !DATE_VALUE_IN_RANGE(0, 59, ptm->tm_min) ||
            !DATE_VALUE_IN_RANGE(0, 59, ptm->tm_sec));
#undef DATE_VALUE_IN_RANGE
}

ARCHIVE_IMPL int ZipTimeToTm(time_t time, struct tm *ptm)
{
    struct tm ltm;
    if (ptm == NULL)
        return ARCHIVE_PARAM_ERROR;
    if (!localtime_r(&time, &ltm)) {
        memset_s(ptm, sizeof(struct tm), 0, sizeof(struct tm));
        return ARCHIVE_INTERNAL_ERROR;
    }
    (void)memcpy_s(ptm, sizeof(struct tm), &ltm, sizeof(struct tm));
    return ARCHIVE_OK;
}

LOCAL uint32_t ZipTimeToDosModDateTime(const struct tm *ptm)
{
    if (ptm == NULL) {
        return 0;
    }
    struct tm fixedTime;

    /* Years supported: */

    /* [00, 79]      (assumed to be between 2000 and 2079) */
    /* [80, 207]     (assumed to be between 1980 and 2107, typical output of old */
    /*                software that dose 'year-1900' to get a double digit year) */
    /* [1980, 2107]  (due to format limitations, only years 1980-2107 can be stored) */

    (void)memcpy_s(&fixedTime, sizeof(struct tm), ptm, sizeof(struct tm));
    if (fixedTime.tm_year >= YEAR_1980)
        fixedTime.tm_year -= YEAR_1980;
    else if (fixedTime.tm_year >= YEAR_80)
        fixedTime.tm_year -= YEAR_80;
    else
        fixedTime.tm_year += YEAR_20;

    if (ZipInvalidDate(&fixedTime))
        return 0;

    return (((uint32_t)fixedTime.tm_mday + MONTH_SHIFT * ((uint32_t)fixedTime.tm_mon + 1) +
             YEAR_SHIFT * (uint32_t)fixedTime.tm_year)
            << DAY_SHIFT) |
           (((uint32_t)fixedTime.tm_sec / SEC_SHIFT) + MIN_SHIFT * (uint32_t)fixedTime.tm_min +
            HOUR_SHIFT * (uint32_t)fixedTime.tm_hour);
}

ARCHIVE_IMPL uint32_t ZipGetCurrentDosModeDateTime()
{
    time_t timestamp = time(NULL);
    struct tm local_time;
    struct tm *plocal_time = localtime_r(&timestamp, &local_time);
    return ZipTimeToDosModDateTime(plocal_time);
}


ARCHIVE_IMPL uint32_t ZipConvertTimeToDosDate(time_t time)
{
    struct tm ptm;
    int ret = ZipTimeToTm(time, &ptm);
    if (ret != ARCHIVE_OK) {
        return 0;
    }
    return ZipTimeToDosModDateTime(&ptm);
}

ARCHIVE_IMPL time_t ZipConvertDosDateToTime(uint32_t dosDateTime)
{
    unsigned int msTime;
    unsigned int msDate;
    struct tm ts;

    msTime = dosDateTime & 0xffff;
    msDate = dosDateTime >> DAY_SHIFT;

    memset_s(&ts, sizeof(ts), 0, sizeof(ts));
    ts.tm_mday = msDate & 0x1f;
    ts.tm_mon = ((msDate >> 5) & 0x0f) - 1; // msDate right shift 5 bits
    ts.tm_year = ((msDate >> 9) & 0x7f) + YEAR_80;  // msDate right shift 9 bits
    ts.tm_hour = (msTime >> 11) & 0x1f; // msTime right shift 11 bits
    ts.tm_min = (msTime >> 5) & 0x3f;   // msTime right shift 5 bits
    ts.tm_sec = (msTime << 1) & 0x3e;   // msTime left shift 1 bit
    ts.tm_isdst = -1;
    return mktime(&ts);
}