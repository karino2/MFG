/*
  Copyright (c) karino2 2023
*/
#ifndef _CPP_UNZIP_
#define _CPP_UNZIP_

#include <string>
#include <vector>
#include <cstdint>
#include <istream>
#include <stdexcept>

// depend on zlib.
#include <zlib.h>

/*
  Only support compressio method 0 and 8 (no compress and deflate.)
  Only support non-encrypted.
*/
namespace cppunzip {

struct UnZipError : public std::runtime_error {
  UnZipError( const std::string& msg ) : std::runtime_error( msg ) {}
};


struct File {
  size_t _size;

  File(size_t size) : _size(size) {}
  virtual ~File(){}

  // utility method.
  int readAt(size_t pos, uint8_t *dst, size_t size) {
    if(pos > _size)
      throw UnZipError("Try to read outside of file end.");

    // size == 0 read is valid.
    // do nothing.
    if (size == 0)
      return 0;
    return readAtImpl(pos, dst, size);
  }

  void readSpecificSize(size_t offset, uint8_t* dst, size_t size, const std::string& errMsg) {
    int res = readAt(offset, dst, size);
    if(res != size)
      throw UnZipError(errMsg);
  }

protected:
  virtual int readAtImpl(size_t pos, uint8_t* dst, size_t size) = 0;
};

struct IStreamFile : public File {
  std::istream& _istream;

  IStreamFile(std::istream& istream) : File(0), _istream(istream) {
    istream.seekg(0, std::istream::end);
    _size = istream.tellg();
    istream.seekg(0);
  }
  virtual ~IStreamFile() {}

protected:
  int readAtImpl(size_t pos, uint8_t* dst, size_t size) {
    _istream.clear();
    _istream.seekg(pos);
    _istream.read((char*)dst, size);
    return (int)_istream.gcount();
  }
};


namespace impl {
// zip format
// https://docs.fileformat.com/compression/zip/
// https://pkware.cachefly.net/webdocs/casestudies/APPNOTE.TXT

/*
  EOCDR: End of Central Directory Record(4.3.16 in spec)
*/

struct EOCDRecord {

  /*
    unsued    
    signature: 4 (0x06054b50)
    diskrel1: 2
    diskrel2: 2
    diskrel3: 2
  */
  // cd stands for central directory
  uint16_t _cdEntryNum; 
  uint32_t _cdSize;
  uint32_t _cdOffset;
  /*
    unused
    commentLen: 2
  */

  EOCDRecord(uint16_t cdEntryNum, uint32_t cdSize, uint32_t cdOffset) : _cdEntryNum(cdEntryNum), _cdSize(cdSize), _cdOffset(cdOffset) {}
};

/*
  Central Directory header.(4.3.12 in spec)
*/
struct CDRecord {
  /*
  unused
  signature: 4(0x02014b50)
  version made by: 2
  version need to extract: 2  
  */
  uint16_t _flags;
  uint16_t _compressionMethod;
  uint16_t _lastModTime;
  uint16_t _lastModDate;
  uint32_t _crc;
  uint32_t _compressedSize;
  uint32_t _uncompressedSize;
  uint16_t _fileNameLength;
  uint16_t _extraFieldLength;
  uint16_t _commentLength;
  // not used: disNumberStart: 2bytes
  uint16_t _internalFileAttrs;
  uint32_t _externalFileAttrs;
  uint32_t _localHeaderOffset;

  std::string _fileName;
  std::vector<uint8_t> _extraField;
  std::string _comment;

  bool isDir() const { return (_fileName.size() != 0) && (_fileName[_fileName.size()-1] =='/'); }
};

inline uint16_t Read2Byte(uint8_t* buf, size_t pos) {
  return ((uint16_t)buf[pos]) | (((uint16_t)buf[pos+1]) << 8);
}

inline uint32_t Read4Byte(uint8_t* buf, size_t pos) {
  return ((uint32_t)buf[pos]) | (((uint32_t)buf[pos+1]) << 8)| (((uint32_t)buf[pos+2]) << 16)| (((uint32_t)buf[pos+3]) << 24);
}

/*
  Read Central Directory one by one and return CDRecord.
*/
struct CDReader {
  File& _file;
  size_t _curOffset;
  size_t _endOffset;

