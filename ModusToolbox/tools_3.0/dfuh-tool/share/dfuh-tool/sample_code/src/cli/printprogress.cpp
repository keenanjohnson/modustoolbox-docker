// Copyright 2018 DuckDB Contributors (see https://github.com/duckdb/duckdb/graphs/contributors)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH
// THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "printprogress.h"

#include <stdio.h>
#include <string.h>

namespace cydfuht {
namespace cli {

void printProgress(double percentage) {
    // Adapted from https://github.com/duckdb/duckdb/blob/master/src/common/printer.cpp and its PrintProgress function
    static const char *progressbar = "############################################################";
    int fullbarsize = (int)strlen(progressbar);
    int barsize = (int)(percentage / 100.0 * fullbarsize);
    int spacesize = fullbarsize - barsize;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    printf("\r%3d%% [%.*s%*s]", (int)percentage, barsize, progressbar, spacesize, "");
    fflush(stdout);
}

}  // namespace cli
}  // namespace cydfuht
