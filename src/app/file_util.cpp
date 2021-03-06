/*
 *    Copyright (c) 2020, The OpenThread Commissioner Authors.
 *    All rights reserved.
 *
 *    Redistribution and use in source and binary forms, with or without
 *    modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *    3. Neither the name of the copyright holder nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *    POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 *  This file implements file utilities.
 *
 */

#include "app/file_util.hpp"

#include <algorithm>

#include <string.h>

#include "common/error_macros.hpp"
#include "common/utils.hpp"

namespace ot {

namespace commissioner {

Error WriteFile(const std::string &aData, const std::string &aFilename)
{
    Error error;
    FILE *f = fopen(aFilename.c_str(), "w");

    if (f == nullptr)
    {
        if (errno == ENOENT)
        {
            ExitNow(error = ERROR_NOT_FOUND("cannot open file '{}', {}", aFilename, strerror(errno)));
        }
        else
        {
            ExitNow(error = ERROR_IO_ERROR("cannot open file '{}', {}", aFilename, strerror(errno)));
        }
    }

    fputs(aData.c_str(), f);

exit:
    if (f != NULL)
    {
        fclose(f);
    }

    return error;
}

Error ReadFile(std::string &aData, const std::string &aFilename)
{
    Error error;

    FILE *f = fopen(aFilename.c_str(), "r");

    if (f == nullptr)
    {
        if (errno == ENOENT)
        {
            ExitNow(error = ERROR_NOT_FOUND("cannot open file '{}', {}", aFilename, strerror(errno)));
        }
        else
        {
            ExitNow(error = ERROR_IO_ERROR("cannot open file '{}', {}", aFilename, strerror(errno)));
        }
    }

    while (true)
    {
        int c = fgetc(f);
        VerifyOrExit(c != EOF);
        aData.push_back(c);
    }

exit:
    if (f != NULL)
    {
        fclose(f);
    }

    return error;
}

Error ReadPemFile(ByteArray &aData, const std::string &aFilename)
{
    Error error;

    std::string str;

    SuccessOrExit(error = ReadFile(str, aFilename));

    aData = {str.begin(), str.end()};
    aData.push_back('\0');

exit:
    return error;
}

Error ReadHexStringFile(ByteArray &aData, const std::string &aFilename)
{
    Error       error;
    std::string hexString;
    ByteArray   data;

    SuccessOrExit(error = ReadFile(hexString, aFilename));

    hexString.erase(std::remove_if(hexString.begin(), hexString.end(), [](int c) { return isspace(c); }),
                    hexString.end());
    SuccessOrExit(error = utils::Hex(data, hexString));

    aData = data;

exit:
    return error;
}

} // namespace commissioner

} // namespace ot