  const size_t CDR_SIZE = 46; // except for filename, extra fields, comment.

  CDReader(File& file, size_t curOffset, size_t endOffset) : _file(file), _curOffset(curOffset), _endOffset(endOffset) {}
  CDReader(File& file, const EOCDRecord& eocd) : CDReader(file, eocd._cdOffset, eocd._cdOffset+eocd._cdSize) {}

  void readSpecificSize(size_t offset, uint8_t* dst, size_t size) {
    _file.readSpecificSize(offset, dst, size, "Fail to read expected size in CDReader");
  }

  bool isEnd() const { return _curOffset >= _endOffset; }

  CDRecord readOne()
  {
    std::vector<uint8_t> buf(CDR_SIZE);
    uint8_t* data = buf.data();
    readSpecificSize(_curOffset, data, CDR_SIZE);

    if (buf[0] != 0x50 || buf[1] != 0x4b || buf[2] != 0x01 || buf[3] != 0x02)
      throw UnZipError("Central Directory header signature does not match.");

    CDRecord rec;
    rec._flags = Read2Byte(data, 8);
    rec._compressionMethod = Read2Byte(data, 10);
    rec._lastModTime = Read2Byte(data, 12);
    rec._lastModDate = Read2Byte(data, 14);
    rec._crc = Read4Byte(data, 16);
    rec._compressedSize = Read4Byte(data, 20);
    rec._uncompressedSize = Read4Byte(data, 24);
    rec._fileNameLength = Read2Byte(data, 28);
    rec._extraFieldLength = Read2Byte(data, 30);
    rec._commentLength = Read2Byte(data, 32);
    // not used: disNumberStart: 2bytes
    rec._internalFileAttrs = Read2Byte(data, 36);
    rec._externalFileAttrs = Read4Byte(data, 38);
    rec._localHeaderOffset = Read4Byte(data, 42);

    // might better be check corrupted length here.
    rec._fileName.resize(rec._fileNameLength);
    rec._extraField.resize(rec._extraFieldLength);
    rec._comment.resize(rec._commentLength);

    readSpecificSize(_curOffset+46, (uint8_t*)&rec._fileName[0], rec._fileNameLength);
    readSpecificSize(_curOffset+46+rec._fileNameLength, rec._extraField.data(), rec._extraFieldLength);
    readSpecificSize(_curOffset+46+rec._fileNameLength+rec._extraFieldLength, (uint8_t*)&rec._comment[0], rec._commentLength);

    _curOffset = _curOffset+46+rec._fileNameLength+rec._extraFieldLength+rec._commentLength;
    
    return rec;
  }
  
};

struct Inflater {
  void doInflate(uint8_t* srcBuf, size_t srcSize, uint8_t* dstBuf, size_t dstSize) {
    z_stream s;
    s.zalloc = Z_NULL;
    s.zfree = Z_NULL;
    s.opaque = Z_NULL;
    s.avail_in = 0;
    s.next_in = Z_NULL;

    // LZ77 use 32K window size with raw deflate data(no header, negative value means raw deflate data).
    if(inflateInit2(&s, -15) != Z_OK)
      throw UnZipError("Fail to initialize zlib inflate.");
    
    s.avail_in = (uint32_t)srcSize;
    s.next_in = srcBuf;

    s.avail_out = (uint32_t)dstSize;
    s.next_out = dstBuf;

    int status = inflate( &s, Z_SYNC_FLUSH );
    inflateEnd( &s );
    if (status != Z_STREAM_END)
      throw UnZipError("Fail to inflate: " + std::to_string(status));

    if (status == Z_STREAM_END)
    {
      // maybe OK, but throw for safety for a while.
      if (s.total_out != dstSize)
        throw UnZipError("Not enough deflate result.");
    }

  }
};

/*
  Read and uncompress CDRecord entry
*/
struct CDRContentReader {
  File& _file;
  CDRecord _entry;
  size_t _offset;

