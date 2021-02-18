/* Copyright (c) 2020 Jeffrey Griffin

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

#ifndef ChunkedTransferDecoder_h
#define ChunkedTransferDecoder_h

#include <stdlib.h>
#include <stdint.h>

class ChunkedTransferDecoder {
public:
  struct ParseResult {
    size_t len;
    bool data;
  };

  ChunkedTransferDecoder();
  ParseResult parse(const unsigned char *data, size_t data_len);

private:
  enum State { PARSING_LENGTH, PARSING_EXTENSION, PARSING_HEADER_CRLF, PARSING_DATA, PARSING_DATA_CRLF,
               PARSING_TRAILER_START, PARSING_TRAILER, PARSING_TRAILER_CRLF, PARSING_FINAL_CRLF, PARSING_DONE,
               PARSING_ERROR };

  State state;
  size_t chunkLen;

  ParseResult parseDataBuf(const unsigned char *data, size_t dataLen);
  ParseResult parseMetadataBuf(const unsigned char *data, size_t dataLen);
  State parseMetadata(unsigned char data);
  State parseLength(unsigned char data);
  State parseLengthDigit(unsigned char data);
  State parseExtension(unsigned char data);
  State parseHeaderCRLF(unsigned char data);
  State parseData(unsigned char data);
  State parseDataCRLF(unsigned char data);
  State parseTrailerStart(unsigned char data);
  State parseTrailer(unsigned char data);
  State parseTrailerCRLF(unsigned char data);
  State parseFinalCRLF(unsigned char data);
};

#endif
