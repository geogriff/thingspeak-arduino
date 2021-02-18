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

#include <stdint.h>
#include <limits.h>
#include "ChunkedTransferDecoder.h"

ChunkedTransferDecoder::ChunkedTransferDecoder() {
  this->chunkLen = 0;
  this->state = PARSING_LENGTH;
}

ChunkedTransferDecoder::ParseResult ChunkedTransferDecoder::parse(const unsigned char *data, size_t dataLen) {
  if (this->state == PARSING_DATA) {
    return parseDataBuf(data, dataLen);
  } else {
    return parseMetadataBuf(data, dataLen);
  }
}

ChunkedTransferDecoder::ParseResult ChunkedTransferDecoder::parseDataBuf(const unsigned char *data, size_t dataLen) {
  if (this->chunkLen > dataLen) {
    this->chunkLen -= dataLen;
    return ChunkedTransferDecoder::ParseResult { .len = dataLen, .data = true };
  } else {
    size_t resDataLen = this->chunkLen;
    this->chunkLen = 0;
    this->state = PARSING_DATA_CRLF;
    return ChunkedTransferDecoder::ParseResult { .len = resDataLen, .data = true };
  }
}

ChunkedTransferDecoder::ParseResult ChunkedTransferDecoder::parseMetadataBuf(const unsigned char *data, size_t dataLen) {
  size_t dataPos = 0;
  while (dataPos < dataLen && this->state != PARSING_DATA) {
    this->state = parseMetadata(data[dataPos]);
    dataPos++;
  }
  return ChunkedTransferDecoder::ParseResult { .len = dataPos, .data = false };
}

ChunkedTransferDecoder::State ChunkedTransferDecoder::parseMetadata(unsigned char data) {
  switch (this->state) {
  case PARSING_LENGTH: {
    return parseLength(data);
  }
  case PARSING_EXTENSION: {
    return parseExtension(data);
  }
  case PARSING_HEADER_CRLF: {
    return parseHeaderCRLF(data);
  }
  case PARSING_DATA: {
    return PARSING_ERROR;
  }
  case PARSING_DATA_CRLF: {
    return parseDataCRLF(data);
  }
  case PARSING_TRAILER_START: {
    return parseTrailerStart(data);
  }
  case PARSING_TRAILER: {
    return parseTrailer(data);
  }
  case PARSING_TRAILER_CRLF: {
    return parseTrailerCRLF(data);
  }
  case PARSING_FINAL_CRLF: {
    return parseFinalCRLF(data);
  }
  case PARSING_DONE: {
    return PARSING_ERROR;
  }
  case PARSING_ERROR: {
    return PARSING_ERROR;
  }
  }
}

ChunkedTransferDecoder::State ChunkedTransferDecoder::parseLength(unsigned char data) {
  if (data >= '0' && data <= '9') {
    return parseLengthDigit(data - '0');
  } else if (data >= 'a' && data <= 'f') {
    return parseLengthDigit(data - 'a' + 0xA);
  } else if (data >= 'A' && data <= 'F') {
    return parseLengthDigit(data - 'A' + 0xA);
  } else if (data == ';') {
    return PARSING_EXTENSION;
  } else if (data == '\r') {
    return PARSING_HEADER_CRLF;
  }
  return PARSING_ERROR;
}

ChunkedTransferDecoder::State ChunkedTransferDecoder::parseLengthDigit(unsigned char digit) {
  if (this->chunkLen > SIZE_MAX / 0x10) {
    return PARSING_ERROR;
  }
  this->chunkLen = this->chunkLen * 0x10 + digit;
  return PARSING_LENGTH;
}

ChunkedTransferDecoder::State ChunkedTransferDecoder::parseExtension(unsigned char data) {
  if (data == '\r') {
    return PARSING_HEADER_CRLF;
  }
  return PARSING_EXTENSION;
}

 ChunkedTransferDecoder::State ChunkedTransferDecoder::parseHeaderCRLF(unsigned char data) {
  if (data == '\n') {
    if (this->chunkLen != 0) {
      return PARSING_DATA;
    } else {
      return PARSING_TRAILER_START;
    }
  }
  return PARSING_ERROR;
}

ChunkedTransferDecoder::State ChunkedTransferDecoder::parseDataCRLF(unsigned char data) {
  if (data == '\n') {
    return PARSING_LENGTH;
  }
  return PARSING_ERROR;
}

ChunkedTransferDecoder::State ChunkedTransferDecoder::parseTrailerStart(unsigned char data) {
  if (data == '\r') {
    return PARSING_FINAL_CRLF;
  }
  return PARSING_TRAILER;
}

ChunkedTransferDecoder::State ChunkedTransferDecoder::parseTrailer(unsigned char data) {
  if (data == '\r') {
    return PARSING_TRAILER_CRLF;
  }
  return PARSING_TRAILER;
}

ChunkedTransferDecoder::State ChunkedTransferDecoder::parseTrailerCRLF(unsigned char data) {
  if (data == '\n') {
    return PARSING_TRAILER_START;
  }
  return PARSING_ERROR;
}

ChunkedTransferDecoder::State ChunkedTransferDecoder::parseFinalCRLF(unsigned char data) {
  if (data == '\n') {
    return PARSING_DONE;
  }
  return PARSING_ERROR;
}