  const size_t LOCAL_FILE_HEADER_SIZE = 30;

  CDRContentReader(File& file, CDRecord entry) : _file(file), _entry(entry), _offset(0) {
    _offset = readFileContentOffset();
  }

  void readSpecificSize(size_t offset, uint8_t* dst, size_t size) {
    _file.readSpecificSize(offset, dst, size, "Fail to read expected size in CDRContentReader");
  }

  size_t readFileContentOffset()
  {
    std::vector<uint8_t> buf(LOCAL_FILE_HEADER_SIZE);
    readSpecificSize(_entry._localHeaderOffset, buf.data(), LOCAL_FILE_HEADER_SIZE);

    if (buf[0] != 0x50 || buf[1] != 0x4b || buf[2] != 0x03 || buf[3] != 0x04)
      throw UnZipError("Local File header signature does not match.");

    uint16_t fnameLen = Read2Byte(buf.data(), 26);
    uint16_t exFieldLen = Read2Byte(buf.data(), 28);

    size_t offset = _entry._localHeaderOffset+LOCAL_FILE_HEADER_SIZE+fnameLen+exFieldLen;

    if(offset >= _file._size)
      throw UnZipError("Wrong local file header. File content offset exceeds file size.");

    return offset;
  }

  size_t uncompressedSize() const { return _entry._uncompressedSize; }
  size_t compressedSize() const { return _entry._compressedSize; }
  uint16_t compressionMethod() const { return _entry._compressionMethod; }

  void readRawContent(uint8_t* dst, size_t size) {
    if(size != compressedSize())
      throw UnZipError("dst buffer size and compressed size differs.");
    
    int len = _file.readAt(_offset, dst, size);
    if(len != size)
      throw UnZipError("Can't read enough in readRawContent.");
  }

  std::vector<uint8_t> readRawContent() {
    std::vector<uint8_t> buf(compressedSize());

    readRawContent(buf.data(), compressedSize());
    return buf;
  }

  void decompressRawContent(uint8_t* srcBuf, size_t srcSize, uint8_t* dstBuf, size_t dstSize) {
    if(compressionMethod() == 0)
      throw UnZipError("File is uncompressed, no need to call decompressRawContent");
    
    if(compressionMethod() != 8)
      throw UnZipError("Only deflate compression is supported");

    if((srcSize < compressedSize()) || (dstSize < uncompressedSize()))
      throw UnZipError("srcSize or dstSize of decompressRawContent mismatch.");
    
    Inflater inflater;
    inflater.doInflate(srcBuf, compressedSize(), dstBuf, uncompressedSize());
  }

  /*
    read entry file content and inflate if necessary (if no compression, just return raw content)
  */
  std::vector<uint8_t> readContent() {
    std::vector<uint8_t> rawContent = readRawContent();
    if(compressionMethod() == 0)
      return rawContent;
    
    if(compressionMethod() != 8)
      throw UnZipError("Only deflate compression is supported");

    std::vector<uint8_t> uncompressedBuf(uncompressedSize());
    decompressRawContent(rawContent.data(), rawContent.size(), uncompressedBuf.data(), uncompressedSize());
    return uncompressedBuf;
  }


};

/*
  Entry point.
*/
struct EOCDRReader {
  File& _file;

  const size_t EOCDR_SIZE = 22; // comment is variable length, so real size is this size plus comment len.

  EOCDRReader(File& zipFile) : _file(zipFile) {}



  // find 0x06054b50, also check comment len is inside buffer.
  // if not found, return -1.
  int findEndOfCDRInBlock(uint8_t* buf, size_t len) {
    for(int pos = (int)len-(int)EOCDR_SIZE; pos >= 0; pos--) {
      if(buf[pos+0] == 0x50 && buf[pos+1] == 0x4b && buf[pos+2] == 0x05 && buf[pos+3] == 0x06) {
        int commentLen = (int)Read2Byte(buf, pos+EOCDR_SIZE-2);
        if(pos+EOCDR_SIZE+commentLen <= len)
          return pos;
      }
    }
    return -1;
  }

  EOCDRecord readEOCDRecord() {
    // go reader first check 1024, and if not found, check 65k. I use the same strategy.
    size_t bufLens[] = {1024, 65*1024};

    for(int i = 0; i < 2; i++) {
      size_t bufSize = bufLens[i];
      std::vector<uint8_t> buf(bufSize);

      size_t origin = _file._size > bufSize ? _file._size - bufSize : 0;

      size_t readSize = _file.readAt( origin, buf.data(), bufSize );
      if (readSize <= EOCDR_SIZE)
        throw UnZipError("Can't read enough size for End of Central Directory Record. Too small file or read error.");

      int sigPos = findEndOfCDRInBlock(buf.data(), readSize);
      if (sigPos == -1)
        continue;

      uint8_t* eocdrBuf = buf.data()+sigPos;

      return EOCDRecord( Read2Byte(eocdrBuf, 10), Read4Byte(eocdrBuf, 12), Read4Byte(eocdrBuf, 16) );      
    }
    throw UnZipError("Can't find End of Central Directory Record. Corrupted zip file.");
  }
  
};


} ///<impl

//
// facade
//

struct FileEntry {
  File& _file;
  impl::CDRecord _entry;

  FileEntry(File& file, impl::CDRecord entry) : _file(file), _entry(entry) {}
  FileEntry& operator=(const FileEntry& src) { _entry = src._entry; return *this; }

  bool isDir() const { return _entry.isDir(); }
  const std::string& fileName() const { return _entry._fileName; }
  size_t contentSize() const { return _entry._uncompressedSize; }

  std::vector<uint8_t> readContent() {
    impl::CDRContentReader ereader(_file, _entry);
    return ereader.readContent();
  }
};

struct file_entry_iterator {
  File& _file;
  size_t _curOffset;
  size_t _endOffset;

  bool _setupDone = false;
  size_t _nextOffset = 0;
  FileEntry _curEntry;
  
  file_entry_iterator(File& file, size_t curOffset, size_t endOffset) : _file(file), _curOffset(curOffset), _endOffset(endOffset), _curEntry(_file, impl::CDRecord()) {}

  void ensureInit() {
    if(_setupDone)
      return;
    _setupDone = true;
    impl::CDReader reader(_file, _curOffset, _endOffset);
    _curEntry = FileEntry(_file, reader.readOne());
    _nextOffset = reader._curOffset;
  }

  FileEntry& operator *() {
    ensureInit();
    return _curEntry;
  }

  file_entry_iterator& operator++() {
    ensureInit();
    _curOffset = _nextOffset;
    _setupDone = false;
    return *this;
  }

  bool operator ==(const file_entry_iterator& other) const { return _curOffset == other._curOffset; }
  bool operator !=(const file_entry_iterator& other) const { return _curOffset != other._curOffset; }
};

struct FileEntryLister {
  File& _file;
  size_t _cdStartOffset;
  size_t _cdEndOffset;

  FileEntryLister(File& file, const impl::EOCDRecord& eocdr) : _file(file), _cdStartOffset(eocdr._cdOffset), _cdEndOffset(eocdr._cdOffset+eocdr._cdSize) {}

  file_entry_iterator begin() const { return file_entry_iterator(_file, _cdStartOffset, _cdEndOffset); }
  file_entry_iterator end() const { return file_entry_iterator(_file, _cdEndOffset, _cdEndOffset); }
};

struct UnZipper {
  File& _file;
  impl::EOCDRecord _eocdRecord;

  UnZipper(File& file) : _file(file), _eocdRecord( ReadEOCDRecord(file) ) {}

  size_t fileEntryNum() const { return (size_t)_eocdRecord._cdEntryNum; }

  FileEntryLister listFiles() { return FileEntryLister(_file, _eocdRecord); }

private:
  static impl::EOCDRecord ReadEOCDRecord(File& file) {
    impl::EOCDRReader reader(
      file);

    return reader.readEOCDRecord();
  }
};

} ///<cppunzip
#endif
